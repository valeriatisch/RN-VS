/*
** selectserver.c -- a cheezy multiperson chat server
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <errno.h>
#include <sys/wait.h>
#include <signal.h>
#include <sudo_plugin.h>
#include "uthash.h"

#define HEADERLENGTH 7

struct ring_message {
    char* commands; //spaeter malloc mit 7
    uint8_t* hash_ID; //oder uint8_t hash_ID[2] oder uint16_t hash_ID, spaeter auf jeden Fall malloc 2 Bytes
    uint16_t node_ID;
    uint32_t node_IP;
    uint16_t node_PORT;
};

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

    if(argc != 10){
        perror("Input is wrong.");
        exit(EXIT_FAILURE);
    }

    fd_set master;    // master file descriptor list
    fd_set read_fds;  // temp file descriptor list for select()
    int fdmax;        // maximum file descriptor number

    int listener;     // listening socket descriptor
    int newfd;        // newly accept()ed socket descriptor
    struct sockaddr_storage remoteaddr; // client address
    socklen_t addrlen;

    char buf[256];    // buffer for client data
    int nbytes;

    char remoteIP[INET6_ADDRSTRLEN];

    int yes=1;        // for setsockopt() SO_REUSEADDR, below
    int i, j, rv;

    struct addrinfo hints, *ai, *p;

    struct ring_message self;
    struct ring_message pre;
    struct ring_message succ;

    self.node_ID = atoi(argv[1]);
    pre.node_ID = atoi(argv[4]);
    succ.node_ID = atoi(argv[7]);
    self.node_PORT = argv[3];

    FD_ZERO(&master);    // clear the master and temp sets
    FD_ZERO(&read_fds);

    // get us a socket and bind it
    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;
    if ((rv = getaddrinfo(self.node_IP, self.node_PORT, &hints, &ai)) != 0) {
        fprintf(stderr, "selectserver: %s\n", gai_strerror(rv));
        exit(1);
    }

    for(p = ai; p != NULL; p = p->ai_next) {
        listener = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
        if (listener < 0) {
            continue;
        }

        // lose the pesky "address already in use" error message
        setsockopt(listener, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int));

        if (bind(listener, p->ai_addr, p->ai_addrlen) < 0) {
            close(listener);
            continue;
        }

        break;
    }

    // if we got here, it means we didn't get bound
    if (p == NULL) {
        fprintf(stderr, "selectserver: failed to bind\n");
        exit(2);
    }

    freeaddrinfo(ai); // all done with this

    // listen
    if (listen(listener, 10) == -1) {
        perror("listen");
        exit(3);
    }

    // add the listener to the master set
    FD_SET(listener, &master);

    // keep track of the biggest file descriptor
    fdmax = listener; // so far, it's this one

    // main loop
    for(;;) {
        read_fds = master; // copy it
        if (select(fdmax+1, &read_fds, NULL, NULL, NULL) == -1) {
            perror("select");
            exit(4);
        }

        // run through the existing connections looking for data to read
        for(i = 0; i <= fdmax; i++) {
            if (FD_ISSET(i, &read_fds)) { // we got one!!
                if (i == listener) {
                    // handle new connections
                    addrlen = sizeof remoteaddr;
                    newfd = accept(listener,
                                   (struct sockaddr *)&remoteaddr,
                                   &addrlen);

                    if (newfd == -1) {
                        perror("accept");
                    } else {
                        FD_SET(newfd, &master); // add to master set
                        if (newfd > fdmax) {    // keep track of the max
                            fdmax = newfd;
                        }
                        printf("selectserver: new connection from %s on "
                               "socket %d\n",
                               inet_ntop(remoteaddr.ss_family,
                                         get_in_addr((struct sockaddr*)&remoteaddr),
                                         remoteIP, INET6_ADDRSTRLEN),
                               newfd);
                    }
                } else {
                    // handle data from a client
                    if ((nbytes = recv(i, buf, sizeof buf, 0)) <= 0) {
                        // got error or connection closed by client
                        if (nbytes == 0) {
                            // connection closed
                            printf("selectserver: socket %d hung up\n", i);
                        } else {
                            perror("recv");
                        }
                        close(i); // bye!
                        FD_CLR(i, &master); // remove from master set
                    } else {
                        // we got some data from a client
                        for(j = 0; j <= fdmax; j++) {
                            // send to everyone!
                            if (FD_ISSET(j, &master)) {
                                // except the listener and ourselves
                                if (j != listener && j != i) {
                                    if (send(j, buf, nbytes, 0) == -1) {
                                        perror("send");
                                    }
                                }
                            }
                        }
                    }
                } // END handle data from client
            } // END got new incoming connection
        } // END looping through file descriptors
    } // END for(;;)--and you thought it would never end!

    return 0;
}
