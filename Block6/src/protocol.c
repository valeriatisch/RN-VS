#include "../include/protocol.h"
#include "../include/sockUtils.h"

#include <string.h>
#include <stdlib.h>
#include <netdb.h>
#include <stdio.h>


protocol *createProtocol(uint8_t li_vn_mode, uint8_t stratum, uint8_t poll, uint8_t precision, uint32_t root_delay, uint32_t root_dispersion, uint32_t ref_ID, 
uint32_t ref_ts_sec, uint32_t ref_ts_fsec, uint32_t orig_ts_sec, uint32_t orig_ts_fsec, uint32_t rec_ts_sec, uint32_t rec_ts_fsec, uint32_t trans_ts_sec, uint32_t trans_ts_fsec){

    protocol *p = calloc(1, sizeof(protocol));
    p->li_vn_mode = li_vn_mode;
    
    p->stratum = stratum;
    p->poll = poll;
    p->precision = precision;

    p->root_delay = root_delay;
    p->root_dispersion = root_dispersion;
    p->ref_ID = ref_ID;

    p->ref_ts_sec = ref_ts_sec;
    p->ref_ts_fsec = ref_ts_sec;

    p->orig_ts_sec = orig_ts_sec;
    p->orig_ts_fsec = orig_ts_fsec;
    
    p->rec_ts_sec = ref_ts_sec;
    p->rec_ts_fsec = rec_ts_fsec;
    
    p->trans_ts_sec = trans_ts_sec;
    p->trans_ts_fsec = trans_ts_fsec;

    return p;
}

buffer *encodeProtocol(protocol* p){
    //TODO
    //0 Byte
        //0-1 LI    (0 << 6) |
        //2-4 VN    (4 << 3) |
        //5-7 Mode  3
    //1 Byte
        //0-7 Stratum
    //2 Byte
        //0-7 Poll
    //3 Byte Precision
    //4-7 Byte Root Delay
    //8-11 Root Dispersion          ntohl
    //12-15 Reference ID
    //16-23 Reference Timestamp
    //24-31 Origin Timestamp        
    //32-39 Receive Timestamp       ntohl
    //40-47 Transmit Timestamp      ntohl / htonl 

    unsigned char *buff = calloc(48, sizeof(unsigned char));
    buff[0] = 0b00100011; // setting LI, VN, MODE
    
    // rest supposed to be zero, has already been calloced
    /*
    memcpy(&buff[1], &p->stratum, 1);   
    memcpy(&buff[2], &p->poll, 1);
    memcpy(&buff[3], &p->precision, 1);
    memcpy(&buff[4], &p->root_delay, 4);
    memcpy(&buff[8], &p->root_dispersion, 4);
    memcpy(&buff[12], &p->ref_ID, 4);
    */
    
    return createBufferFrom(48, buff);
}
/*
/  Turn received buffer into readable protocol 
*/
protocol *decodeProtocol(buffer* buff){
    //TODO
    uint32_t root_dispersion = ntohl(buff[8]);
    uint32_t rec_ts_sec = ntohl(buff[32]);
    uint32_t rec_ts_fsec = ntohl(buff[36]);
    uint32_t trans_ts_sec = ntohl(buff[40]);
    uint32_t trans_ts_fsec = ntohl(buff[44]);
    
    return createProtocol(0, 0, 0, 0, 0, root_dispersion, 0, 0, 0, 0, 0,
    rec_ts_sec, rec_ts_fsec, trans_ts_sec, trans_ts_fsec);
}

int sendProtocol(int socket, protocol* p, struct addrinfo *addr){
    buffer *b = encodeProtocol(p);
    fprintf(stderr,"===========================\tSend Protocol\t=========================== ");
    printProtocol(p);

    return sendAll(socket, b->buff, b->length, addr);
}

protocol *recvProtocol(int socket, struct addrinfo *addr){
    buffer *buff = recvBytesAsBuffer(socket, 48, addr);
    protocol *p = decodeProtocol(buff);
    fprintf(stderr,"===========================\tReceived Protocol\t=========================== ");
    printProtocol(p);

    return p;
}

void printProtocol(protocol *p){
    fprintf(stderr, "{\n");
    fprintf(stderr, "\tLI: %d\n",p->LI);
    fprintf(stderr, "\tVN: %d\n",p->VN);
    fprintf(stderr, "\tmode: %d\n", p->mode);
    fprintf(stderr, "\tstratum %d\n", p->stratum);
    fprintf(stderr, "\tpoll: %d\n", p->poll);
    fprintf(stderr, "\tprecision: %d\n", p->precision);
    fprintf(stderr, "\troot_delay: %d\n", p->root_delay);
    fprintf(stderr, "\troot_dispersion: %d\n", p->root_dispersion);
    fprintf(stderr, "\tref_ID: %d\n", p->ref_ID);
    fprintf(stderr, "\tref_ts: %d\n", p->ref_ts);
    fprintf(stderr, "\torig_ts: %d\n", p->orig_ts);
    fprintf(stderr, "\trec_ts: %d\n", p->rec_ts);
    fprintf(stderr, "\ttrans_ts: %d\n", p->trans_ts);
    fprintf(stderr, "}\n");
}
