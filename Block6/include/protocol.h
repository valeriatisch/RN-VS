#include "../include/sockUtils.h"
#define OFFSET 2208988800

typedef struct _protocol  {
    uint8_t li_vn_mode;
    
    uint8_t stratum;
    uint8_t poll;
    uint8_t precision;

    uint32_t root_delay;
    uint32_t root_dispersion;
    uint32_t ref_ID;

    uint32_t ref_ts_sec;
    uint32_t ref_ts_fsec;

    uint32_t orig_ts_sec;
    uint32_t orig_ts_fsec;
    
    uint32_t rec_ts_sec;
    uint32_t rec_ts_fsec;
    
    uint32_t trans_ts_sec;
    uint32_t trans_ts_fsec;
} protocol;

typedef struct _buffer {
    uint8_t *buff;
    uint32_t length;
    uint32_t maxLength;
} buffer;

protocol *createProtocol(uint8_t li_vn_mode, uint8_t stratum, uint8_t poll, uint8_t precision, uint32_t root_delay, uint32_t root_dispersion, uint32_t ref_ID, 
uint32_t ref_ts_sec, uint32_t ref_ts_fsec, uint32_t orig_ts_sec, uint32_t orig_ts_fsec, uint32_t rec_ts_sec, uint32_t rec_ts_fsec, uint32_t trans_ts_sec, uint32_t trans_ts_fsec);
buffer *encodeProtocol(protocol *p);
protocol *decodeProtocol(buffer* buff);

int sendProtocol(int socket, protocol* p, struct addrinfo *addr);
protocol *recvProtocol(int socket, struct addrinfo *addr);
void printProtocol(protocol *p);



