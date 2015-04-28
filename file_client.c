#include "file_client.h"

int main(int argc, char *argv[]) {
	struct sockaddr_in server;
	socklen_t len = sizeof(struct sockaddr_in);
	char *ep, buf[BUFLEN];
	struct hostent *host;
	struct timeval tv;
	int n, s;
	u_long	p;
	u_short port;

	if (argc < 4) {
		usage();
		return 1;
	}

	host = gethostbyname(argv[1]);
	if (host == NULL) {
		perror("gethostbyname");
		return 1;
	}

 	p = strtoul(argv[2], &ep, 10);
 	if (*argv[2] == '\0' || *ep != '\0') {
		/* port wasn't a number, or was empty */
		fprintf(stderr, "%s - not a number\n", argv[2]);
		usage();
	}
	if ((errno == ERANGE && p == ULONG_MAX) || (p > USHRT_MAX)) {
		/*
		 * port is a number, but it either can't fit in an unsigned
		 * long, or is too big for an unsigned short
		 */
		fprintf(stderr, "%s - Port value out of range\n", argv[1]);
		usage();
	}
	port = p;

	/* initialize socket */
	if ((s=socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1) {
		perror("socket");
		return 1;
	}

	/* initialize server addr */
	memset((char *) &server, 0, sizeof(struct sockaddr_in));
	server.sin_family = AF_INET;
	server.sin_port = htons(port);
	server.sin_addr = *((struct in_addr*) host->h_addr);

	/* send message */
	if (sendto(s, argv[3], strlen(argv[3]),
		0, (struct sockaddr *) &server, len) == -1) {
		perror("sendto()");
		return 1;
	}
	fprintf(stdout,"Received from %s:%d: \n", inet_ntoa(server.sin_addr),
			ntohs(server.sin_port)); 

    /* receive echo.
    ** for single message, "while" is not necessary. But it allows the client 
    ** to stay in listen mode and thus function as a "server" - allowing it to 
    ** receive message sent from any endpoint.
    */
		
	tv.tv_sec = 5;
	tv.tv_usec = 0;
	
	setsockopt(s, SOL_SOCKET, SO_RCVTIMEO, (char *)&tv, sizeof(struct timeval));
	while(1){
		if ((n = recvfrom(s, buf, BUFLEN, 0,
				(struct sockaddr *) &server, &len)) != -1) {

				setsockopt(s, SOL_SOCKET, SO_RCVTIMEO, (char *)&tv, 
						sizeof(struct timeval));

			if((n < 17) && 
				(strncmp(buf,"File not found.\n", 16) == 0)){
				fprintf(stderr, "Error: File not found.\n");
				break;
			} else {
				fflush(stdout);
				if((n < 2) && (strncmp(buf, "$", 1) == 0)){
					fprintf(stdout, "All chunks have been recived.\n");
					break;
				} else {
					write(1, buf, n);
				}
				if(strlen(buf) < 1024){
					fprintf(stdout, "All chunks have been recived.\n");
					break;
				}
			}
			continue;
		}
		fprintf(stderr, "Server time out, exit the program.\n");
		break;
	}
	
	close(s);
	return 0;
}

