#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include "include/sockUtils.h"
#include "include/packet.h"

/**
 * Liest Eingabe von Stdin ein.
 *
 * @return buffer mit eingelesenen Daten und Länge
 */
buffer *readStdIn() {
    DEBUG_CALLBACK({
        freopen("../file.txt","r",stdin);
    })

    int chunkSize = 512;
    int buffLength = chunkSize;

    void *valueTemp = malloc(chunkSize);
    void *value = malloc(chunkSize);

    int valueLength = 0;
    size_t nread;

    // read 1 byte per iteration and reallocs one more byte per iteration
    while ((nread = fread(valueTemp, 1, chunkSize, stdin)) > 0) {
        if ((valueLength + nread) > buffLength) {
            buffLength += chunkSize;
            value = realloc(value, buffLength);
        }
        // concatenation of the current read byte and the already read bytes
        memmove(value + valueLength, valueTemp, nread);

        // determing the value length
        valueLength += nread;
    }

    free(valueTemp);
    //creating the buffer for key
    return createBufferFrom(valueLength, value);
}


/**
 * Erstellt eine neue Anfrage and die übergebenen Adresse
 *
 * @param dnsAddress e.g. "localhost"
 * @param port e.g. "3002"
 * @param instr Insructions struct mit GET, SET, DELETE
 * @param key buffer
 * @return -1 bei Fehler, ansonsten 0
 */
int makeRequest(char dnsAddress[], char port[], int op, buffer *key) {
    int clientSocket = setupClient(dnsAddress, port);
    INVARIANT(clientSocket != -1, -1, "");

    // Falls es eine SET anfrage ist, den Wert vom Stdin einlesen
    buffer *value = NULL;
    if (op == SET_CODE) {
        value = readStdIn();
    }

    // create Message
    message *requestMessage = createMessage(op, NOT_ACKNOWLEDGED, key, value);

    // error handling if createMessage failed
    INVARIANT_CB(requestMessage != NULL, -1, "", {
        close(clientSocket);
        freeBuffer(key);
        if(value) freeBuffer(value);
    })

    // send Message
    int status = sendMessage(clientSocket, requestMessage);
    freeMessage(requestMessage);

    // error handling if sendMessage failed
    INVARIANT_CB(status != -1, -1, "Failed to send message", {
        close(clientSocket);
        if(value) freeBuffer(value);
        freeBuffer(key);
    })


    // recv Message from Server
    packet *responsePacket = recvPacket(clientSocket);

    // error handling if recvMessage failed
    INVARIANT_CB(responsePacket && responsePacket->message, -1, "Failed to recv response", {
        close(clientSocket);
    })

    // printing the value when value != NULL && GET && ACK Bit is set
    if (responsePacket->message->op == GET_CODE && responsePacket->message->ack == ACKNOWLEDGED && responsePacket->message->value) {
        fwrite(responsePacket->message->value->buff, responsePacket->message->value->length, 1, stdout);
    }

    close(clientSocket);
    freePacket(responsePacket);
    return 0;
}

int parseOpCode(char codeAsString[]) {
    if (strcmp(codeAsString, "SET") == 0) {
        return SET_CODE;
    } else if (strcmp(codeAsString, "GET") == 0) {
        return GET_CODE;
    } else if (strcmp(codeAsString, "DELETE") == 0) {
        return  DELETE_CODE;
    } else {
        fprintf(stderr, "Invalid instruction! expected e.g. SET, GET or Delete! ");
        exit(1);
    }
}

int main(int argc, char *argv[]) {
    INVARIANT(argc == 5, 1, "Invalid arguments! expected e.g. './client localhost 4711 GET /pics/cat.jpg' ")

    // Parsen vom Anfrangen Typ
    int op = parseOpCode(argv[3]);

    // create Buffer, with key and keylength
    char* keyRaw = malloc(sizeof(char) * strlen(argv[4]));
    memcpy(keyRaw, argv[4], strlen(argv[4]));
    buffer *key = createBufferFrom(strlen(argv[4]), keyRaw);

    return makeRequest(argv[1], argv[2], op, key);
}

