#ifndef BLOCK4_FINGERTABLE_H
#define BLOCK4_FINGERTABLE_H

#include <stdlib.h>
#include <stdint.h>
#include <math.h>

#include "../include/sockUtils.h"
#include "../include/sockUtils.h"
#include "../include/lookup.h"
#include "../include/peerClientStore.h"
#include "../include/hash.h"
#include "../include/packet.h"
#include "../include/clientStore.h"
#include "../include/sockUtils.h"

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
int fingertable_full(ft** fingertable);
int ft_index_of_peer(ft** fingertable, int hash, int own_id);
void print_fingertable(serverArgs* args, ft** fingertable);


#endif //BLOCK4_FINGERTABLE_H
