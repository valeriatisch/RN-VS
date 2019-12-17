#include "../include/fingertable.h"
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
        
        int start = formula(args->ownID, i);
        
    }

    return fingertable;
}
