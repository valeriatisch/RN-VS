/*
** selectserver.c -- a cheezy multiperson chat server
*/

#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <errno.h>
#include <sys/wait.h>
#include <signal.h>
#include <sudo_plugin.h>

#include "communicationfuncs4.h"
#include "hashtablefuncs4.h"

#define HEADERLENGTH 7

void *get_in_addr(struct sockaddr *sa){
    if (sa->sa_family == AF_INET) {
        return &(((struct sockaddr_in*)sa)->sin_addr);
    }
    return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

int main(int argc, char* argv[]){

    if(argc != 10){
        perror("Input is wrong.");
        exit(EXIT_FAILURE);
    }

    fd_set master;    // master file descriptor list
    fd_set read_fds;  // temp file descriptor list for select()
    int fdmax;        // maximum file descriptor number

    int listener;     // listening socket descriptor
    int newfd;        // newly accept()ed socket descriptor
    struct sockaddr_storage remoteaddr; // client address
    socklen_t addrlen;

    char buf[256];    // buffer for client data
    int nbytes;

    char remoteIP[INET6_ADDRSTRLEN];
    int yes=1;        // for setsockopt() SO_REUSEADDR, below
    int i, j, rv;

    struct addrinfo hints, *ai, *p;

    struct peer* self;

    self->node_ID = atoi(argv[1]); 
    self->node_IP = inet_network(argv[2]); //converts a string in IPv4 numbers-and-dots notation host byte order
    self->node_PORT = atoi(argv[3]);
    // I don't know yet if we're gonna need all this crap
    self->predecessor->node_ID = atoi(argv[4]);
    self->predecessor->node_IP = inet_addr(argv[5]);
    self->predecessor->node_PORT = atoi(argv[6]);
    self->successor->node_ID = atoi(argv[7]);
    self->successor->node_IP = inet_addr(argv[8]);
    self->successor->node_PORT = atoi(argv[9]);

    FD_ZERO(&master);    // clear the master and temp sets
    FD_ZERO(&read_fds);

    // get us a socket and bind it
    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;
    if ((rv = getaddrinfo(self->node_IP, self->node_PORT, &hints, &ai)) != 0) {
        fprintf(stderr, "selectserver: %s\n", gai_strerror(rv));
        exit(1);
    }

    for(p = ai; p != NULL; p = p->ai_next) {
        listener = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
        if (listener < 0) {
            continue;
        }

        // lose the pesky "address already in use" error message
        setsockopt(listener, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int));

        if (bind(listener, p->ai_addr, p->ai_addrlen) < 0) {
            close(listener);
            continue;
        }

        break;
    }

    // if we got here, it means we didn't get bound
    if (p == NULL) {
        fprintf(stderr, "selectserver: failed to bind\n");
        exit(2);
    }
                 
                

    freeaddrinfo(ai); // all done with this

    // listen
    if (listen(listener, 10) == -1) {
        perror("listen");
        exit(3);
    }

    // add the listen
    FD_SET(listener, &master);

    // keep track of the biggest file descriptor
    fdmax = listener; // so far, it's this one

    // main loop
    for(;;) {
        read_fds = master; // copy it
        if (select(fdmax + 1, &read_fds, NULL, NULL, NULL) == -1) {
            perror("select");
            exit(4);
        }

        // run through the existing connections looking for data to read
        for (i = 0; i <= fdmax; i++) {
            if (FD_ISSET(i, &read_fds)) { // we got one!!
                if (i == listener) {
                    // handle new connections
                    addrlen = sizeof remoteaddr;
                    newfd = accept(listener,
                                   (struct sockaddr *) &remoteaddr,
                                   &addrlen);

                    if (newfd == -1) {
                        perror("accept");
                    } else {
                        FD_SET(newfd, &master); // add to master set
                        if (newfd > fdmax) {    // keep track of the max
                            fdmax = newfd;
                        }
                        printf("selectserver: new connection from %s on "
                               "socket %d\n",
                               inet_ntop(remoteaddr.ss_family,
                                         get_in_addr((struct sockaddr *) &remoteaddr),
                                         remoteIP, INET6_ADDRSTRLEN),
                               newfd);
                    }
                } else {
                
                    /*
                     * TODO: this is where the fun begins *imagine an Anakin-Skywalker-GIF!*
                     */
                    char *commands = recv_n_char(i, 1); //recv the first byte, the commands, so we can check what kind of a protocol it is
                    int control = (commands[0] >> 7) & 0b1; //check the first bit
                    if (control <= 0) { //so it's the old protocol

                        int ack = (commands[0] >> 3) & 0b1;
                        if (ack <= 0) { //if the ack bit isn't set it's a request from a client
                            // receive header
                            char *rest_header = recv_n_char(i, HEADERLENGTH - 1);
                            char *header = strcat(commands, rest_header);
                            
                            //receive key
                            uint16_t keylen = (header[1] << 8) | header[2];
                            char *key = recv_n_char(i, keylen);
                            
                            //get call type from header get/set/del
                            int requested_del = header[0] & 0b1; //wird groesser 0 sein, wenn das delete bit gesetzt ist
                            int requested_set = (header[0] >> 1) & 0b1; //
                            int requested_get = (header[0] >> 2) & 0b1;

                            uint16_t hashed_key = hash(key, keylen); //hash key into binary

                            struct intern_HT* new_elem = malloc(sizeof(struct intern_HT));
                            new_elem->hashed_key = hashed_key;
                            new_elem->fd = i;
                            memcpy(new_elem->header, header, sizeof header);
                            memcpy(new_elem->key, key, sizeof key);

                            //get value-length and receive value
                            uint32_t valuelen = 0;
                            char* value = NULL;
                            if(requested_set > 0){
                                valuelen = (header[3] << 24)
                                                    | ((header[4] & 0xFF) << 16)
                                                    | ((header[5] & 0xFF) << 8)
                                                    | (header[6] & 0xFF);
                                value = recv_n_char(i, valuelen);
                                memcpy(new_elem->value, value, sizeof value);

                            }
                            //I am responsible
                            if (check_datarange(hashed_key, self->node_ID, self->successor->node_ID, self->predecessor->node_ID) == 1) {                             
                                send_message2client(header, i, HEADERLENGTH, keylen, key, valuelen, value);
                            }
                             //my successor is responsible
                            else if (check_datarange(hashed_key, self->node_ID, self->successor->node_ID, self->predecessor->node_ID) == 2) {
                                //get file decsriptor of my successor, who's responsible
                                int peer_fd = get_fd(self->successor->node_IP, self->successor->node_PORT);
                                //send_ringmessage(peer_fd, create_reply(hashed_key, self->successor)); ??
                                //send him cliet's request
                                if(requested_set > 0) {
                                    send_n_char(peer_fd,header,HEADERLENGTH);
                                    send_n_char(peer_fd,key,keylen);
                                    send_n_char(peer_fd,value,valuelen);
                                }
                                else{
                                    send_n_char(peer_fd,header,HEADERLENGTH);
                                    send_n_char(peer_fd,key,keylen);
                                }
                            }
                            //lookup for next peer
                            else if (check_datarange(hashed_key, self->node_ID, self->successor->node_ID, self->predecessor->node_ID) == 3) {
                                int peer_fd = get_fd(self->successor->node_IP, self->successor->node_PORT);
                                send_ringmessage(peer_fd, create_lookup(hashed_key, self));
                            }
                        } else if (ack > 0) { //it's a reply from another peer then //TODO: was soll hier wirklich passieren?
                            //TODO: was soll hier recvt werden?

                            /*
                             * use recv_ringmessage to recv the new protocol
                             * struct ring_message* x = recv_ringmessage(i);
                            */

                           /*
                            //receive key
                            uint16_t key = recv_n_char(i,2);
                            //receive node ID from responsible node
                            uint16_t node_ID = recv_n_char(i,2);
                            //receive node IP from responsible node
                            uint32_t node_IP = recv_n_char(i,4);
                            //receive node PORT from responsible node
                            uint16_t node_PORT = recv_n_char(i,2);
                            //hash key into binary, just hash with 2??? we have no keylength here
                            uint16_t hashed_key = hash(key, sizeof key); 
                            
                            // access intern hash table;
                            struct intern_HT *new_elem = intern_get(reply->hash_ID);
                            send_message2client(new_elem->header, new_elem->fd, HEADERLENGTH, sizeof(new_elem->key), new_elem->key,
                             sizeof(new_elem->value), new_elem->value);
                             */
                        }
                    }
                        //it's the new protocol!
                    else if (control > 0) {
                        if ((commands[0] >> 1) & 0b1) { // make sure it's a reply

                            //recv intern reply message (contains the ip and port of responsible peer)
                            struct ring_message* reply = recv_ringmessage(i);
                            //get file descriptor of the responsible peer
                            int rpeer_fd = get_fd(reply->node_IP, reply->node_PORT);
                            //get client's request from intern hashtable
                            struct intern_HT* clients_request = intern_get(reply->hash_ID);
                            //send client's request to responsible peer
                            send_n_char(rpeer_fd, clients_request->header, HEADERLENGTH);
                            send_n_char(rpeer_fd, clients_request->key, sizeof clients_request->key);
                            if(((clients_request->header[0] >> 1) & 0b1) > 0){ //send value only if set-bit is set
                                end_n_char(rpeer_fd, clients_request->value, sizeof clients_request->value);

                            }

                            /*
                            int call_type;
                            if(requested_del > 0) call_type = 0;
                            else if(requested_set > 0) call_type = 1;
                            else if(requested_get > 0) call_type = 2;
                            */

                            //char *lookup_message = make_old_from_new(reply->hash_ID, call_type); //TODO

                            //send_toPeer(node_id, node_ip, node_port, lookup_message);//TODO

                        } else if ((commands[0]) & 0b1) { // make sure it's a lookup
                            // check_datarange(hash_key, self->node_ID, successor->node_ID, predecessor->node_ID) == 2 aka my succ is responsible --> reply, else forward lookup
                            struct ring_message* lookup_msg = recv_ringmessage(i);

                            if(check_datarange(lookup_msg->hash_ID,self->node_ID,self->successor->node_ID,self->predecessor->node_ID) == 1){
                                //never
                                perror("check_datarange");
                            }
                            else if(check_datarange(lookup_msg->hash_ID,self->node_ID,self->successor->node_ID,self->predecessor->node_ID) == 2){
                                //next node is responsible, send responsible node to first node
                                int peer_fd = get_fd(lookup_msg->node_IP,lookup_msg->node_PORT);
                                send_ringmessage(peer_fd, create_reply(lookup_msg->hash_ID, self->successor));

                            }
                            else if(check_datarange(lookup_msg->hash_ID,self->node_ID,self->successor->node_ID,self->predecessor->node_ID) == 3){
                                //send lookup to next node
                                //get fd of my successor
                                int peer_fd = get_fd(self->successor->node_IP, self->successor->node_PORT);
                                //send my successor the same lookup-message I have received
                                send_ringmessage(peer_fd, lookup_msg);                              

                            } 
                        } 
                    } 
                }// END handle data from client
            }// END got new incoming connection
        }// END looping through file descriptors 
    }// END for(;;)--and you thought it would never end!             
    return 0;
    
}
