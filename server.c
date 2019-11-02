
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <signal.h>

#define BACKLOG 10 // how many pending connections queue will hold

void sigchld_handler(int s){
// waitpid() might overwrite errno, so we save and restore it:
    int saved_errno = errno;

    while(waitpid(-1, NULL, WNOHANG) > 0);
    errno = saved_errno;
}

// get sockaddr, IPv4 or IPv6:
void *get_in_addr(struct sockaddr *sa){
    if (sa->sa_family == AF_INET) {
        return &(((struct sockaddr_in*)sa)->sin_addr);
    }

    return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

//get lines from .txt file
char** readfile(char* filename){
    char** quotes = malloc(sizeof(char**));
    if(quotes == NULL){
        perror("quotes:failed to alloc");
        exit(1);
    }

    FILE* fp;
    char* line = NULL;
    size_t n = 0;
    int len;

    fp = fopen(filename,"r");
    if(fp == NULL){
        perror("fopen");
        exit(1);
    }

    quotes[0] = 0; //count up for array length
    int i = 1; //index
    while((len = getline(&line,&n,fp)) != -1){
        //sscanf(line,"[^\n]");
        char* tmp;
        quotes[0]++;
        //delete '\n' from lines
        if((tmp = strchr(line,'\n')) != NULL){
            *tmp = "";
            //allocate memory for one line
            quotes[i] = malloc(sizeof(char)*len);
            if(quotes[i] == NULL){
                perror("quote: failed to alloc");
                exit(1);
            }
            //save quote in array
            quotes[i++] = line;
        }
            //no '\n' found -> stop getting lines
        else{
            //allocate memory for one line
            quotes[i] = malloc(sizeof(char)*len);
            if(quotes[i] == NULL){
                perror("quote: failed to alloc");
                exit(1);
            }
            //save quote in array
            quotes[i++] = line;
            break;
        }
    }

    fclose(fp);
    if(line){
        free(line);
    }

    return quotes;
}

void freequotes(char** quotes){
    for(int i = quotes[0]; i > 0; i--){
        free(quotes[i]);
    }
    free(quotes);
}

int main(int argc, char *argv[]) {

    //test server call
    if (argc != 3) {
        fprintf(stderr, "usage: server port filename\n");
        exit(1);
    }

    int port = argv[1];
    char *filename = argv[2];
    char **quotes = readfile(filename);


    int sockfd, new_fd; // listen on sock_fd, new connection on new_fd
    struct addrinfo hints, *servinfo, *p;
    struct sockaddr_storage their_addr; // connector's address information
    socklen_t sin_size;
    struct sigaction sa;
    int yes = 1;
    char s[INET6_ADDRSTRLEN];
    int rv;

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE; // use my IP

    if ((rv = getaddrinfo(NULL, port, &hints, &servinfo)) != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
        return 1;
    }

    // loop through all the results and bind to the first we can
    for (p = servinfo; p != NULL; p = p->ai_next) {
        if ((sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1) {
            perror("server: socket");
            continue;
        }

        if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1) {
            perror("setsockopt");
            exit(1);
        }

        if (bind(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
            close(sockfd);
            perror("server: bind");
            continue;
        }

        break;
    }

    freeaddrinfo(servinfo); // all done with this structure

    if (p == NULL) {
        fprintf(stderr, "server: failed to bind\n");
        exit(1);
    }

    if (listen(sockfd, BACKLOG) == -1) {
        perror("listen");
        exit(1);
    }

    sa.sa_handler = sigchld_handler; // reap all dead processes
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESTART;
    if (sigaction(SIGCHLD, &sa, NULL) == -1) {
        perror("sigaction");
        exit(1);
    }

    printf("server: waiting for connections...\n");

    while (1) { // main accept() loop
        sin_size = sizeof their_addr;
        new_fd = accept(sockfd, (struct sockaddr *) &their_addr, &sin_size);
        if (new_fd == -1) {
            perror("accept");
            continue;
        }

        inet_ntop(their_addr.ss_family, get_in_addr((struct sockaddr *) &their_addr), s, sizeof s);
        printf("server: got connection from %s\n", s);

        if (!fork()) { // this is the child process
            close(sockfd); // child doesn't need the listener
            int r = rand() % ((int)quotes[0]-1);
            if (send(new_fd, quotes[r+1], 13, 0) == -1)
                perror("send");
            close(new_fd);
            exit(0);
        }
        close(new_fd); // parent doesn't need this
    }

    freequotes(quotes);
    return 0;
}