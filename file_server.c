#include "file_server.h"

int main(int argc,char *argv[])
{
	struct	sockaddr_in si_server, si_client;
	struct	sigaction sa;
	char	*ep, *rec_time, buffer[BUFLEN];
	int	ss;
	socklen_t  slen;
   	pid_t	pid;
   	u_long	p;
	u_short port;
	char	*doc_dir, *log_dir;

  	if (argc != 4){
		usage();
		errno = 0;
	}
	
 	p = strtoul(argv[1], &ep, 10);
 	if (*argv[1] == '\0' || *ep != '\0') {
		/* port wasn't a number, or was empty */
		fprintf(stderr, "%s - not a number\n", argv[1]);
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
	doc_dir = argv[2];
	log_dir = argv[3];
	createLog(log_dir);

	/* daemonize */
	if (daemon(1, 0) == -1)
		err(1, "daemon() failed");

	bzero(&si_server, sizeof(si_server));
	
	si_server.sin_family = AF_INET;
	si_server.sin_port = htons(port);
	si_server.sin_addr.s_addr = htonl(INADDR_ANY);

	if ((ss = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1)
		err(1,"Client: cannot open socket\n");

	if (bind(ss, (struct sockaddr *)&si_server, sizeof(si_server)) == -1)
		err(1,"Error in binding the socket\n");

	/* Catching SIGCHLD to make sure we have no zombies children */
	sa.sa_handler = kidhandler;
		sigemptyset(&sa.sa_mask);

	/*
	* allow system calls(eg. accept) to be restarted 
	* if they get interrupted by a SIGCHLD
	*/
	sa.sa_flags = SA_RESTART;
	if (sigaction(SIGCHLD, &sa, NULL) == -1)
		err(1, "sigaction failed\n");

	printf("File Server listening to port: %d\n", ntohs(si_server.sin_port));
	
	while(1){
		memset(buffer, 0, sizeof(buffer));
		slen = sizeof(si_client);
		if((recvfrom(ss, buffer, sizeof(buffer), 0,
				(struct sockaddr *)&si_client, &slen)) != -1) {
			pid = fork();
			if (pid == 0){
				rec_time = getTime();
				handle_transmission(log_dir, doc_dir, ss,
						si_client, rec_time, buffer);
				close(ss);
				exit(0);				
			} else if (pid == -1) {
				err(1, "Fork failed");
			}
		}
	}
}


void createLog(char* log_dir) {
	/* create a logfile in the specific directory */
	FILE*	file = fopen(log_dir, "a");
	if (file < 0) {
		perror("CLIENT:\n");
		exit(EXIT_FAILURE);
	}
	fclose(file);
}

char* getTime(){
	/*Get time */
	time_t	rawtime;
	struct tm *info;
	char	buffer[80];
	memset(buffer, 0, 81);
 	time( &rawtime );
	info = localtime( &rawtime );
	strftime(buffer,80,"%a %d %b %Y %X %Z", info);
	return strdup(buffer);
}

void handle_transmission(char* log_dir, char* doc_dir, int ss, 
			struct sockaddr_in si_client, char *rec_time, char buffer[BUFLEN]) {

	char	doc_directory[BUFLEN];
	char	context[CHUNKLEN];
	char	*end_time;
	char	*log_str;
	int	c, index, complete = 0, ret = 0;
	
	log_str = malloc(1024 * sizeof(char));
	snprintf(doc_directory, sizeof(doc_directory),"%s/%s", doc_dir, buffer);
	sprintf(log_str, "%s %d %s %s", inet_ntoa(si_client.sin_addr),
			ntohs(si_client.sin_port), buffer, rec_time);
	
	FILE* doc_fp=fopen(doc_directory, "r");

	if(doc_fp == NULL) {
		fprintf(stderr, "%s", notFound);
		sendto(ss, notFound, strlen(notFound), 0,
				(struct sockaddr*)&si_client, sizeof(si_client));
		strcat(log_str, " file not found\n");
	} else {
		if(access( doc_dir, R_OK) != -1){
		
			while(!feof(doc_fp)){

				bzero(&context, sizeof(context));
				for(index=0; index < 1024; index++){
					c=fgetc(doc_fp);
					if(c!=EOF){
						strcat(context,(const char*)&c);
					}
					if(feof(doc_fp)){
						if(index == 0)
							strcat(context, "$");
						break;
					}
				}
				ret = sendto(ss, context, sizeof(context), 0,
							(struct sockaddr*)&si_client,
							sizeof(si_client));
								
				if(ret < 0){
					fprintf(stderr, "%s", notComplete);
					break;
				}
			}
			if(ret >= 0){
				fprintf(stdout, "Transmission completed\n");
				end_time = getTime();
				complete = 1;
			}
		} else {
			fprintf(stderr, "No read permission.\n");
		}
	}
		
	FILE* log_fp = fopen(log_dir,"a");
	flock(fileno(log_fp), LOCK_EX);
	if(log_fp < 0){
		perror("CLIENT:\n");
		exit(1);
	}
	if(complete == 1)
		fprintf(log_fp,"%s %s\n", log_str, end_time);
	else
		fprintf(log_fp,"%s  transmission not completed\n", log_str);
	flock(fileno(log_fp), LOCK_UN);
	fclose(log_fp); 
	free(log_str);
}

