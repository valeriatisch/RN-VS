//
// Created by valeria on 24.11.19.
//

#ifndef BLOCK4_HASHTABLEFUNCS4_H
#define BLOCK4_HASHTABLEFUNCS4_H

#include "structs.h"
#include "uthash.h"

struct HASH_elem *table = NULL;
struct intern_HT *intern_table = NULL;

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

void intern_delete(uint16_t hashed_key){
    struct intern_HT *s = NULL;
    HASH_FIND_INT(intern_table, &hashed_key, s);
    if(s != NULL){
        HASH_DEL(intern_table, s);
    }
}

void intern_set(struct intern_HT* s){
    struct intern_HT *q = NULL;

    HASH_FIND_INT(intern_table, &s->hashed_key, q);

    if(q != NULL) {
        intern_delete(q->hashed_key);
    }
    q = (struct intern_HT*) malloc(sizeof(struct intern_HT));
    q->hashed_key = s->hashed_key;
    q->fd = s->fd;
    memcpy(q->header, s->header, sizeof(s->header));
    memcpy(q->key, s->key, sizeof s->key);
    memcpy(q->value, s->value, sizeof s->value);
    HASH_ADD_INT(intern_table, hashed_key, q);
}

struct intern_HT* intern_get(uint16_t hashed_key){
    struct intern_HT *s = NULL;
    HASH_FIND_INT(intern_table, &hashed_key, s);
    return s;
}

#endif //BLOCK4_HASHTABLEFUNCS4_H