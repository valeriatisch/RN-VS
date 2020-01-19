#include "../include/protocol.h"
#include "../include/sockUtils.h"

protocol *createProtocol(uint8_t li_vn_mode, uint8_t stratum, uint8_t poll, uint8_t precision, uint32_t root_delay, uint16_t root_dispersion_s, uint16_t root_dispersion_f, uint32_t ref_ID,
                         uint32_t ref_ts_sec, uint32_t ref_ts_fsec, uint32_t orig_ts_sec, uint32_t orig_ts_fsec, uint32_t rec_ts_sec, uint32_t rec_ts_fsec, uint32_t trans_ts_sec, uint32_t trans_ts_fsec){

    protocol *p = calloc(1, sizeof(protocol));
    p->li_vn_mode = li_vn_mode;

    p->stratum = stratum;
    p->poll = poll;
    p->precision = precision;

    p->root_delay = root_delay;
    p->root_dispersion_s = root_dispersion_s;
    p->root_dispersion_f = root_dispersion_f;
    p->ref_ID = ref_ID;

    p->ref_ts_sec = ref_ts_sec;
    p->ref_ts_fsec = ref_ts_sec;

    p->orig_ts_sec = orig_ts_sec;
    p->orig_ts_fsec = orig_ts_fsec;

    p->rec_ts_sec = rec_ts_sec;
    p->rec_ts_fsec = rec_ts_fsec;

    p->trans_ts_sec = trans_ts_sec;
    p->trans_ts_fsec = trans_ts_fsec;

    return p;
}

buffer *encodeProtocol(protocol* p){

    unsigned char *buff = calloc(1, 48*sizeof(unsigned char));
    buff[0] = 0b00100011; // setting LI, VN, MODE

    return createBufferFrom(48, buff);
}
/*
/  Turn received buffer into readable protocol
*/
protocol *decodeProtocol(buffer* buff){
    //TODO
    uint32_t root_dispersion_s;
    uint32_t root_dispersion_f;
    uint32_t rec_ts_sec;
    uint32_t rec_ts_fsec;
    uint32_t trans_ts_sec;
    uint32_t trans_ts_fsec;

    memcpy(&root_dispersion_s, &buff->buff[8], 2* sizeof(char));
    memcpy(&root_dispersion_f, &buff->buff[10], 2* sizeof(char));
    memcpy(&rec_ts_sec, &buff->buff[32], 4* sizeof(char));
    memcpy(&rec_ts_fsec, &buff->buff[36], 4* sizeof(char));

    rec_ts_sec = htonl(rec_ts_sec);

    root_dispersion_s = htons(root_dispersion_s);
    root_dispersion_f = htons(root_dispersion_f);

    rec_ts_fsec = htonl(rec_ts_fsec);


    memcpy(&trans_ts_sec, &buff->buff[40], 4* sizeof(char));
    memcpy(&trans_ts_fsec, &buff->buff[44], 4* sizeof(char));
    trans_ts_sec = htonl(trans_ts_sec);
    trans_ts_fsec = htonl(trans_ts_fsec);

    return createProtocol(0, 0, 0, 0, 0, (root_dispersion_s), (root_dispersion_f), 0, 0, 0, 0, 0,
                          (rec_ts_sec), (rec_ts_fsec), (trans_ts_sec), (trans_ts_fsec));
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
    fprintf(stderr, "\tLI: %u\n", p->li_vn_mode >> 6);
    fprintf(stderr, "\tVN: %u\n", (p->li_vn_mode & 0x3f) >> 3);
    fprintf(stderr, "\tmode: %u\n", p->li_vn_mode & 0x7);
    fprintf(stderr, "\tstratum %u\n", p->stratum);
    fprintf(stderr, "\tpoll: %u\n", p->poll);
    fprintf(stderr, "\tprecision: %u\n", p->precision);
    fprintf(stderr, "\troot_delay: %u\n", p->root_delay);
    fprintf(stderr, "\troot_dispersion_s: %u\n", p->root_dispersion_s);
    fprintf(stderr, "\troot_dispersion_f: %u\n", p->root_dispersion_f);
    fprintf(stderr, "\tref_ID: %u\n", p->ref_ID);
    fprintf(stderr, "\tref_ts: %u.%u\n", p->ref_ts_sec, p->ref_ts_fsec);
    fprintf(stderr, "\torig_ts: %u.%u\n", p->orig_ts_sec, p->orig_ts_fsec);
    fprintf(stderr, "\trec_ts: %u.%u\n", p->rec_ts_sec, p->rec_ts_fsec);
    fprintf(stderr, "\ttrans_ts: %u.%u\n", p->trans_ts_sec, p->trans_ts_fsec);
    fprintf(stderr, "}\n");
}
