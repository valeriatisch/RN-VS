#include <stdio.h>
#include <sys/socket.h>
#include <unistd.h>
#include <stdint.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <pthread.h>
#include <signal.h>

#include "./include/fingertable.h"
#include "./include/hash.h"
#include "./include/lookup.h"
#include "./include/packet.h"
#include "./include/clientStore.h"
#include "./include/peerClientStore.h"

//New functions:
int sendJoinMsg(serverArgs *args);

uint32_t ip_to_uint(char *ip_addr) {
    struct sockaddr_in sa;
    if((inet_pton(AF_INET, ip_addr, (&sa.sin_addr))) == 0){
        perror("Converting IP to int\n");
    }
    return sa.sin_addr.s_addr;
}


char* ip_to_str(uint32_t ip){
    //buffer to save ip as an array
    char* input_buffer = calloc(1, INET_ADDRSTRLEN);
    memcpy(input_buffer, &ip, INET_ADDRSTRLEN);

    //array to save result of ntop
    char* ip_str = calloc(1, INET_ADDRSTRLEN);

    //DEBUG
    //printf("ip in \n\tuint32_t: %d\n\tstring: %s\n", ip, input_buffer);


    inet_ntop(AF_INET, input_buffer, ip_str, INET_ADDRSTRLEN);
    if (input_buffer == NULL)
        perror("Converting IP to string\n");

    //printf("ip_str after conversion: %s\n", ip_str);
    free(input_buffer);
    return  ip_str;
}

char* port_to_str(uint16_t port){
    char* str = malloc(sizeof(char)*5);
    sprintf(str,"%u", port);
    return str;
}

void stabilize(serverArgs* args){
    if(args->nextIP != NULL && args->nextPort != NULL){
        printf("args->nextIP: %s, args->nextPort: %s",args->nextIP, args->nextPort);
        int peerSock = setupClient(args->nextIP, args->nextPort);
        //send stabilize
        lookup *stabilize_msg = createLookup(0, 0, 0, 1, 0, 0, 0, 0, args->ownID, ip_to_uint(args->ownIP),atoi(args->ownPort));
        sendLookup(peerSock, stabilize_msg);
        free(stabilize_msg);
        close(peerSock);
    }
    else{
        perror("stabilize: kein nachfolger eingetragen\n");
    }
}

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

int handlePacket(packet *pkt, int sock, fd_set *master, serverArgs *args, int *fdMax, ft** fingertable) {
    if (pkt->control) {
        /******* Handle Join/Notify/Stabilize/Lookup/Reply Request  *******/

        if(pkt->lookup->finger){
        //setup finger table
            fingertable = create_ft(args);
            /*
            if(fingertable != NULL){
            //TODO: send lookup with f_ack
                peerToClientHashStruct *pHash = getPeerToClientHash(sock);    
                close(pHash->peerSocket);
                FD_CLR(pHash->peerSocket, master);
                lookup* ack_msg = createLookup(0, 1, 0, 0, 0, 0, 0, 0, args->ownID, ip_to_uint(args->ownIP), atoi(args->ownPort));
                sendMessage(pHash->clientSocket, ack_msg);
                close(pHash->clientSocket);

                deletePeerToClientHash(pHash->peerSocket);
            }
            */

        }

        else if(pkt->lookup->join){
            //ersten peer hinzufügen
            if(args->nextID == -1) {
                args->prevID = pkt->lookup->nodeID;
                args->prevIP = calloc(1, sizeof(ip_to_str(pkt->lookup->nodeIP)) + 1);
                strncpy(args->prevIP,  ip_to_str(pkt->lookup->nodeIP), sizeof(ip_to_str(pkt->lookup->nodeIP)) + 1);
                args->prevPort = calloc(1, 5);
                strncpy(args->prevPort, port_to_str(pkt->lookup->nodePort), 5);

                args->nextID = pkt->lookup->nodeID;
                args->nextIP = calloc(1, sizeof(ip_to_str(pkt->lookup->nodeIP)) + 1);
                strncpy(args->nextIP, ip_to_str(pkt->lookup->nodeIP),sizeof(ip_to_str(pkt->lookup->nodeIP)) + 1);
                args->nextPort = calloc(1,5);
                strncpy(args->nextPort, port_to_str(pkt->lookup->nodePort), 5);

                lookup* notify_msg = createLookup(0, 0, 0, 1, 0, 0, 0, pkt->lookup->hashID, args->ownID, ip_to_uint(args->ownIP), atoi(args->ownPort));
                int peerSock = setupClientWithAddr(pkt->lookup->nodeIP, pkt->lookup->nodePort);
                //send notify to joined peer
                sendLookup(peerSock, notify_msg);
                free(notify_msg);
                close(peerSock);
            }
            //join an erster stelle
            else if(args->ownID < args->prevID){

                if((pkt->lookup->nodeID > args->ownID && pkt->lookup->nodeID > args->prevID) || (pkt->lookup->nodeID < args->ownID)){
                    //join->update pre
                    args->prevID = pkt->lookup->nodeID;
                    args->prevIP = calloc(1, sizeof(ip_to_str(pkt->lookup->nodeIP)) + 1);
                    strncpy(args->prevIP,  ip_to_str(pkt->lookup->nodeIP), sizeof(ip_to_str(pkt->lookup->nodeIP)) + 1);
                    args->prevPort = calloc(1, 5);
                    strncpy(args->prevPort, port_to_str(pkt->lookup->nodePort), 5);

                    //create notify message
                    lookup* notify_msg = createLookup(0, 0, 0, 1, 0, 0, 0, pkt->lookup->hashID, args->ownID, ip_to_uint(args->ownIP), atoi(args->ownPort));
                    int peerSock = setupClientWithAddr(pkt->lookup->nodeIP, pkt->lookup->nodePort);
                    //send notify to joined peer
                    sendLookup(peerSock, notify_msg);
                    free(notify_msg);
                    close(peerSock);
                }
                //forward join-message
                else {
                    int peerSock = setupClient(args->nextIP, args->nextPort);
                    sendLookup(peerSock, pkt->lookup);
                    close(peerSock);
                }

            } else {
                //join->update pre
                if(pkt->lookup->nodeID < args->ownID && pkt->lookup->nodeID > args->prevID){
                    args->prevID = pkt->lookup->nodeID;
                    args->prevIP = calloc(1, sizeof(ip_to_str(pkt->lookup->nodeIP)) + 1);
                    strncpy(args->prevIP,  ip_to_str(pkt->lookup->nodeIP), sizeof(ip_to_str(pkt->lookup->nodeIP)) + 1);
                    args->prevPort = calloc(1, 5);
                    strncpy(args->prevPort, port_to_str(pkt->lookup->nodePort), 5);

                    //create notify message
                    lookup* notify_msg = createLookup(0, 0, 0, 1, 0, 0, 0, pkt->lookup->hashID, args->ownID, ip_to_uint(args->ownIP), atoi(args->ownPort));

                    int peerSock = setupClientWithAddr(pkt->lookup->nodeIP, pkt->lookup->nodePort);
                    //send notify to joined peer
                    sendLookup(peerSock, notify_msg);
                    free(notify_msg);
                    close(peerSock);
                }
                //forward join message
                else{
                    int peerSock = setupClient(args->nextIP, args->nextPort);
                    sendLookup(peerSock, pkt->lookup);
                    close(peerSock);
                }
            }

        } else if(pkt->lookup->notify){
            if(args->ownID != pkt->lookup->nodeID) {
                //update successor
                args->nextID = pkt->lookup->nodeID;
                args->nextIP = calloc(1, sizeof(ip_to_str(pkt->lookup->nodeIP)) + 1);
                strncpy(args->nextIP, ip_to_str(pkt->lookup->nodeIP),sizeof(ip_to_str(pkt->lookup->nodeIP)) + 1);
                args->nextPort = calloc(1,5);
                strncpy(args->nextPort, port_to_str(pkt->lookup->nodePort), 5);
            }

        } else if(pkt->lookup->stabilize){
            // predecessor not set
            if(args->prevID == NULL || args->prevIP == NULL) {
                //update predecessor
                args->prevID = pkt->lookup->nodeID;
                args->prevIP = calloc(1, sizeof(ip_to_str(pkt->lookup->nodeIP)) + 1);
                strncpy(args->prevIP, ip_to_str(pkt->lookup->nodeIP), sizeof(ip_to_str(pkt->lookup->nodeIP)) + 1);
                args->prevPort = calloc(1, 5);
                strncpy(args->prevPort, port_to_str(pkt->lookup->nodePort), 5);
            }
            
            else if (args->prevID != pkt->lookup->nodeID) {
                //create notify message
                lookup* notify_msg = createLookup(0, 0, 0, 1, 0, 0, 0, pkt->lookup->hashID, args->prevID, ip_to_uint(args->prevIP), atoi(args->prevPort));
                int peerSock = setupClientWithAddr(pkt->lookup->nodeIP, pkt->lookup->nodePort); 

                //send notify to joined peer
                sendLookup(peerSock, notify_msg);
                free(notify_msg);
                close(peerSock);
            }

        } else if (pkt->lookup->lookup) {
            int hashDestination = checkHashID(pkt->lookup->hashID, args);

            switch (hashDestination) {
                // Case 1 Der Key liegt auf dem next Server -> REPLY an lookup server schicken mit next IP, next PORT, next ID
                case NEXT_SERVER: {

                    // IP String zu 32bit Zahl konvertieren
                    struct sockaddr_in sa;
                    inet_pton(AF_INET, args->nextIP, (&sa.sin_addr));

                    lookup *responseLookup = createLookup(0, 0, 0, 0, 0, 0, 1, pkt->lookup->hashID, args->nextID, sa.sin_addr.s_addr,
                                                          atoi(args->nextPort));

                    int peerSock = setupClientWithAddr(pkt->lookup->nodeIP, pkt->lookup->nodePort);
                    sendLookup(peerSock, responseLookup);
                    free(responseLookup);
                    close(peerSock);
                    break;
                }

                // Case 2 Der key liegt auf einem unbekannten Server -> Lookup an nächsten Server
                case UNKNOWN_SERVER: {
                    int peerSock;

                    //fingertable wenn möglich benutzen
                    if((fingertable != NULL) && (fingertable_full == 1)){
                        //fingertable durchsuchen
                        int i = ft_index_of_peer(fingertable, pkt->lookup->hashID, args->ownID);
                        if(i == -1){
                            perror("fingertable");
                            exit(42);
                        }
                        //richtigen peer ansprechen
                        peerSock = setupClient(fingertable[i]->ip, fingertable[i]->port);
                    }
                    
                    //keine fingertable vorhanden
                    else{
                        peerSock = setupClient(args->nextIP, args->nextPort);
                    }
                    sendLookup(peerSock, pkt->lookup);
                    close(peerSock); 
                }
            }

        } else if (pkt->lookup->reply) {
            
            //reply for fingertable
            if(fingertable != NULL){
                for(int i = 0; i < 16;i++){
                    if(fingertable[i]->id == pkt->lookup->hashID){ 
                        fingertable[i]->ip = pkt->lookup->nodeIP;
                        fingertable[i]->port = pkt->lookup->nodePort;
                    }
                }
            }
            
            else{
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
                    lookup *l = createLookup(0, 0, 0, 0, 0, 0, 1, hashID, args->ownID, args->ownIpAddr, atoi(args->ownPort));
