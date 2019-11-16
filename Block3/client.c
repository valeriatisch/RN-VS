#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <netdb.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>

#include <arpa/inet.h>
void    print_bits(unsigned char octet)
{
    int z = 128, oct = octet;

    while (z > 0)
    {
        if (oct & z)
            write(1, "1", 1);
        else
            write(1, "0", 1);
        z >>= 1;
    }
    printf("\n");
}

#define MAXDATASIZE 100 // max number of bytes we can get at once

// get sockaddr, IPv4 or IPv6:
void *get_in_addr(struct sockaddr *sa){
    if (sa->sa_family == AF_INET) {
        return &(((struct sockaddr_in*)sa)->sin_addr);
    }

    return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

int main(int argc, char *argv[]){
    int sockfd, numbytes;
    char buf[MAXDATASIZE];
    struct addrinfo hints, *servinfo, *p;
    int rv;
    char s[INET6_ADDRSTRLEN];
    char* send_buffer[100];

    if (argc != 5) {
        printf("%d\n",argc);
        fprintf(stderr,"usage: client hostname key methode value\n");
        exit(1);
    }
    //TODO: key richtig auslesen
    char* port = argv[2];
    char* call_type = argv[3];
    void* res_value = argv[4];
    char* res_key = argv[4];

    printf("key name: %s\n", res_key);
    //alloc header
    char* call_header = calloc(1,sizeof(char)*1);

    //set flags in header
    if(strcmp(call_type,"GET") == 0) {
        call_header[0] |= 1UL << 2;
    } else if(strcmp(call_type,"SET") == 0) {
        call_header[0] |= 1UL << 1;
    } else if(strcmp(call_type,"DELETE") == 0) {
        call_header[0] |= 1UL;
    } else
    {
        perror("invalid command\n");
    }

    print_bits(call_header[0]);
    //TODO: keylen + valuelen berechen -> in header speichern

    char* key_length_header = calloc(1, sizeof(char)*2);
    char* value_length_header = calloc(1, sizeof(char)*4);

    short key_len = (short) strlen(res_key);

    printf("key length is: %hu\n",key_len );

    key_length_header[0] = (key_len << 8) & 0xFF;
    key_length_header[1] = key_len & 0xFF;

    printf("key length:\n");
    print_bits(key_length_header[0]);
    print_bits(key_length_header[1]);
    printf("\n");


    // Calculate value size by reading it bytewise till EOF,
    int value_len = 0;
    while(getchar() != EOF) {
        value_len++;
    }
    printf("Value length is: %d\n", value_len);
    value_length_header[0] = (value_len >> 24) & 0xFF;
    value_length_header[1] = (value_len >> 16) & 0xFF;
    value_length_header[2] = (value_len >> 8) & 0xFF;
    value_length_header[3] = value_len & 0xFF;

    printf("Value length:\n");

    print_bits(value_length_header[0]);
    print_bits(value_length_header[1]);
    print_bits(value_length_header[2]);
    print_bits(value_length_header[3]);

    printf("\n");



    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;

    if ((rv = getaddrinfo(argv[1], port, &hints, &servinfo)) != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
        return 1;
    }

    // loop through all the results and connect to the first we can
    for(p = servinfo; p != NULL; p = p->ai_next) {
        if ((sockfd = socket(p->ai_family, p->ai_socktype,p->ai_protocol)) == -1) {
            perror("client: socket");
            continue;
        }

        if (connect(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
            close(sockfd);
            perror("client: connect");
            continue;
        }
        ;
        break;
    }

    if (p == NULL) {
        fprintf(stderr, "client: failed to connect\n");
        return 2;
    }

    //send data to server
    //send header + key
    int tmp = 0;
    // send first byte
    printf("Debug: sending first byte\n");
    printf("size of first header: %lu \n", sizeof(call_header)/sizeof(char));
    while(tmp < sizeof(call_header)){
        if(tmp += send(sockfd,call_header,1-tmp,0) == -1){
            perror("sending first byte");
        }
    }
    printf("debug:sending first byte\n");

    tmp = 0;
    // sending key length
    while(tmp < sizeof(key_length_header)) {
        if(tmp += send(sockfd, key_length_header, 2-tmp,0)) {
            perror("sending value value");
        }
    }

    tmp = 0;
    //sending value length
    while(tmp < sizeof(value_length_header)) {
        if(tmp += send(sockfd, value_length_header,4-tmp,1)) {
            perror("sending value length");
        }
    }

    //send value
    tmp = 0;
    while(tmp < sizeof(res_value)){
        if(tmp += send(sockfd,res_value,sizeof(res_value)-tmp,0) == -1){
            perror("send:value");
        }
    }

    freeaddrinfo(servinfo); // all done with this structure

    printf("---------------------- ALL DATA SENT; READY TO RECEIVE DATE FROM SERVER --------------------\n");

    //receive data from server
    if ((numbytes = recv(sockfd, buf, MAXDATASIZE-1, 0)) == -1) {
        perror("recv");
        exit(1);
    }

    buf[numbytes] = '\0';

    //return value for 'GET'
    //TODO: nur value ohne header printen
    if(strcmp(call_type,"GET") == 0) {
        printf("client: received '%s'\n", buf);
    }
    close(sockfd);

    return 0;
}
