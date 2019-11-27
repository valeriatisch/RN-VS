//
// Created by valeria on 24.11.19.
//

#ifndef BLOCK4_COMMUNICATIONFUNCS4_H
#define BLOCK4_COMMUNICATIONFUNCS4_H

#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include "hashtablefuncs4.h"

struct peer {
    uint16_t node_ID;
    uint32_t node_IP;
    uint16_t node_PORT;
};

struct ring_message {
    char* commands;
    uint16_t hash_ID;
    uint16_t node_ID;
    uint32_t node_IP;
    uint16_t node_PORT;
};

char* recv_n_char(int new_fd, int size);
void send_n_char(int new_fd, char* arr, int size);
void send_message2client(char* header, int  new_fd, int headerlength);
int check_datarange(uint16_t hash_key, uint16_t self_ID, uint16_t successor_ID, uint16_t predecessor_ID);
struct ring_message* create_lookup(uint16_t hashed_key, struct peer* p);
struct ring_message* create_reply(uint16_t hashed_key, struct peer* successor);
void sendringmessage2peer(int new_fd, struct ring_message* msg);    


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

void send_message2client(char* header, int  new_fd, int headerlength){
    int requested_del = header[0] & 0b1; //wird groesser 0 sein, wenn das delete bit gesetzt ist
    int requested_set = (header[0] >> 1) & 0b1; //same shit with set
    int requested_get = (header[0] >> 2) & 0b1; //and get

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
        send_n_char(new_fd, header, headerlength);
    }
    else if(requested_set > 0){
        set(key, keylen, value, valuelen);
        header[0] |= 0b1000;
        //send header
        send_n_char(new_fd, header, headerlength);
    }
    else if(requested_get > 0){
        struct HASH_elem *result = get(key, keylen);
        header[0] |= 0b1000;
        //no element found
        if(result == NULL){
            //send header without ack
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
            send_n_char(new_fd, result->key, keylen);
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

struct ring_message* create_reply(uint16_t hashed_key, struct peer* successor){

    struct ring_message* msg = (struct ring_message*) malloc(sizeof(struct ring_message));

    msg->commands[0] |= 0b10000010; //set control bit and reply bit
    msg->hash_ID = hashed_key;
    msg->node_ID = successor->node_ID;
    msg->node_IP = successor->node_IP;
    msg->node_PORT = successor->node_PORT;

    return msg;
}

void send_ringmessage2peer(int new_fd, struct ring_message* msg){
    send_n_char(new_fd, msg->commands, sizeof(msg->commands));
    send_n_char(new_fd, &msg->node_ID, sizeof(uint16_t));
    send_n_char(new_fd, &msg->node_IP, sizeof(uint32_t));
    send_n_char(new_fd, &msg->node_PORT, sizeof(uint16_t));
}

#endif //BLOCK4_COMMUNICATIONFUNCS4_H
