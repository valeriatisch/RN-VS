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

#define PORT "3490" // the port client will be connecting to

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
        fprintf(stderr,"usage: client hostname key methode value\n");
        exit(1);
    }
    
    char* res_key = argv[2];
    char* call_type = argv[3];
    void* res_value = argv[4];

    //alloc  
    char* header = calloc(1,sizeof(char)*7);
    
    if(res_value == 'GET') {
        header[0] |= 1UL << 2; 
    } else if(res_value == 'SET') {
        header[0] |= 1UL << 1;
    } else if(res_value == 'DEL') {
        header[0] |= 1UL;
    } else
    {
        perror("invalid command\n");
    }

    //merge header and key
    char* send_data = malloc(sizeof(char)*(strlen(header)+strlen(res_key)));
    if(send_data == NULL){
        perror("alloc:send_data");
        exit(1);
    }
    snprintf(send_data,sizeof(send_data),"%s%s",header,res_key);


    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;

    if ((rv = getaddrinfo(argv[1], PORT, &hints, &servinfo)) != 0) {
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
    //send header + key
    int tmp = 0;
    while(tmp < sizeof(send_data)){
        if(tmp += send(sockfd,send_data,sizeof(send_data)-tmp,0) == -1){
            perror("send:header+key");
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

    //receive data from server
    if ((numbytes = recv(sockfd, buf, MAXDATASIZE-1, 0)) == -1) {
        perror("recv");
        exit(1);
    }

    buf[numbytes] = '\0';

    printf("client: received '%s'\n",buf);

    close(sockfd);

    return 0;
}
