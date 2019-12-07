#include "../include/sockUtils.h"
#include "../include/lookup.h"

#include <string.h>
#include <stdlib.h>
#include <netdb.h>
#include <stdio.h>

join *createJoin(int reply, int isLookup, uint16_t hashID, uint16_t nodeID, uint32_t nodeIP, uint16_t nodePort) {
    lookup *ret = calloc(1, sizeof(lookup));
    ret->reply = reply;
    ret->lookup = isLookup;
    ret->
    ret->hashID = hashID;
    ret->nodeID = nodeID;
    ret->nodeIP = nodeIP;
    ret->nodePort = nodePort;

    return ret;
}

buffer *encodeLookup(lookup *l) {
    unsigned char *buff = calloc(11, sizeof(unsigned char));

    buff[0] = buff[0] | 0b10000000; // Control bit immer auf 1

    if (l->reply) buff[0] = buff[0] | 0b00000010;
    if (l->lookup) buff[0] = buff[0] | 0b00000001;

    uint16_t encodedNodeID = htons(l->nodeID);
    uint16_t encodedNodePort = htons(l->nodePort);

    memcpy(&buff[1], &l->hashID, 2); // bitstring
    memcpy(&buff[3], &encodedNodeID, 2);
    memcpy(&buff[5], &l->nodeIP, 4); // schon in network byte order
    memcpy(&buff[9], &encodedNodePort, 2);

    return createBufferFrom(11, buff);
}

/**
 * Helper der ein Packet zu einem Lookup Struct parsed.
 *
 * @param firstLine
 * @param buff
 * @return lookup
 */
lookup *decodeLookup(uint8_t firstLine, buffer* buff) {
    int reply = checkBit(firstLine, 1);
    int lookup = checkBit(firstLine, 0);

    uint16_t hashID = 0;
    uint16_t nodeID = 0;
    uint32_t nodeIP = 0;
    uint16_t nodePort = 0;

    memcpy(&hashID, &buff->buff[0], 2);
    memcpy(&nodeID, &buff->buff[2], 2);
    memcpy(&nodeIP, &buff->buff[4], 4);
    memcpy(&nodePort, &buff->buff[8], 2);

    // nodeIP schon in network byte order
    // und da hashID ein bitstring ist, muss es nicht in eine bestimmte order
    return createLookup(reply, lookup, hashID, ntohs(nodeID), nodeIP, ntohs(nodePort));
}

int sendLookup(int socket, lookup* l) {
    buffer *b = encodeLookup(l);
    LOG_DEBUG("===========================\tSend Lookup\t=========================== ")
    printLookup(l);
    return sendAll(socket, b->buff, b->length);
}

lookup *recvLookup(int socket, uint8_t firstLine) {
    buffer* b = recvBytesAsBuffer(socket, 10);
    lookup* ret = decodeLookup(firstLine, b);
    LOG_DEBUG("===========================\tReceived Lookup\t=========================== ")
    printLookup(ret);

    return ret;
}

/**
 * Kopiert die ersten zwei bytes des keys in ein uint16_t int.
 * Falls weniger als zwei Bytes im Key vorhanden sind, werden nur keyLength bytes kopiert.
 *
 * @param key
 * @return uint_16t hash wert von key
 */
uint16_t getHashForKey(buffer *key) {
    uint16_t ret = 0;

    memcpy(&ret, key->buff, key->length > 2 ? 2 : key->length);

    return ret;
}

uint16_t checkHashID(uint16_t hashID, serverArgs* args) {
    if(hashID > args->prevID && hashID <= args->ownID) return OWN_SERVER;
    else if(hashID > args->ownID && hashID <= args->nextID) return NEXT_SERVER;
    else return UNKNOWN_SERVER;
}

void printLookup(lookup* l) {
#ifdef DEBUG
    fprintf(stderr, "{\n");
    fprintf(stderr, "\treply: %d\n", l->reply);
    fprintf(stderr, "\tlookup: %d\n", l->lookup);
    fprintf(stderr, "\thashID: %d\n", l->hashID);
    fprintf(stderr, "\tnodeID: %d\n", l->nodeID);
    fprintf(stderr, "\tnodeIP: %d\n", l->nodeIP);
    fprintf(stderr, "\tnodePort: %d\n", l->nodePort);
    fprintf(stderr, "}\n");
#endif
}



