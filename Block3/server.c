//
// Created by valeria on 01.11.19.
//

/*
 * server.c - TCP - vorgegebenes Protokoll
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <signal.h>
#include <stdint.h>

#include "uthash.h"


/* Aus dem Tut:
 * recv buffer mit 7 bytes
 * wissen dann keylen und valuelen
 * recv key und value mit entsprechenden laengen
 * recv gibt nicht mehr oder weniger als keylen und valuelen
 *
 * TODO: hton anschauen
 */

typedef struct {
    void* key;
    uint16_t key_len;
    void* value;
    uint32_t value_len;
    UT_hash_handle hh;
} element;

element* table = NULL;

void set(void* key, void* value,size_t key_len,size_t value_len){
    element* newelem;
    newelem = malloc(sizeof(element));
    memcpy(newelem->key, key, key_len);
    memcpy(newelem->value, value, value_len);
    newelem->key_len = key_len; //das ist eigentlich auch vollkommen in Ordnung
    newelem->value_len = value_len;
    element* rep_item;
    HASH_REPLACE(hh,table,key,key_len,newelem,rep_item);
    if(rep_item != NULL){
        free(rep_item);
    }
}

element* get(void* key,size_t key_len){
    element* search_elem;

    HASH_FIND(hh,table,key,key_len,search_elem);
    return search_elem;
}

void delete(element* del_elem){
    HASH_DEL(table,del_elem);
    free(del_elem);
}

/*
void sigchld_handler(int s) {
    (void)s;
    int saved_errno = errno;
    while(waitpid(-1, NULL, WNOHANG) > 0);
    errno = saved_errno;
}
*/

void *get_in_addr(struct sockaddr *sa) {
    if (sa->sa_family == AF_INET) {
        return &(((struct sockaddr_in*)sa)->sin_addr);
    }

    return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

void char2short(unsigned char* pchar, unsigned short* pshort) {
    *pshort = (pchar[0] << 8) | pchar[1];
}

int main(int argc, char* argv[]) {

    int new_fd,
        sockfd = 0;
    struct addrinfo *servinfo,
            *p;
    struct sockaddr_storage their_addr;
    socklen_t sin_size;
    struct sigaction sa;
    int yes = 1;
    char s[INET6_ADDRSTRLEN];
    uint8_t request[7];
    uint8_t confirmation[7];
    element* pair = (element*) malloc(sizeof(element));
    pair->key = (void*) malloc(sizeof(uint16_t));
    pair->value = (void*) malloc(sizeof(uint32_t));

    if(argc != 2){
        perror("Port number is required.");
        exit(EXIT_FAILURE);
    }

    char* portNr = argv[1];

    struct addrinfo hints = {
            .ai_family = AF_UNSPEC,
            .ai_socktype = SOCK_STREAM,
            .ai_flags = AI_PASSIVE
    };

    int rv = getaddrinfo("localhost", portNr, &hints, &servinfo);
    if(rv != 0){
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
        exit(EXIT_FAILURE);
    }

    for(p = servinfo; p != NULL; p = p->ai_next) {
        if ((sockfd = socket(p->ai_family, p->ai_socktype,
                             p->ai_protocol)) == -1) {
            perror("Couldn't create a socket.");
            continue;
        }
        if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes,
                       sizeof(int)) == -1) {
            perror("Setsockopt() function failed.");
            exit(EXIT_FAILURE);
        }
        if (bind(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
            close(sockfd);
            perror("Server couldn't bind.");
            continue;
        }
        break;
    }

    freeaddrinfo(servinfo);

    if (p == NULL)  {
        fprintf(stderr, "Server failed to bind.\n");
        exit(EXIT_FAILURE);
    }

    if (listen(sockfd, 7) == -1) {
        perror("An error occurred while listening.");
        exit(EXIT_FAILURE);
    }
    /*
    sa.sa_handler = sigchld_handler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESTART;
    if (sigaction(SIGCHLD, &sa, NULL) == -1) {
        perror("sigaction");
        exit(EXIT_FAILURE);
    }
    */
    memset(&request, 0, sizeof request);
    memset(&confirmation, 0, sizeof confirmation); //confirmation

    while(1) {
        sin_size = sizeof their_addr;
        new_fd = accept(sockfd, (struct sockaddr *) &their_addr, &sin_size);
        if (new_fd == -1) {
            perror("An error occurred while accepting.");
            continue;
        }

        inet_ntop(their_addr.ss_family,
                  get_in_addr((struct sockaddr *) &their_addr),
                  s,
                  sizeof s);
        printf("Server got connection from %s\n", s);

        /*
         * TODO: select() http://manpages.ubuntu.com/manpages/cosmic/de/man2/select.2.html
         */

        if (recv(new_fd, &request, sizeof request, 0) == -1) { //hier holen wir uns den Header
            perror("An error occurred while receiving.");
            exit(EXIT_FAILURE);
        }
        int requested_del = request[0] & 0b1; //wird groesser 0 sein, wenn das delete bit gesetzt ist
        int requested_set = (request[0] >> 1) & 0b1; //
        int requested_get = (request[0] >> 2) & 0b1;

        //https://stackoverflow.com/questions/25787349/convert-char-to-short/25787662
        char2short(request, &pair->key_len); //keylength holen

        if(pair->key_len > 0){
            void* ptr_key = &pair->key;
            ssize_t recv_key = recv(new_fd, ptr_key, pair->key_len, 0);
            if (recv_key == -1) {
                perror("An error occurred while receiving the key.");
                exit(EXIT_FAILURE);
            }
        } else if (pair->key_len == 0){
            perror("Key length is 0.");
            exit(EXIT_FAILURE);
        }

        pair->value_len = (request[3] << 24)
                        | ((request[4] & 0xFF) << 16)
                        | ((request[5] & 0xFF) << 8)
                        | (request[6] & 0xFF);

        if(pair->value_len > 0) {
            void* ptr_value = &pair->value;
            ssize_t recv_value = recv(new_fd, ptr_value, pair->value_len, 0);
            if (recv_value == -1) {
                perror("An error occurred while receiving the value.");
                exit(EXIT_FAILURE);
            }
        }

        if(requested_del > 0 || requested_set > 0){
            if(requested_del > 0){
                confirmation[0] |= 0b1001; //del und ack bit setzen
                delete(pair);
            } else if(requested_set > 0){
                confirmation[0] |= 0b1010; //set und ack bit setzen
                set(pair->key, pair->value, pair->key_len, pair->value_len);
            }
            ssize_t sent = send(new_fd, confirmation, sizeof confirmation, 0);
            if(sent != sizeof confirmation){
                perror("An error occurred while sending.");
                exit(EXIT_FAILURE);
            }
        } else if(requested_get > 0){
            confirmation[0] |= 0b1100; //get und ack bit setzen
            element* elem = get(pair->key, pair->key_len);
            ssize_t sent_conf = send(new_fd, confirmation, sizeof confirmation, 0);
            if(sent_conf != sizeof confirmation){
                perror("An error occurred while sending.");
                exit(EXIT_FAILURE);
            }
            ssize_t sent_key = send(new_fd, elem->key, elem->key_len, 0);
            if(sent_key != elem->key_len){
                perror("An error occurred while sending.");
                exit(EXIT_FAILURE);
            }
            ssize_t sent_value = send(new_fd, elem->value, elem->value_len, 0);
            if(sent_value != elem->value_len){
                perror("An error occurred while sending.");
                exit(EXIT_FAILURE);
            }

        }

        close(new_fd);
    }
    close(sockfd);
}

/*
 * TODO: WENN DER CODE ENDLICH MAL LAEUFT: die haessliche ewig lange main etwas schoener und sauberer machen, Funktionen auslagern, header Dateien und so
 */