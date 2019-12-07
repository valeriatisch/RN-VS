#ifndef BLOCK3_LOOKUP_H
#define BLOCK3_LOOKUP_H

#include "sockUtils.h"

typedef struct _lookup {
    int join;
    int notify;
    int stabilize;
    int reply;
    int lookup;
    uint16_t hashID;
    uint16_t nodeID;
    uint32_t nodeIP;
    uint16_t nodePort;
} lookup;


buffer *encodeLookup(lookup *l);

lookup *createLookup(int isJoin, int isNotify, int isStabilize, int reply, int isLookup, uint16_t hashID, uint16_t nodeID, uint32_t nodeIP, uint16_t nodePort);

int sendLookup(int socket, lookup* l);
lookup *recvLookup(int socket, uint8_t firstLine);

uint16_t getHashForKey(buffer *key);
uint16_t checkHashID(uint16_t hashID, serverArgs* args);

void printLookup(lookup* l);

#endif //BLOCK3_LOOKUP_H
