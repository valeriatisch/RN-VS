#ifndef BLOCK4_FINGERTABLE_H
#define BLOCK4_FINGERTABLE_H

#include <stdlib.h>
#include <stdint.h>
#include <math.h>

#define FT_SIZE 16

typedef struct ft {
    int start;
    uint16_t id;
    uint16_t port;
    uint32_t ip;
} ft;

int formula(uint16_t my_id, int i);
ft* create_ft_item(serverArgs* args);
ft** create_ft(serverArgs* args);
void print_fingertable(serverArgs* args, ft** fingertable);


#endif //BLOCK4_FINGERTABLE_H
