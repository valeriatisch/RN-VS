//
// Created by valeria on 24.11.19.
//

#ifndef BLOCK4_HASHTABLEFUNCS4_H
#define BLOCK4_HASHTABLEFUNCS4_H

#include <stdio.h>
#include "uthash.h"

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
    HASH_FIND(hh, table, key, keylen, s);
    return s;
}

void delete(char* key, uint16_t keylen){
    struct HASH_elem *s = NULL;
    HASH_FIND(hh, table, key, keylen, s);
    if(s != NULL){
        HASH_DEL(table, s);
        free(s->key);
        free(s->value);
        free(s);
    }

}

void set(char* new_key, uint16_t key_length, char* value, uint32_t value_length){
    struct HASH_elem *q = NULL;
    HASH_FIND(hh, table, new_key, key_length, q);
    if(q != NULL){
        delete(new_key, key_length);
        q = (struct HASH_elem*) malloc(sizeof(struct HASH_elem));
        q->key = (char*) malloc(key_length);
        q->key_length = key_length;
        memcpy(q->key, new_key, key_length);
        HASH_ADD_KEYPTR(hh, table, q->key, q->key_length, q);
    }
    if(q == NULL){
        q = (struct HASH_elem*) malloc(sizeof(struct HASH_elem));
        q->key = (char*) malloc(key_length);
        q->key_length = key_length;
        memcpy(q->key, new_key, key_length);
        HASH_ADD_KEYPTR(hh, table, q->key, q->key_length, q);
    }

    q->value = (char*) malloc(value_length);
    q->value_length = value_length;
    memcpy(q->value, value, value_length);
}

//https://stackoverflow.com/questions/36896420/casting-uint8-t-array-into-uint16-t-value-in-c
uint16_t hash(char* key, uint16_t key_len){
    if(key_len == 1){
        return (uint16_t) ((uint8_t) key[0]);
    }
    return ntohs(*(uint16_t*)key);
}

#endif //BLOCK4_HASHTABLEFUNCS4_H
