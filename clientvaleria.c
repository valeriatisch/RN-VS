//
// Created by Valeria on 27.10.19.
//

/*
 * client.c - TCP - Quote of the Day Protocol
 */

#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netdb.h>
#include <netinet/in.h>
#include <unistd.h>
#include <arpa/inet.h>

void *get_in_addr(struct sockaddr *sa); //Get sockaddr, IPv4 or IPv6
void print_conn_addr(struct addrinfo *p); //Print the server address we're connecting to
void get_line(int sockfd); //Receive incoming data and print it out

int main(int argc, char* argv[]){

    struct addrinfo *servinfo,
                    *p;
    int sockfd = -1;

    if(argc != 3){ //Don't forget: argv[0] holds the name of the programm
        perror("IP address or DNS and port number are required.");
        exit(EXIT_FAILURE);
    }

    char* ipAdr_or_dns = argv[1]; //Get the server ip or dns we want to connect to
    char* portNr = argv[2]; //Get its port number

    //Obtain address matching host/port:
    struct addrinfo hints = {
            .ai_family = AF_UNSPEC, //Allow IPv4 or IPv6
            .ai_socktype = SOCK_STREAM //Use TCP stream sockets
    };

    //Returns a list of suitable address structures:
    int rv = getaddrinfo(ipAdr_or_dns, portNr, &hints, &servinfo);

    if(rv != 0){
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
        exit(EXIT_FAILURE);
    }

    /*
     * Try each address until we successfully connect.
     * (So, loop through the address list pointed by servinfo until we can connect.)
     * If socket() or connect() fails, we close the socket and try the next address.
     */
    for(p = servinfo; p != NULL; p = p->ai_next){
        //Create a socket file descriptor:
        sockfd = socket(p->ai_family, p->ai_socktype, IPPROTO_TCP);
        if(sockfd == -1) continue; //If it fails
        //Establish a connection with this server:
        if(connect(sockfd, p->ai_addr, p->ai_addrlen) != -1) break; //Success
        close(sockfd); //Otherwise
    }

    if(p == NULL){ //No address succeeded (in the loop).
        fprintf(stderr, "Couldn't connect.\n");
        exit(EXIT_FAILURE);
    }

    print_conn_addr(p); //Comment out for final version

    freeaddrinfo(servinfo); //No longer needed

    /*
     * So, due to the many recv() calls, the following function is pretty slow,
     * I know, sorry for that, but I don't really care.
     */
    get_line(sockfd);

    close(sockfd); //Close our socket and terminate

    return 0;
}

void *get_in_addr(struct sockaddr *sa) {
    if(sa->sa_family == AF_INET){
        return &(((struct sockaddr_in*)sa)->sin_addr);
    }
    return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

void print_conn_addr(struct addrinfo *p){
    char s[INET6_ADDRSTRLEN];
    /*
     * inet_ntop() converts IP(v4 or v6) into a string in Internet standard format,
     * which is copied to the buffer pointed by s.
     */
    inet_ntop(p->ai_family, get_in_addr((struct sockaddr *)p->ai_addr), s, sizeof s);
    printf("Client is connecting to: %s\n", s);
}

void get_line(int sockfd){
    char buf[1]; //Where we store our received data
    int numbytes = recv(sockfd, &buf, sizeof buf, 0); //Read the first single byte

    while(numbytes > 0) {  //While there's still something to read
        if (numbytes == -1) {
            perror("Error occured while receiving");
            exit(EXIT_FAILURE);
        }
        //printf("%s", buf);
        fwrite(&buf, sizeof(char), sizeof buf, stdout);
        numbytes = recv(sockfd, &buf, sizeof buf, 0);
    }
}