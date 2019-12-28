#include "../include/fingertable.h"
#include "../include/sockUtils.h"
#include "../include/lookup.h"
#include "../include/peerClientStore.h"
#include "../include/hash.h"
#include "../include/packet.h"
#include "../include/clientStore.h"
#include "../include/sockUtils.h"


int formula(uint16_t my_id, int i){
    return (my_id + (int) pow(2, i)) % (int) pow(2, 16);
}

ft* create_ft_item(serverArgs* args){

    ft* item = malloc(sizeof(ft));

    /** TODO **/

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

void print_fingertable(serverArgs* args, ft** fingertable){
    for(int i = 0; i < FT_SIZE; i++) {
        printf("i: %d start: %d ft[i]: %d\n", i, formula(args->ownID, i), fingertable[i]->id);
    }
}