#ifndef BLOCK6_SOCKUTILS_H
#define BLOCK6_SOCKUTILS_H

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

struct timespec sendPacket(int sockfd, struct addrinfo *p);
void receivePacket(int n, int sockfd, double* delay_arr, struct timespec start, struct addrinfo *p_arg);

void print_result(uint32_t IP,int nummer, double root_dispersion, double dispersion8, double delay, double offset);
double dispersion8(double *delay_arr);
double delay_as_seconds(double* delay_arr, int n, double t1, double t2, double t3, double t4);
double offset_as_seconds(double t1, double t2, double t3, double t4);

double get_max(double* array);
double get_min_not_zero(double* array);
void getTime(struct timespec time_to_get);
long getTimeDiff_asNano(struct timespec start, struct timespec stop);
void timeSleep_nano(long time_nano);
uint32_t ip_to_uint(char *ip_addr);

buffer* createBuffer(uint32_t length);
buffer* createBufferFrom(uint32_t length, void* existingBuffer);
void freeBuffer(buffer *buff);
buffer* copyBuffer(buffer* b);

buffer* recvBytesAsBuffer(int socket, int length, struct addrinfo *p);
int sendAll(int socket, void* value, uint32_t length, struct addrinfo *p);

int checkBit(unsigned bitsequence, int n);


#endif //BLOCK6_SOCKUTILS_