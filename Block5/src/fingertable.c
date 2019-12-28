#include "../include/fingertable.h"


int formula(uint16_t my_id, int i){
    return (my_id + (int) pow(2, i)) % (int) pow(2, 16);
}

ft* create_ft_item(serverArgs* args){

    ft* item = malloc(sizeof(ft));

    item->id = NULL;
    item->ip = NULL;
    item->port = NULL;

    return item;
}

ft** create_ft(serverArgs* args){

    ft** fingertable = malloc(sizeof(ft*) * 16);

    for(int i = 0; i < 16; i++){
        /** TODO **/
        fingertable[i] = create_ft_item(args);

        int start = formula(args->ownID, i);
        fingertable[i]->id = start;

        lookup *ft_message = createLookup(0, 0, 0, 0, 0, 0, 1, start, args->ownID, args->ownIP, args->ownPort);
    }

    return fingertable;
}

int fingertable_full(ft** fingertable){

    for(int i = 0; i < 16; i++){
        if(fingertable[i]->ip == NULL)
            return 0;
    }

    return 1;
}

int ft_index_of_peer(ft** fingertable, int hash, int own_id){

    for(int i = 0; i < FT_SIZE; i++){
        int start = formula(own_id, i);
        if(hash <= fingertable[i]->id && start <= hash)
            return i;
    }

    return -1;
}


void print_fingertable(serverArgs* args, ft** fingertable){
    for(int i = 0; i < FT_SIZE; i++) {
        printf("i: %d start: %d ft[i]: %d\n", i, formula(args->ownID, i), fingertable[i]->id);
    }
}