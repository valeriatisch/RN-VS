//
// Created by valeria on 01.11.19.
//

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <signal.h>
#include <time.h>

#define BACKLOG 10
#define MAXLINESIZE 512

void sigchld_handler(int s)
{
    (void)s;

    int saved_errno = errno;

    while(waitpid(-1, NULL, WNOHANG) > 0);

    errno = saved_errno;
}

void *get_in_addr(struct sockaddr *sa)
{
    if (sa->sa_family == AF_INET) {
        return &(((struct sockaddr_in*)sa)->sin_addr);
    }

    return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

FILE* check_file(char* filename){

    FILE *fp = fopen(filename, "r");
    long fsize = 0;

    if(fp == NULL){
        perror("File doesn't exist.");
        exit(EXIT_FAILURE);
    } else {
        fseek(fp, 0, SEEK_END);
        fsize = ftell(fp);
        if (fsize == 0) {
            perror("File is empty.");
            exit(EXIT_FAILURE);
        }
    }

    return fp;
}

char* get_random_line(FILE* fp) {

    char* line = malloc(MAXLINESIZE * sizeof(char));

    if(line == NULL){
        fprintf(stderr, "Insufficient memory available. Trying to allocate %li Bytes failed.", MAXLINESIZE * sizeof(char));
        exit(EXIT_FAILURE);
    }

    int number_of_lines = 0;

    while(!feof(fp)){
        fgets(line, MAXLINESIZE, fp);
        number_of_lines++;
    }

    srandom(time(NULL) + getpid());
    long int random_line = random() % number_of_lines;

    size_t n = 0;
    fseek (fp, 0, SEEK_SET);
    for(int i = 0; i < number_of_lines; i++){
        int line_length_1 = getline(&line, &n, fp);
        int line_length_2 = sscanf(line, "[^\n]");
        fgets(line, MAXLINESIZE, fp);
        if(i == random_line){
            if(line_length_1 == line_length_2){
                get_random_line(fp);
            }
            char* line_return = NULL;
            strncpy(line_return, line, line_length_2);
            return line;
        }
    }

    free(line);
    fclose(fp);
    return NULL;

}

int main(int argc, char* argv[]){

    int sockfd = 0,
        new_fd;
    struct addrinfo *servinfo,
                    *p;
    struct sockaddr_storage their_addr;
                            socklen_t sin_size;
    struct sigaction sa;
    int yes = 1;
    char s[INET6_ADDRSTRLEN];

    if(argc != 3){
        perror("Port number and filename are required.");
        exit(EXIT_FAILURE);
    }

    char* portNr = argv[1];
    char* file = argv[2];

    FILE* fp = check_file(file);

    struct addrinfo hints = {
            .ai_family = AF_UNSPEC,
            .ai_socktype = SOCK_STREAM
    };

    int rv = getaddrinfo("localhost", portNr, &hints, &servinfo);

    if(rv != 0){
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
        exit(EXIT_FAILURE);
    }

    for(p = servinfo; p != NULL; p = p->ai_next) {
        if ((sockfd = socket(p->ai_family, p->ai_socktype,
                             p->ai_protocol)) == -1) {
            perror("server: socket");
            continue;
        }

        if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes,
                       sizeof(int)) == -1) {
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

    freeaddrinfo(servinfo);

    if (p == NULL)  {
        fprintf(stderr, "server: failed to bind\n");
        exit(1);
    }

    if (listen(sockfd, BACKLOG) == -1) {
        perror("listen");
        exit(1);
    }

    sa.sa_handler = sigchld_handler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESTART;
    if (sigaction(SIGCHLD, &sa, NULL) == -1) {
        perror("sigaction");
        exit(1);
    }

    while(1) {
        sin_size = sizeof their_addr;
        new_fd = accept(sockfd, (struct sockaddr *)&their_addr, &sin_size);
        if (new_fd == -1) {
            perror("accept");
            continue;
        }

        inet_ntop(their_addr.ss_family,
                  get_in_addr((struct sockaddr *)&their_addr),
                  s, sizeof s);
        printf("server: got connection from %s\n", s);

        if (!fork()) {
            char* random_quote = get_random_line(fp);
            close(sockfd);
            if (send(new_fd, random_quote, strlen(random_quote), 0) == -1)
                perror("send");
            close(new_fd);
            exit(0);
        }

        close(new_fd);
    }

}