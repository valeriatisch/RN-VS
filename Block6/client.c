#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <time.h>

#define SERVERPORT "4950"	// the port users will be connecting to

int main(int argc, char *argv[])
{
	int sockfd;
	struct addrinfo hints, *servinfo, *p;
	int rv;
	int numbytes;

	if (argc <= 3) {
		fprintf(stderr,"usage: ./ntpclient n server1 server2 ... \n");
		exit(42);
	}

	//save input arguments
	int n = argv[2];

	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_DGRAM;

	//get addressinfo from every server ->
	for(int i = 2; i < argc; i++){

		if ((rv = getaddrinfo(argv[i], SERVERPORT, &hints, &servinfo)) != 0) {
			fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
			return 1;
		}

		//loop through all the results and make a socket
		for(p = servinfo; p != NULL; p = p->ai_next) {
			if ((sockfd = socket(p->ai_family, p->ai_socktype,
					p->ai_protocol)) == -1) {
				perror("talker: socket");
				continue;
			}

			break;
		}

		if (p == NULL) {
			fprintf(stderr, "talker: failed to bind socket\n");
			return 2;
		}
		
		//send ntp-protocol and receive from server
		for(int j = 0; j < n; j++) {
			//get time t_1
			struct timespec start, stop;
			// get system time at start
			getTime(start);

			//send and wait for answer
			sendPacket(j, sockfd);//TODO
			receivePacket(sockfd);//TODO
			
			//get system time at stop
			getTime(stop);

			//sleep 8 - (t_2 - t_1) seconds
			long diff_nano = getTimeDiff_asNano(start,stop);
			timeSleep_nano(diff_nano);
			
		}

		freeaddrinfo(servinfo);
		close(sockfd);
		
	}
	return 0;
}