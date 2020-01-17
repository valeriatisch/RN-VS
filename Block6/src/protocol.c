#include "../include/protocol.h"
#include "../include/sockUtils.h"

#include <string.h>
#include <stdlib.h>
#include <netdb.h>
#include <stdio.h>

protocol *createProtocol(int LI, int VN, int mode, int stratum, int poll, int precision, uint32_t root_delay, uint32_t root_dispersion,
uint32_t ref_ID, uint64_t ref_ts, uint64_t orig_ts, uint64_t rec_ts, uint64_t trans_ts){

    protocol *p = calloc(1, sizeof(protocol));
    p->LI = LI;
    p->VN = VN;
    p->mode = mode;
    p->stratum = stratum;
    p->poll = poll;
    p->precision = precision;
    p->root_delay = root_delay;
    p->root_dispersion = root_dispersion;
    p->ref_ID = ref_ID;
    p->ref_ts = ref_ts;
    p->orig_ts = orig_ts;
    p->rec_ts = rec_ts;
    p->trans_ts = trans_ts;

    return p;
}

buffer *encodeProtocol(protocol* p){
    //TODO
    return createBufferFrom(48, buff);
}

protocol *decodeProtocol(buffer* buff){
    //TODO
    return createProtocol(LI, VN, mode, stratum, poll, precision, root_delay, root_dispersion, ref_ID, ref_ts, orig_ts, rec_ts, trans_ts);
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
