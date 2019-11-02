#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <netdb.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>

#include <arpa/inet.h>

#define MAXDATASIZE 100

typedef struct list_buf{
    char *text;
    struct list_buf *next;
}list_buf;

void *get_in_addr(struct sockaddr *sa){
    if(sa->sa_family == AF_INET){
        return &(((struct sockaddr_in*)sa)->sin_addr);
    }
    else{
        return &(((struct sockaddr_in6*)sa)->sin6_addr);
    }
}

void allocbuf(list_buf *anker, char* newtext){
    //curser
    list_buf* tmp = anker;

    //allocate new list element
    list_buf* new = malloc(sizeof(list_buf));
    new->text = malloc(sizeof(char)*MAXDATASIZE);
    snprintf(new->text,MAXDATASIZE,"%s",newtext);
    new->next = NULL;

    //loop to end of list
    while(tmp->next != NULL){
        tmp = tmp->next;
    }
    //insert new element
    tmp->next = new;
}

void freelist(list_buf *anker){
    list_buf* tmp = anker;
    list_buf* tmp2 = anker;
    while(tmp != NULL){
        tmp = tmp->next;

        free(tmp2->text);
        free(tmp2);

        tmp2 = tmp;
    }
}

int main(int argc, char *argv[]) {
    int sockfd;
    int numbytes;

    //init anker for the linked list
    list_buf *anker = malloc(sizeof(list_buf));
    if(anker == NULL){
        perror("failed to alloc buffer");
        exit(1);
    }
    anker->text = NULL;
    anker->next = NULL;

    //tmp-var for received text
    char tmp[MAXDATASIZE];

    struct addrinfo hints, *servinfo, *p;
    int rv;

    //test client call
    if(argc != 3){
        fprintf(stderr,"usage: client hostname port\n");
        exit(1);
    }

    memset(&hints , 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;

    if((rv = getaddrinfo(argv[1], argv[2], &hints, &servinfo)) != 0){
        fprintf(stderr, "getaddrinfo:%s\n",gai_strerror(rv));
        return 1;
    }

    for(p = servinfo; p != NULL; p = p->ai_next){
        if((sockfd = socket(p->ai_family, p->ai_socktype,p->ai_protocol))==-1){
            perror("client: socket");
            continue;
        }
        if(connect(sockfd, p->ai_addr, p->ai_addrlen) == -1){
            close(sockfd);
            perror("client:connect");
            continue;;
        }
        break;
    }

    if(p == NULL){
        fprintf(stderr,"client: failed to connect");
        return 2;
    }

    freeaddrinfo(servinfo);

    //receive text
    while((numbytes =recv(sockfd, tmp, MAXDATASIZE-1,0)) > 0){
        if (numbytes == -1) {
            perror("recv");
            exit(1);
        }
        //put received text in linked list
        char buf[numbytes];
        snprintf(buf,numbytes+1,"%s",tmp);
        allocbuf(anker,buf);
    }

    //print received text
    list_buf* cursor = anker->next;
    while(cursor != NULL) {
        fprintf(stdout, "%s", cursor->text);
        cursor = cursor->next;
    }

    close(sockfd);
    freelist(anker);

    return 0;

}