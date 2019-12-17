#ifndef BLOCK4_FINGERTABLE_H
#define BLOCK4_FINGERTABLE_H

#include <stdlib.h>
#include <stdint.h>
#include <math.h>

typedef struct ft {
    int start;
    uint16_t id;
    uint16_t port;
    uint32_t ip;
} ft;

int formula(uint16_t my_id, int i);

#endif //BLOCK4_FINGERTABLE_H
