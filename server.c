
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

#define PORT "2626"
#define BACKLOG 1 // how many pending connections queue will hold
typedef struct qotd{
    char** quotes;
    size_t len;
}qotd;


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
qotd* readfile(char* filename){

    qotd* qotd1 = malloc(sizeof(qotd));
    if(qotd1 == NULL){
        perror("quote struct: failed to alloc");
        exit(1);
    }

    qotd1->quotes = malloc(sizeof(char*)*32);
    if(qotd1->quotes == NULL){
        perror("quotes:failed to alloc");
        exit(1);
    }
    qotd1->len = 0;

    FILE* fp;
    char* line = NULL;
    size_t n = 0;
    int len;

    fp = fopen(filename,"r");
    if(fp == NULL){
        perror("fopen");
        exit(1);
    }

    int i = 0; //index
    while((len = getline(&line,&n,fp)) != -1){
        //sscanf(line,"[^\n]");
        char* tmp;
        qotd1->len++;

        int linelength = sscanf(line,"[^\n]");

        //line without '\n'
        if(linelength == len){
            break;
        }
        //save new line in array
        else{
            //allocate memory for one line
            qotd1->quotes[i] = malloc(sizeof(char)*len);
            if(qotd1->quotes[i] == NULL){
                perror("quote: failed to alloc");
                exit(1);
            }
            snprintf(qotd1->quotes[i++],len,"%s",line);
        }
    }

    fclose(fp);
    if(line){
        free(line);
    }

    return qotd1;
}

void freequotes(qotd* quotes){
    for(int i = quotes->len; i > 0; i--){
        free(quotes->quotes[i]);
    }
    free(quotes);
}

int main(int argc, char *argv[]) {

    //test server call
    if (argc != 3) {
        fprintf(stderr, "usage: server port filename\n");
        exit(1);
    }

    //read inputs arguments
    const char* port = argv[1];
    char* filename = malloc(sizeof(char)*(strlen(argv[2])+5));
    if(filename == NULL){
        perror("filename: failed to alloc");
        exit(0);
    }
    strcpy(filename,"..//");
    filename = strcat(filename,argv[2]);

    qotd * qotd1 = readfile(filename);

    free(filename);



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

    if ((rv = getaddrinfo(NULL, PORT, &hints, &servinfo)) != 0) {
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
            int r = rand() % qotd1->len;
            if (send(new_fd, qotd1->quotes[r], 13, 0) == -1)
                perror("send");
            close(new_fd);
            exit(0);
        }
        close(new_fd); // parent doesn't need this
    }

    freequotes(qotd1);
    return 0;
}
