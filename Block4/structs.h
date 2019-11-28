//
// Created by valeria on 27.11.19.
//

#ifndef BLOCK4_STRUCTS_H
#define BLOCK4_STRUCTS_H

#include "uthash.h"

struct intern_HT{
    uint16_t hashed_key;
    int fd;
    char* header;
    char* key;
    char* value;
    UT_hash_handle hh;
};

struct HASH_elem{
    char* key;
    uint16_t key_length;
    char* value;
    uint32_t value_length;
    UT_hash_handle hh;
};

struct peer {
    uint16_t node_ID;
    uint32_t node_IP;
    uint16_t node_PORT;
    struct peer* predecessor;
    struct peer* successor;
};

struct ring_message {
    char* commands;
    uint16_t hash_ID;
    uint16_t node_ID;
    uint32_t node_IP;
    uint16_t node_PORT;
};

#endif //BLOCK4_STRUCTS_H
