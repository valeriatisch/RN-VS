#include <sys/socket.h>
#include <stdio.h>
#include <netdb.h>
#include <string.h>
#include <unistd.h>
#include "stdlib.h"
#include <time.h>
#include "../include/protocol.h"

struct timespec sendPacket(int sockfd, struct addrinfo *p){
    //send ntp protocol
    protocol* prot = createProtocol(0, 4, 3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
    sendProtocol(sockfd, prot, p);
    
    //timestamp t1
    struct timespec sent;
    getTime(sent);
    return sent;  
}

void receivePacket(int n, int sockfd, double* delay_arr, struct timespec start, struct addrinfo *p_arg){
    protocol *prot = recvProtocol(sockfd, p_arg);

    //timestamp t4
    struct timespec received;
    getTime(received);

    print_result(); 
}

double get_max(double* array) {
    double max = array[0];
    for(int i = 1; i < 8; i++){
        if(array[i] > max) max = array[i];
    }
    return max;
}

double get_min_not_zero(double* array) {
    if(array[0] == 0.0) return array[0];
    double min_not_zero = array[0];
    for(int i = 1; i < 8; i++) {
        if(array[i] != 0.0 && array[i] < min_not_zero) min_not_zero = array[i];
    }
    return min_not_zero;
}

void getTime(struct timespec time_to_get) {
    if (clock_gettime(CLOCK_REALTIME, &time_to_get) == -1) {
		peeror("failed to get time");
		exit(1);
	}
} 

long getTimeDiff_asNano(struct timespec start, struct timespec stop){
    long start_sec_as_nano = start.tv_sec * 1E+9;
	long stop_sec_as_nano = stop.tv_sec * 1E+9;
	return ((stop_sec_as_nano + stop.tv_nsec) - (start_sec_as_nano + start.tv_nsec));
}

void timeSleep_nano(long time_nano) {
    if(time_nano <= 0) {
        perror("timeout, receive took longer then 8 seconds");
        exit(1);
    }
    else {
        struct timespec time;
        time.tv_sec = time_nano / 1E+9;
        time.tv_nsec = time_nano % (long) 1E+9;
        nanosleep(time, NULL);
    }
}

/**
 * Erstellt einen neuen buffer mit der maximalen Länge "maxLength"
 *
 * @param maxLength
 * @return buffer
 */
buffer* createBuffer(uint32_t maxLength) {
    buffer *ret = malloc(sizeof(buffer));
    INVARIANT(ret != NULL, NULL, "Malloc error: buffer")

    ret->buff = malloc(maxLength);
    ret->maxLength = maxLength;
    ret->length = 0;

    return ret;
}

/**
 * Erstellt einen neuen Buffer ohne neuen Speicher zu allokieren.
 * Bekommt speicher als Argument "existingBuffer" übergeben.
 *
 * @param length
 * @param existingBuffer
 * @return buffer
 */
buffer* createBufferFrom(uint32_t length, void* existingBuffer) {
    buffer *ret = malloc(sizeof(buffer));
    INVARIANT(ret != NULL, NULL, "Malloc error: buffer")

    ret->buff = existingBuffer;
    ret->maxLength = length;
    ret->length = length;

    return ret;
}

buffer* copyBuffer(buffer* b) {
    uint8_t *cpy = calloc(b->length, sizeof(uint8_t));
    memcpy(cpy, b->buff, b->length);
    buffer* ret = createBufferFrom(b->length, cpy);
    return ret;
}

/**
 * Gibt den Speicher von einem buffer struct frei
 *
 * @param bufferToFree
 */
void freeBuffer(buffer *bufferToFree) {
    if(bufferToFree != NULL) {
        free(bufferToFree->buff);
        free(bufferToFree);
    }
}


/**
 * Recv "length" bytes über den angegebenen socket.
 *
 * @param socket
 * @param length
 * @param p addrinfo struct für recvfrom()
 * @return buffer mit den empfangenen Daten
 */
buffer* recvBytesAsBuffer(int socket, int length, struct addrinfo *p) {
    buffer *temp = createBuffer(length);
    INVARIANT(temp != NULL, NULL, "");

    while (temp->length < temp->maxLength) {
        int recvLength = recvfrom(socket, (temp->buff + temp->length), temp->maxLength - temp->length, 0, p->ai_addr, p->ai_addrlen);
        INVARIANT_CB(recvLength != -1, NULL, "RecvError", {
            freeBuffer(temp);
        })

        temp->length += recvLength;
    }

    return temp;
}

/**
 * Hilfsfunktion die "length" bytes von value über den übergebenen socket verschickt.
 *
 * @param socket Socket über den "value" verschickt werden soll
 * @param value
 * @param length Anzahl an Bytes die von "value" verschickt werden sollen
 * @param p addrinfo struct für sendto()
 * @return -1 bei Fehler, ansonsten anzahl an verschickten bytes
 */
int sendAll(int socket, void* value, uint32_t length, struct addrinfo *p) {
    int totalSend = 0;

    // sendto server until everything is send
    while (totalSend < length) {
        int sendn = sendto(socket, value + totalSend, length - totalSend, 0, p->ai_addr, p->ai_addrlen);
        INVARIANT(sendn != -1, -1, "Failed to send all bytes")

        totalSend += sendn;
    }

    INVARIANT(totalSend == length, -1, "Failed to send all bytes")

    return totalSend;
}

struct addrinfo getHints() {
    struct addrinfo hints;
    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_INET;     //AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;
    return hints;
}

uint32_t addrinfoToServerAddress(struct addrinfo *addr) {
    struct sockaddr_in *serverIpAddr = (struct sockaddr_in*) addr->ai_addr;
    return serverIpAddr->sin_addr.s_addr;
}

/**
 * Hilfsfunktion die schaut ob das n-te bit in der bitsequence gleich 1 ist.
 *
 * @param bitsequence
 * @param n
 * @return 1 wenn bit an stelle n 1 ist, ansonsten 0
 */
int checkBit(unsigned bitsequence, int n) {
    return (bitsequence >> n) & 1;
}
