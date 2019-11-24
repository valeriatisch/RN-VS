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
#include <assert.h>

#define CALL_LENGTH_SIZE 1
#define KEY_LENGTH_SIZE 2
#define VALUE_LENGTH_SIZE 4
#define HEADER_SIZE 7


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
    int sockfd;
    char buf[7];
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

    //printf("key name: %s\n", res_key);
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

    //print_bits(call_header[0]);

    char* key_length_header = calloc(1, sizeof(char)*2);
    char* value_length_header = calloc(1, sizeof(char)*4);

    short key_len = (short) strlen(res_key);

    //printf("key length is: %hu\n",key_len );

    key_length_header[0] = (key_len << 8) & 0xFF;
    key_length_header[1] = key_len & 0xFF;

    //printf("key length:\n");
    //print_bits(key_length_header[0]);
    //print_bits(key_length_header[1]);
    //printf("\n");

    int value_len = 0;
    char *value_buf = NULL;

    if(strcmp(call_type,"SET")==0) {
        //Read value from stdin and read its size
        char ch;
        while (1) {
            ch = (char) getchar();
            if (ch == EOF) {
                break;
            } else {
                value_len++;
                value_buf = realloc(value_buf, (value_len * sizeof(char)));
                value_buf[value_len - 1] = ch;
            }
        }

        //printf("Value length is: %d\n", value_len);
        value_length_header[0] = (value_len >> 24) & 0xFF;
        value_length_header[1] = (value_len >> 16) & 0xFF;
        value_length_header[2] = (value_len >> 8) & 0xFF;
        value_length_header[3] = value_len & 0xFF;

       // printf("Value length:\n");

        //print_bits(value_length_header[0]);
       // print_bits(value_length_header[1]);
        //print_bits(value_length_header[2]);
       // print_bits(value_length_header[3]);

       // printf("\n");


    } else {
        memset(value_length_header,0, sizeof(int32_t));
    }
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

        break;
    }

    if (p == NULL) {
        fprintf(stderr, "client: failed to connect\n");
        return 2;
    }

    //send data to server

    int tmp = 0;
    int rec = 0;
    // send first byte
    //printf("BEFORE: sending first byte ...\n");
    while(tmp < CALL_LENGTH_SIZE){
        if((rec = send(sockfd,call_header,CALL_LENGTH_SIZE-tmp,0)) == -1){
            perror("ERROR:sending first byte\n");
            exit(1);
        }
        tmp = tmp + rec;
        //printf("%d\n",tmp);
    }
    //printf("%d\n",tmp);

   // printf("AFTER: first byte has been sent, byte sent is: \n");
   // print_bits(call_header[0]);

    tmp = 0;
    rec = 0;
    // sending key length

    //printf("BEFORE: sending key length header \n");
    while(tmp < KEY_LENGTH_SIZE) {
        if((rec = send(sockfd, key_length_header, KEY_LENGTH_SIZE-tmp,0)) == -1) {
            perror("ERROR: sending key length\n");
            exit(1);
        }
        tmp = tmp + rec;
    }
    //printf("Bytes that should ve been send: \n");
    //for(int i = 0; i < KEY_LENGTH_SIZE; i++) {
       // print_bits(key_length_header[i]);
   // }

    //printf("AFTER: sending key length header \n");
    //printf("BEFORE: sending value length size \n ");


    tmp = 0;
    rec= 0;
    //sending value length

   // printf("Bytes that will be sent: \n");
    //for(int i = 0; i < VALUE_LENGTH_SIZE; i++) {
     //   print_bits(value_length_header[i]);
   // }

    while(tmp < VALUE_LENGTH_SIZE) {
        if((rec = send(sockfd, value_length_header,VALUE_LENGTH_SIZE-tmp,0)) == -1) {
            perror("error: sending value length\n");
            exit(1);
        }
        tmp = tmp + rec;
    }

    //printf("Bytes that should ve been send: \n");
    //for(int i = 0; i < VALUE_LENGTH_SIZE; i++) {
    //    print_bits(value_length_header[i]);
    //}

   // printf("AFTER: sending value length size \n");
    //printf("BEFORE: sending key\n");


    //send key
    tmp = 0;
    rec = 0;
    while(tmp < key_len) {
        if((rec = send(sockfd, res_key, key_len - tmp, 0)) == -1) {
            perror("ERROR: sending key\n");
            exit(1);
        }
        tmp = tmp + rec;
    }

   // printf("AFTER: sending key \n");
    //printf("BEFORE: sending value \n");

    if(strcmp(call_type,"SET")==0) {
        //send value
        tmp = 0;
        rec = 0;
        char *value[value_len];
        while (tmp < value_len) {
            if ((rec = send(sockfd, value_buf, value_len - tmp, 0)) == -1) {
                perror("ERROR: sending value");
                exit(1);
            }
            tmp = tmp + rec;
        }

        //printf("AFTER: sending value\n");
    }
    freeaddrinfo(servinfo); // all done with this structure

    //printf("---------------------- ALL DATA SENT; READY TO RECEIVE DATE FROM SERVER --------------------\n");

    //receive header from server, with potential acknowledgment
    tmp = 0;
    rec = 0;
    while(tmp < HEADER_SIZE) {
        if ((rec = recv(sockfd, buf, 7, 0)) == -1) {
            perror("recv");
            exit(1);
        }
        tmp = tmp + rec;
    }
    print_bits(buf[3]);
    print_bits(buf[4]);
    print_bits(buf[5]);
    print_bits(buf[6]);


    //printf("Received header\n");


    // write value to stdoutput
    if(strcmp(call_type,"GET") == 0) {

        int32_t  received_valuelen = (buf[3] << 24)
                                     | ((buf[4] & 0xFF) << 16)
                                     | ((buf[5] & 0xFF) << 8)
                                     | (buf[6] & 0xFF);

        tmp= 0;
        rec= 0;
        char* received[1];
        while(tmp < (int) received_valuelen) {
            printf("entering inner loop:\n");
            if((rec = recv(sockfd,received, 1,0)) == -1) {
                perror("rec: value");
                exit(1);
            }
            printf("%s", received[0]);
            tmp = tmp + rec;
        }
    }

    //printf("Printing received header for debugging purposes, not part of handin version \n");
    //for(int i = 0; i < numbytes; i++){
     //   print_bits(buf[i]);
    //}


    close(sockfd);

    free(call_header);
    free(key_length_header);
    free(value_length_header);
    free(value_buf);


    return 0;
}
