#ifndef BLOCK32_SOCKUTILS_H
#define BLOCK32_SOCKUTILS_H

#include <stdint.h>

#define DEBUG

#define LOG(msg_string) \
    fprintf(stderr, "%s\n", msg_string); \

#ifdef DEBUG
#define LOG_DEBUG(msg_string) \
    fprintf(stderr, "%s\n", msg_string);
#else
#define LOG_DEBUG(msg_string)
#endif


#ifdef DEBUG
#define DEBUG_CALLBACK(callback) \
    do callback while(0);
#else
#define DEBUG_CALLBACK(callback)
#endif


#define INVARIANT(expression, returnValue, message) \
  if (!(expression)) { \
    LOG(message) \
    exit(-1); \
    return returnValue; \
  }

#define INVARIANT_CB(expression, returnValue, message, callback) \
  if (!(expression)) { \
    do callback while(0); \
    LOG(message) \
    exit(-1); \
    return returnValue; \
  }

#define INVARIANT_CONTINUE_CB(expression, message, callback) \
  if (!(expression)) { \
    do callback while(0); \
    LOG(message) \
    continue; \
  }

#define OWN_SERVER 0
#define NEXT_SERVER 1
#define UNKNOWN_SERVER 2

typedef struct serverArgs_ {
    int ownID;
    char* ownIP;
    char* ownPort;
    uint32_t ownIpAddr;

    int nextID;
    char* nextIP;
    char* nextPort;

    int prevID;
    char* prevIP;
    uint16_t prevPort;
} serverArgs;

typedef struct _buffer {
    uint8_t *buff;
    uint32_t length;
    uint32_t maxLength;
} buffer;

//void start_stabilize(serverArgs* args);
//void stabilize(lookup* stabilize_msg, char* nextIP, char* nextPort);
buffer* createBuffer(uint32_t length);
buffer* createBufferFrom(uint32_t length, void* existingBuffer);
void freeBuffer(buffer *buff);
buffer* copyBuffer(buffer* b);

int recvBytes(int socket, int length, void* buff);
buffer* recvBytesAsBuffer(int socket, int length);
int sendAll(int socket, void* value, uint32_t length);

int setupServer(char address[], char port[], uint32_t *ownIpAddr);
int setupClient(char dnsAddress[], char port[]);
int setupClientWithAddr(uint32_t s_addr, uint16_t port);

int checkBit(unsigned bitsequence, int n);


#endif //BLOCK32_SOCKUTILS_
