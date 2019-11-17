#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <signal.h>
#include <sudo_plugin.h>
#include "uthash.h"

#define HEADERLENGTH 7

struct HASH_elem{
    char* key;
    uint16_t key_length;
    char* value;
    uint32_t value_length;
    UT_hash_handle hh;
};

struct HASH_elem *table = NULL;

struct HASH_elem *get(char* key, uint16_t keylen){
    struct HASH_elem *s = NULL;
    HASH_FIND_STR(table, key, s);
    return s;
}

void delete(char* key, uint16_t keylen){
    struct HASH_elem *s = NULL;
    HASH_FIND_STR( table, key, s);
    if(s != NULL){
        HASH_DEL(table, s);
        free(s);
    }

}

void set(char* new_key, uint16_t key_length, char* value, uint32_t value_length){
    //delete old elem
    struct HASH_elem *q = NULL;
    HASH_FIND_STR( table, new_key, q);
    if(q != NULL) {
        delete(new_key,key_length);
    }
    //alloc verzweifelten struct
    struct HASH_elem *s = malloc(sizeof(struct HASH_elem));
    if(s == NULL){
        perror("alloc:new element");
        exit(EXIT_FAILURE);
    }

    memcpy(&s->key, &new_key, key_length);
    memcpy(&s->value, &value, value_length);
    s->key_length = key_length;
    s->value_length = value_length;

    struct HASH_elem *rep_elem;
    HASH_ADD_KEYPTR(hh,table,s->key,key_length,s);

}

void *get_in_addr(struct sockaddr *sa){
    if (sa->sa_family == AF_INET) {
        return &(((struct sockaddr_in*)sa)->sin_addr);
    }

    return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

char* recv_n_char(int new_fd, int size){
    //TODO: malloc free'n
    //alloc buffer
    char* recv_buf = calloc(1,sizeof(char)*size+1);
    if(recv_buf == NULL){
        perror("alloc:recv_buf");
        exit(EXIT_FAILURE);
    }
    char* buff_ptr = recv_buf;
    //receive 'size' bytes
    while(size > 0){
        ssize_t curr = recv(new_fd, buff_ptr, size, 0);
        if(curr <= 0){
            if(curr == -1){
                perror("An error occurred while receiving.");
                exit(EXIT_FAILURE);
            }
            break;
        }
        size -= curr;
        buff_ptr += curr;
    }
    return recv_buf;
}

void send_n_char(int new_fd, char* arr, int size){

    char* ptr = arr;
    while(size > 0){

        ssize_t curr = send(new_fd, ptr, size, 0);
        if(curr <= 0){
            if(curr == -1){
                perror("An error occurred while sending.");
                exit(EXIT_FAILURE);
            }
            break;
        }
        size -= curr;
        ptr += curr;

    }
}

int main(int argc, char* argv[]){
    int sockfd, new_fd; // listen on sock_fd, new connection on new_fd
    struct addrinfo hints, *servinfo, *p;
    struct sockaddr_storage their_addr; // connector's address information
    socklen_t sin_size;
    struct sigaction sa;
    int yes=1;
    char s[INET6_ADDRSTRLEN];
    int rv;

    char* portNr = argv[1];

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE; // use my IP

    if ((rv = getaddrinfo("localhost", portNr, &hints, &servinfo)) != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
        return 1;
    }

    // loop through all the results and bind to the first we can
    for(p = servinfo; p != NULL; p = p->ai_next) {
        if ((sockfd = socket(p->ai_family, p->ai_socktype,p->ai_protocol)) == -1) {
            perror("server: socket");
            continue;
        }

        if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes,sizeof(int)) == -1) {
            perror("setsockopt");
            exit(1);
        }

        if (bind(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
            close(sockfd);
            perror("server: bind");
            continue;
        }

        break;
    }

    freeaddrinfo(servinfo); // all done with this structure
    if (p == NULL) {
        fprintf(stderr, "server: failed to bind\n");
        exit(1);
    }

    if (listen(sockfd, 5) == -1) {
        perror("listen");
        exit(1);
    }

    while(1) { // main accept() loop
        sin_size = sizeof their_addr;
        new_fd = accept(sockfd, (struct sockaddr *)&their_addr, &sin_size);
        if (new_fd == -1) {
            perror("accept");
            continue;
        }

        //receive header
        char* header = recv_n_char(new_fd, HEADERLENGTH);

        int requested_del = header[0] & 0b1; //wird groesser 0 sein, wenn das delete bit gesetzt ist
        int requested_set = (header[0] >> 1) & 0b1; //
        int requested_get = (header[0] >> 2) & 0b1;

        uint16_t keylen = (header[1] << 8) | header[2];
        uint32_t valuelen = (header[3] << 24)
                            | ((header[4] & 0xFF) << 16)
                            | ((header[5] & 0xFF) << 8)
                            | (header[6] & 0xFF);
        //receive key
        char* key = recv_n_char(new_fd, keylen);
        //receive value
        char* value = NULL;
        if(valuelen > 0) value = recv_n_char(new_fd, valuelen);

        if(requested_del > 0){
            delete(key, keylen);
            header[0] |= 0b1000;
            //send header
            send_n_char(new_fd, header, HEADERLENGTH);
        }
        else if(requested_set > 0){
            set(key, keylen, value, valuelen);
            header[0] |= 0b1000;
            //send header
            send_n_char(new_fd, header, HEADERLENGTH);
        }
        else if(requested_get > 0){
            struct HASH_elem *result = get(key, keylen);
            //no element found
            if(result == NULL){
                //send header without ack
                send_n_char(new_fd, header, HEADERLENGTH);
            }
                //element found
            else{
                uint32_t len = result->value_length;
                header[3] = (len >> 24) & 0xFF;
                header[4] = (len >> 16) & 0xFF;
                header[5] = (len >> 8) & 0xFF;
                header[6] = len & 0xFF;
                header[0] |= 0b1000;
                send_n_char(new_fd, header, HEADERLENGTH);
                send_n_char(new_fd, result->key, keylen);
                send_n_char(new_fd, result->value, result->value_length);
            }
        }
        close(new_fd); // parent doesn't need this
    }
    return 0;
}
