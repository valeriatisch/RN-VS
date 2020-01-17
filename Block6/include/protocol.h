#include "../include/sockUtils.h"

typedef struct _protocol  {
    int LI;
    int VN;
    int mode;
    int stratum;
    int poll;
    int precision;
    uint32_t root_delay;
    uint32_t root_dispersion;
    uint32_t ref_ID;
    uint64_t ref_ts;
    uint64_t orig_ts;
    uint64_t rec_ts;
    uint64_t trans_ts;
}protocol;

typedef struct _buffer {
    uint8_t *buff;
    uint32_t length;
    uint32_t maxLength;
} buffer;

protocol *createProtocol(int LI, int VN, int mode, int stratum, int poll, int precision, uint32_t root_delay, uint32_t root_dispersion,
uint32_t ref_ID, uint64_t ref_ts, uint64_t orig_ts, uint64_t rec_ts, uint64_t trans_ts)
buffer *encodeProtocol(protocol *p);
protocol *decodeProtocol(buffer* buff);

int sendProtocol(int socket, protocol* p, struct addrinfo *addr);
protocol *recvProtocol(int socket, struct addrinfo *addr);
void printProtocol(protocol *p);



