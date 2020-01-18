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
    protocol* prot = createProtocol(35, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
    sendProtocol(sockfd, prot, p);
    
    //timestamp t1
    struct timespec sent;
    getTime(sent);
    return sent;  
}

void receivePacket(int n, int sockfd, double* delay_arr, struct timespec sent, struct addrinfo *p_arg){
    protocol *prot = recvProtocol(sockfd, p_arg);

    //timestamp t4
    struct timespec received;
    getTime(received);

    //get ip from addrinfo struct
    struct sockaddr_in *tmp = (struct sockaddr_in*) p_arg; 
    uint32_t ip = inet_ntoa(tmp->sin_addr);

    //timestamp structs to seconds
    double t_2 = (prot->rec_ts_sec - OFFSET) + (prot->rec_ts_fsec / 4294967296); // 2^32 
    double t_3 = (prot->trans_ts_sec - OFFSET) + (prot->trans_ts_sec / 4294967296);

    double t_1 =  sent.tv_sec + ((double) sent.tv_nsec) / 1E+9;
    double t_4 =  received.tv_sec + ((double) received.tv_nsec) / 1E+9;

    print_result(ip, n, prot->root_dispersion, disperion8(delay_arr), delay_as_seconds(delay_arr, n, t_1, t_2, t_3, t_4), offset_as_seconds(t_1, t_2, t_3, t_4)); 
}

void print_result(uint32_t IP,int nummer, double root_dispersion, double dispersion8, double delay, double offset){
    printf("%d;%d;%ld;%ld;%ld;%ld\n", IP, nummer, root_dispersion, dispersion8, delay, offset);
}

double dispersion8(double* delay_arr) {
    double max = get_max(delay_arr);
    double min = get_min_not_zero(delay_arr);
    return (max - min);
}

double delay_as_seconds(double* delay_arr, int n, double t1, double t2, double t3, double t4) {
    double seconds = ((t4 - t1)  - (t3 - t2)) / 2;
    delay_arr[n % 8] = seconds;
    return seconds;
}

double offset_as_seconds(double t1, double t2, double t3, double t4){
    return ((t2 - t1) + (t3 - t4))/2;
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
    long start_sec_as_nano = start.tv_sec * (1E+9);
	long stop_sec_as_nano = stop.tv_sec * (1E+9);
	return ((stop_sec_as_nano + stop.tv_nsec) - (start_sec_as_nano + start.tv_nsec));
}

void timeSleep_nano(long time_nano) {
    if(time_nano <= 0) {
        perror("timeout, receive took longer then 8 seconds");
        exit(1);
    }
    else {
        struct timespec time;
        time.tv_sec = time_nano / (1E+9);
        time.tv_nsec = time_nano % (long) (1E+9);
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
