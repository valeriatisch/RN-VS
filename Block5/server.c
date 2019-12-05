#include <stdio.h>
#include <sys/socket.h>
#include <unistd.h>
#include <stdint.h>
#include <arpa/inet.h>

#include "./include/hash.h"
#include "./include/lookup.h"
#include "./include/packet.h"
#include "./include/clientStore.h"
#include "./include/peerClientStore.h"

int handleHashTableRequest(int socket, message *msg) {
    message *responseMessage;

    switch (msg->op) {
        case DELETE_CODE: {
            int deleteStatus = delete(msg->key);
            INVARIANT(deleteStatus == 0, -1, "Failed to delete")
            responseMessage = createMessage(DELETE_CODE, ACKNOWLEDGED, NULL, NULL);
            break;
        }
        case GET_CODE: {
            hash_struct *obj = get(msg->key);
            responseMessage = createMessage(GET_CODE, ACKNOWLEDGED, copyBuffer(msg->key),
                                            obj != NULL ? copyBuffer(obj->value) : NULL);
            break;
        }
        case SET_CODE: {
            int setStatus = set(copyBuffer(msg->key), copyBuffer(msg->value));
            INVARIANT(setStatus == 0, -1, "Failed to set");
            responseMessage = createMessage(SET_CODE, ACKNOWLEDGED, NULL, NULL);
            break;
        }
        default: {
            LOG("Invalid instructions");
        }
    }

    INVARIANT(responseMessage != NULL, -1, "");
    int status = sendMessage(socket, responseMessage);
    freeMessage(responseMessage);
    INVARIANT(status == 0, -1, "Failed to send message");

    return 0;
}

int handlePacket(packet *pkt, int sock, fd_set *master, serverArgs *args, int *fdMax) {
    if (pkt->control) {
        /******* Handle Lookup/Reply Request  *******/

        if (pkt->lookup->lookup) {
            int hashDestination = checkHashID(pkt->lookup->hashID, args);

            switch (hashDestination) {
                // Case 1 Der Key liegt auf dem next Server -> REPLY an lookup server schicken mit next IP, next PORT, next ID
                case NEXT_SERVER: {

                    // IP String zu 32bit Zahl konvertieren
                    struct sockaddr_in sa;
                    inet_pton(AF_INET, args->nextIP, (&sa.sin_addr));

                    lookup *responseLookup = createLookup(1, 0, pkt->lookup->hashID, args->nextID, sa.sin_addr.s_addr,
                                                          atoi(args->nextPort));

                    int peerSock = setupClientWithAddr(pkt->lookup->nodeIP, pkt->lookup->nodePort);
                    sendLookup(peerSock, responseLookup);
                    free(responseLookup);
                    close(peerSock);
                    break;
                }

                    // Case 2 Der key liegt auf einem unbekannten Server -> Lookup an nächsten Server
                case UNKNOWN_SERVER: {
                    int peerSock = setupClient(args->nextIP, args->nextPort);
                    sendLookup(peerSock, pkt->lookup);
                    close(peerSock);
                }
            }

        } else if (pkt->lookup->reply) {
            // Case 2 REPLY  -> GET/SET/DELETE Anfrage an Server schicken und das Ergebnis an Client schicken
            clientHashStruct *s = getClientHash(pkt->lookup->hashID);

            int peerSock = setupClientWithAddr(pkt->lookup->nodeIP, pkt->lookup->nodePort);
            FD_SET(peerSock, master);
            if (*(fdMax) < peerSock) {
                *fdMax = peerSock;
            }
            setPeerToClientHash(peerSock, s->clientSocket);


            int sendMessageStatus = sendMessage(peerSock, s->clientRequest);
            INVARIANT_CB(sendMessageStatus != -1, -1, "Failed to send Message", {
                close(peerSock);
            });
            deleteClientHash(pkt->lookup->hashID);
        }

        // Sock kann bei einer Control Nachricht immer geschlossen werden
        FD_CLR(sock, master);
        close(sock);
    } else {
        /******* Handle GET/SET/DELETE Request  *******/
        if (pkt->message->ack) {
            peerToClientHashStruct *pHash = getPeerToClientHash(sock);
            close(pHash->peerSocket);
            FD_CLR(pHash->peerSocket, master);


            sendMessage(pHash->clientSocket, pkt->message);
            close(pHash->clientSocket);
            FD_CLR(pHash->clientSocket, master);

            deletePeerToClientHash(pHash->peerSocket);
        } else {
            uint16_t hashID = getHashForKey(pkt->message->key);
            int hashDestination = checkHashID(hashID, args);

            switch (hashDestination) {

                // Case 1 Der Key liegt auf dem aktuellen Server -> Anfrage ausführen und an Client schicken
                case OWN_SERVER: {
                    handleHashTableRequest(sock, pkt->message);
                    FD_CLR(sock, master);
                    close(sock);
                    break;
                }

                    // Case 2 Der Key liegt auf dem next Server -> GET/SET/DELETE an Server und Ergebnis an Client schicken
                case NEXT_SERVER: {
                    int peerSock = setupClient(args->nextIP, args->nextPort);
                    FD_SET(peerSock, master);
                    if (*(fdMax) < peerSock) {
                        *fdMax = peerSock;
                    }
                    setPeerToClientHash(peerSock, sock);

                    int sendMessageStatus = sendMessage(peerSock, pkt->message);
                    INVARIANT_CB(sendMessageStatus != -1, -1, "Failed to send Message", {
                        close(peerSock);
                    });
                    break;
                }

                    // Case 3 Der key liegt auf einem unbekannten Server -> Lookup an nächsten Server
                case UNKNOWN_SERVER: {
                    int status = setClientHash(hashID, copyMessage(pkt->message), sock);
                    if (status == -1){
                        close(sock);
                        return -1;
                    }
                    int peerSock = setupClient(args->nextIP, args->nextPort);
                    lookup *l = createLookup(0, 1, hashID, args->ownID, args->ownIpAddr, atoi(args->ownPort));
                    sendLookup(peerSock, l);
                    free(l);
                    close(peerSock);
                    break;
                }
            }
        }
    }
}

int startServer(serverArgs *args) {
    fd_set master;    // Die Master File Deskriptoren Liste
    fd_set read_fds;  // Temporäre File Deskriptor Liste für select()
    int fdMax;        // Die maximale Anzahl an File Deskriptoren

    FD_ZERO(&master);
    FD_ZERO(&read_fds);

    int socketServer = setupServer(args->ownIP, args->ownPort, &args->ownIpAddr);
    INVARIANT(socketServer != -1, -1, "");


    int listenStatus = listen(socketServer, 10);
    INVARIANT_CB(listenStatus != -1, -1, "Failed to listen for incoming requests", {
        close(socketServer);
    });

    FD_SET(socketServer, &master);
    fdMax = socketServer;


    for (;;) {
        read_fds = master; // Kopieren
        int selectStatus = select(fdMax + 1, &read_fds, NULL, NULL, NULL);
        INVARIANT(selectStatus != -1, -1, "Error in select");

        for (int sock = 0; sock <= fdMax; sock++) {
            if (FD_ISSET(sock, &read_fds)) {
                if (sock == socketServer) {
                    struct sockaddr_storage their_addr;
                    socklen_t addr_size = sizeof their_addr;

                    // Accept a request from a client
                    int clientSocket = accept(socketServer, (struct sockaddr *) &their_addr, &addr_size);
                    INVARIANT_CONTINUE_CB(clientSocket != -1, "Failed to accept client", {});

                    FD_SET(clientSocket, &master);

                    if (clientSocket > fdMax) fdMax = clientSocket;

                    LOG_DEBUG("New connection");
                } else {
                    packet *pkt = recvPacket(sock);
                    INVARIANT_CONTINUE_CB(pkt != NULL, "Failed to recv message", {
                        close(sock);
                        FD_CLR(sock, &master);
                    });

                    handlePacket(pkt, sock, &master, args, &fdMax);
                    freePacket(pkt);
                }
            }
        }
    }
}

serverArgs *parseArguments(char *argv[]) {
    serverArgs *ret = calloc(1, sizeof(serverArgs));

    ret->ownID = atoi(argv[1]);
    ret->ownIP = argv[2];
    ret->ownPort = argv[3];
    ret->ownIpAddr = 0;

    ret->prevID = atoi(argv[4]);

    ret->nextID = atoi(argv[7]);
    ret->nextIP = argv[8];
    ret->nextPort = argv[9];

    return ret;
}

int main(int argc, char *argv[]) {
    // Parse arguments
    INVARIANT(argc == 10, -1, "Command line args invalid");
    serverArgs *args = parseArguments(argv);

    return startServer(args);
}
