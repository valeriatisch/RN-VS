//
// Created by valeria on 24.11.19.
//

#ifndef BLOCK4_COMMUNICATIONFUNCS4_H
#define BLOCK4_COMMUNICATIONFUNCS4_H

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <netdb.h>
#include <errno.h>
#include <sys/wait.h>
#include <signal.h>
#include <sudo_plugin.h>
#include "hashtablefuncs4.h"

char* recv_n_char(int new_fd, int size);
void send_n_char(int new_fd, void* arr, int size);
void send_message2client(char* header, int  new_fd, int headerlength, uint16_t _keylen, char* _key, uint32_t _valuelen, char* _value);
int check_datarange(uint16_t hash_key, uint16_t self_ID, uint16_t successor_ID, uint16_t predecessor_ID);
struct ring_message* create_lookup(uint16_t hashed_key, struct peer* p);
struct ring_message* create_reply(uint16_t hashed_key, struct peer* successor);
void sendringmessage(int new_fd, struct ring_message* msg);
void recv_parts_of_rm(int new_fd, void* ptr, int size);
struct ring_message* recv_ringmessage(int new_fd);
int get_fd(uint32_t node_ip, char* node_port);
uint32_t ip_to_uint(char *ip_addr);
char* ip_to_str(uint32_t ip);

uint32_t ip_to_uint(char *ip_addr) {
    struct in_addr ip;
    if (inet_aton(ip_addr, &ip) == 0)
        perror("converting ip to int");
    return ip.s_addr;
}

char* ip_to_str(uint32_t ip){
    struct in_addr ips;
    ips.s_addr = ip;
    char* ip_str = inet_ntoa(ips);
    if (ip_str == NULL)
        perror("converting ip to string");
    return  ip_str;
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

void send_n_char(int new_fd, void* arr, int size){

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

void send_message2client(char* header, int  new_fd, int headerlength, uint16_t _keylen, char* _key, uint32_t _valuelen, char* _value){
    int _requested_del = header[0] & 0b1; //wird groesser 0 sein, wenn das delete bit gesetzt ist
    int _requested_set = (header[0] >> 1) & 0b1; //same shit with set
    int _requested_get = (header[0] >> 2) & 0b1; //and get

    if(_requested_del > 0){
        delete(_key, _keylen);
        header[0] |= 0b1000;
        //send header
        send_n_char(new_fd, header, headerlength);
        send_n_char(new_fd,_key,_keylen);
    }
    else if(_requested_set > 0){
        set(_key, _keylen, _value, _valuelen);
        header[0] |= 0b1000;
        //send header
        send_n_char(new_fd, header, headerlength);
        send_n_char(new_fd,_key,_keylen);
    }
    else if(_requested_get > 0){
        struct HASH_elem *result = get(_key, _keylen);
        header[0] |= 0b1000;
        //no element found
        if(result == NULL){
            send_n_char(new_fd, header, headerlength);
        }
            //element found
        else{
            uint32_t len = result->value_length;
            header[3] = (len >> 24) & 0xFF;
            header[4] = (len >> 16) & 0xFF;
            header[5] = (len >> 8) & 0xFF;
            header[6] = len & 0xFF;
            send_n_char(new_fd, header, headerlength);
            send_n_char(new_fd, result->key, _keylen);
            send_n_char(new_fd, result->value, result->value_length);
        }
    }
}

int check_datarange(uint16_t hash_key, uint16_t self_ID, uint16_t successor_ID, uint16_t predecessor_ID){

    if(predecessor_ID > self_ID){ //I'm the first peer, so pre is larger than me
        if(hash_key > predecessor_ID) return 1; //hash_key is located between my pre and me -> i'm responsible
        if(hash_key <= successor_ID && hash_key > self_ID) return 2; //hash_key is located between my succ and me -> my succ is responsible
    }
    else if(predecessor_ID < self_ID){ //I'm larger than my pre -> all cases except one
        if(hash_key > predecessor_ID && hash_key <= self_ID) return 1; //hash_key is located between my pre and me -> i'm responsible
        if(hash_key <= successor_ID && hash_key > self_ID) return 2; //hash_key is located between my succ and me -> my succ is responsible
    } else return 3; //I can't see anyone who's responsible -> send lookup to the next peer

}

struct ring_message* create_lookup(uint16_t hashed_key, struct peer* p){

    struct ring_message* msg = (struct ring_message*) malloc(sizeof(struct ring_message));

    msg->commands[0] |= 0b10000001; //set control bit and lookup bit
    msg->hash_ID = hashed_key;
    msg->node_ID = p->node_ID;
    msg->node_IP = p->node_IP;
    msg->node_PORT = p->node_PORT;

    return msg;
}

struct ring_message* create_reply(uint16_t hashed_key, struct peer* self){ //hier ist self der Peer, dessen Nachfolger der zustanedige gesuchte Peer ist

    struct ring_message* msg = (struct ring_message*) malloc(sizeof(struct ring_message));

    msg->commands[0] |= 0b10000010; //set control bit and reply bit
    msg->hash_ID = hashed_key;
    msg->node_ID = self->successor->node_ID;
    msg->node_IP = self->successor->node_IP;
    msg->node_PORT = self->successor->node_PORT;

    return msg;
}

void send_ringmessage(int new_fd, struct ring_message* msg){
    send_n_char(new_fd, msg->commands, sizeof(msg->commands));
    send_n_char(new_fd, &msg->hash_ID, sizeof(uint16_t));
    send_n_char(new_fd, &msg->node_ID, sizeof(uint16_t));
    send_n_char(new_fd, &msg->node_IP, sizeof(uint32_t));
    send_n_char(new_fd, &msg->node_PORT, sizeof(uint16_t));
}

//function to receive a part of a ring message e.g. ID
void recv_parts_of_rm(int new_fd, void* ptr, int size) {
    while (size > 0) {
        ssize_t curr = recv(new_fd, ptr, size, 0);
        if (curr <= 0) {
            if (curr == -1) {
                perror("An error occurred while receiving.");
                exit(EXIT_FAILURE);
            }
            break;
        }
        size -= curr;
        ptr += curr;
    }
}

struct ring_message* recv_ringmessage(int new_fd){

    struct ring_message* msg = (struct ring_message*) malloc(sizeof(struct ring_message));

    msg->commands = recv_n_char(new_fd, 1);
    recv_parts_of_rm(new_fd, &msg->hash_ID, sizeof(uint16_t));
    recv_parts_of_rm(new_fd, &msg->node_ID, sizeof(uint16_t));
    recv_parts_of_rm(new_fd, &msg->node_IP, sizeof(uint32_t));
    recv_parts_of_rm(new_fd, &msg->node_PORT, sizeof(uint16_t));

    return msg;
}

int get_fd(uint32_t node_ip, char* node_port){
    int sockfd, new_fd; // listen on sock_fd, new connection on new_fd
    struct addrinfo hints, *servinfo, *p;
    int yes=1;
    int rv;

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE; // use my IP

    char* nodeIP = ip_to_str(node_ip);
    //char* nodePORT = malloc(sizeof(uint16_t)+1);
    //sprintf(nodePORT, "%d", node_port);

    if ((rv = getaddrinfo(nodeIP, node_port, &hints, &servinfo)) != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
        return 1;
    }

    //https://stackoverflow.com/questions/27014955/socket-connect-vs-bind
    // loop through all the results and bind to the first we can
    for(p = servinfo; p != NULL; p = p->ai_next) {
        if ((sockfd = socket(p->ai_family, p->ai_socktype,p->ai_protocol)) == -1) {
            perror("server: socket");
            continue;
        }
        if((new_fd = connect(sockfd, p->ai_addr, p->ai_addrlen)) < 0){
            perror("perror connect");
            return 1;
        }
    }

    freeaddrinfo(servinfo); // all done with this structure

    return sockfd;
}



#endif //BLOCK4_COMMUNICATINFUNCS4_H