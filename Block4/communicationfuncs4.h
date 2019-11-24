//
// Created by valeria on 24.11.19.
//

#ifndef BLOCK4_COMMUNICATIONFUNCS4_H
#define BLOCK4_COMMUNICATIONFUNCS4_H

#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>

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

#endif //BLOCK4_COMMUNICATIONFUNCS4_H
