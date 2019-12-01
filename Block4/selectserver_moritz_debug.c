#include "communicationfuncs4.h"
#include "hashtablefuncs4.h"

#define HEADERLENGTH 7

void sigchld_handler(int s){
// waitpid() might overwrite errno, so we save and restore it:
    int saved_errno = errno;

    while(waitpid(-1, NULL, WNOHANG) > 0);

    errno = saved_errno;
}

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
    struct sigaction sa;

    char buf[256];    // buffer for client data
    int nbytes;

    char remoteIP[INET6_ADDRSTRLEN];

    int yes=1;        // for setsockopt() SO_REUSEADDR, below
    int i, j, rv;

    struct addrinfo hints, *ai, *p;

    struct peer* self = malloc(sizeof(struct peer));
    self->predecessor = malloc(sizeof(struct peer));
    self->successor = malloc(sizeof(struct peer));

    self->node_ID = atoi(argv[1]);
    self->node_IP = ip_to_uint(argv[2]); //converts a string in IPv4 numbers-and-dots notation host byte order
    self->node_PORT = atoi(argv[3]);
    self->predecessor->node_ID = atoi(argv[4]);
    self->predecessor->node_IP = ip_to_uint(argv[5]);
    self->predecessor->node_PORT = atoi(argv[6]);
    self->successor->node_ID = atoi(argv[7]);
    self->successor->node_IP = ip_to_uint(argv[8]);
    self->successor->node_PORT = atoi(argv[9]);
    char* test_port = argv[9];

    FD_ZERO(&master);    // clear the master and temp sets
    FD_ZERO(&read_fds);

    // get us a socket and bind it
    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;
    char* IP = ip_to_str(self->node_IP);
    char PORT[sizeof(uint16_t)];
    sprintf(PORT, "%d", self->node_PORT);
    if ((rv = getaddrinfo(IP, PORT, &hints, &ai)) != 0) {
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

    sa.sa_handler = sigchld_handler; // reap all dead processes
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESTART;
    if (sigaction(SIGCHLD, &sa, NULL) == -1) {
        perror("sigaction");
        exit(1);
    }

    // main loop
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wmissing-noreturn"
    for(;;) {
        addrlen = sizeof(remoteaddr);
        i = accept(listener,(struct sockaddr * )&remoteaddr, &addrlen);
        if (i == -1) {
            perror("accept");
            continue;
        }

        inet_ntop(remoteaddr.ss_family,get_in_addr((struct sockaddr *)&remoteaddr),remoteIP, sizeof remoteIP);
        printf("server: got connection from %s\n", remoteIP);

        if(!fork()){

            char* commands = recv_n_char(i, 1); //recv the first byte, the commands, so we can check what kind of a protocol it is
            int control = (commands[0] >> 7) & 0b1; //check the first bit

            int rpeer_fd = 0;

            // it's the old protocol
            if (control <= 0) {

                //https://stackoverflow.com/questions/9746866/how-to-concat-two-char-in-c
                //receive complete header
                printf("test0\n");
                char* header = malloc(HEADERLENGTH);
                //strcpy(header,commands);
                printf("hi1\n");
                char *rest_header = recv_n_char(i, HEADERLENGTH - 1);
                printf("hi2\n");
                //printf("%d\n", rest_header[1]);
                //header = strcat(header, rest_header);
                memcpy(header, commands, 1);
                memcpy(header+1, rest_header, HEADERLENGTH - 1);
                //receive key
                uint16_t keylen = (header[1] << 8) | header[2];
                printf("hi3\n");
                char *key = recv_n_char(i, keylen);
                printf("hi4\n");

                //get call type from header get/set/del
                int requested_del = header[0] & 0b1;
                int requested_set = (header[0] >> 1) & 0b1;
                int requested_get = (header[0] >> 2) & 0b1;
                //hash key into binary
                uint16_t hashed_key = hash(key, keylen);

                int ack = (commands[0] >> 3) & 0b1;
                // ack bit not set, therefore request from client
                if (ack <= 0) {
                    printf("test1\n");
                    struct intern_HT* new_elem = malloc(sizeof(struct intern_HT));
                    new_elem->hashed_key = hashed_key;
                    new_elem->fd = i;
                    new_elem->header = malloc(HEADERLENGTH);
                    memcpy(new_elem->header, header, HEADERLENGTH);
                    new_elem->key = malloc(keylen); //eventuell +1
                    memcpy(new_elem->key, key, keylen);
                    printf("test1.2\n");

                    //get value-length and receive value
                    uint32_t valuelen = 0;
                    char* value = NULL;
                    new_elem->value = NULL;
                    if(requested_set > 0){
                        valuelen = (header[3] << 24)
                                   | ((header[4] & 0xFF) << 16)
                                   | ((header[5] & 0xFF) << 8)
                                   | (header[6] & 0xFF);
                        printf("hi5\n");
                        value = recv_n_char(i, valuelen);
                        printf("hi6\n");
                        new_elem->value = malloc(sizeof(value)); //eventuell +1
                        memcpy(new_elem->value, value, sizeof value);
                    }
                    printf("test1.3\n");
                    intern_set(new_elem);
                    printf("test1.4\n");
                    //I am responsible
                    if (check_datarange(hashed_key, self->node_ID, self->successor->node_ID, self->predecessor->node_ID) == 1) {
                        printf("test1.5\n");
                        printf("m2c\n");

                        char* new_port = malloc(999);
                        snprintf(new_port, 10000,"%d", self->predecessor->node_PORT);
                        int new_fd = get_fd(self->predecessor->node_IP,new_port);

                        printf("SHould be first node(new fd): \n");
                        print_peer_information(new_fd);
                        send_message2client(header, new_fd, HEADERLENGTH, keylen, key, valuelen, value);
                        //send_message2client(header, i, HEADERLENGTH, keylen, key, valuelen, value);
                        printf("m2c...\n");


                        close(i);
                        FD_CLR(i, &master);
                    }


                        //my successor is responsible
                    else if (check_datarange(hashed_key, self->node_ID, self->successor->node_ID, self->predecessor->node_ID) == 2) {
                        printf("client information: \n");
                        print_peer_information(i);
                        printf("test1.6\n");
                        //send_ringmessage(listener,create_reply(hashed_key, self));
                        printf("test2\n");

                        //get file decsriptor of my successor, who's responsible
                        int peer_fd = get_fd(self->successor->node_IP, test_port);

                        printf("Will send to:\n");
                        print_peer_information(peer_fd);

                        //send him client's request
                        if(requested_set > 0) {
                            printf("keylen:%d\nvaluelen:%d\n",keylen,valuelen);
                            send_n_char(peer_fd,header,HEADERLENGTH);
                            send_n_char(peer_fd,key,keylen);
                            send_n_char(peer_fd,value,valuelen);
                        }
                        else{
                            send_n_char(peer_fd,header,HEADERLENGTH);
                            send_n_char(peer_fd,key,keylen);
                        }
                        printf("test2\n");
                        close(peer_fd);

                    }
                        //lookup for next peer
                    else if (check_datarange(hashed_key, self->node_ID, self->successor->node_ID, self->predecessor->node_ID) == 3) {
                        printf("test lookup send\n");
                        int peer_fd = get_fd(self->successor->node_IP, test_port);
                        send_ringmessage(peer_fd, create_lookup(hashed_key, self));
                        close(peer_fd);
                        printf("test3\n");
                    }
                } else if (ack > 0) { //it's a reply(old-protocol) from another peer -> send to client
                    printf("test koennen wir das\n");
                    struct intern_HT* new_elem = intern_get(hashed_key);
                    int fd = new_elem->fd;

                    printf("client information from hash table:\n");
                    print_peer_information(fd);

                    //send protocol to client
                    send_n_char(fd,header,HEADERLENGTH);

                    // receive valuelength and value and send to client
                    if(requested_get > 0){
                        uint32_t valuelen = (header[3] << 24)
                                            | ((header[4] & 0xFF) << 16)
                                            | ((header[5] & 0xFF) << 8)
                                            | (header[6] & 0xFF);
                        char* value = recv_n_char(i, valuelen);
                        send_n_char(fd,key,keylen);
                        send_n_char(fd,value,valuelen);
                    }

                    close(fd);
                    close(rpeer_fd);
                    close(i);
                    FD_CLR(i, &master);
                }
            }
                //it's the new protocol!
            else if (control > 0) {

                // make sure it's a reply
                if ((commands[0] >> 1) & 0b1) {

                    //recv intern reply message (contains the ip and port of responsible peer)
                    struct ring_message* reply = recv_ringmessage(i);

                    //get file descriptor of the responsible peer
                    char p[3];
                    sprintf(p, "%u", reply->node_PORT);
                    rpeer_fd = get_fd(reply->node_IP, p);

                    //get client's request from intern hashtable
                    struct intern_HT* clients_request = intern_get(reply->hash_ID);

                    //send client's request to responsible peer
                    send_n_char(rpeer_fd, clients_request->header, HEADERLENGTH);
                    send_n_char(rpeer_fd, clients_request->key, sizeof clients_request->key);

                    //send value only if set-bit is set
                    if(((clients_request->header[0] >> 1) & 0b1) > 0){
                        send_n_char(rpeer_fd, clients_request->value, sizeof clients_request->value);
                    }
                    //make sure it's a lookup
                } else if ((commands[0]) & 0b1) {
                    struct ring_message* lookup_msg = recv_ringmessage(i);

                    //never, check just in case
                    if(check_datarange(lookup_msg->hash_ID,self->node_ID,self->successor->node_ID,self->predecessor->node_ID) == 1){
                        perror("check_datarange");
                    }
                        //next node is responsible, send responsible node to first node
                    else if(check_datarange(lookup_msg->hash_ID,self->node_ID,self->successor->node_ID,self->predecessor->node_ID) == 2){
                        char p[3];
                        sprintf(p, "%u", lookup_msg->node_PORT);
                        int peer_fd = get_fd(lookup_msg->node_IP,lookup_msg->node_PORT);
                        send_ringmessage(peer_fd, create_reply(lookup_msg->hash_ID, self->successor));
                        close(peer_fd);
                    }
                        //send lookup to next node
                    else if(check_datarange(lookup_msg->hash_ID,self->node_ID,self->successor->node_ID,self->predecessor->node_ID) == 3){

                        //get fd of my successor and send the lookup
                        int peer_fd = get_fd(self->successor->node_IP, self->successor->node_PORT);
                        send_ringmessage(peer_fd, lookup_msg);
                        close(peer_fd);
                    }
                }
            }
        }
        // END handle data from client
        // END got new incoming connection
        // END looping through file descriptors
    }// END for(;;)--and you thought it would never end!
#pragma clang diagnostic pop
    return 0;

}

