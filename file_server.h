#include "config.h"
#define BUFLEN 2048
#define LINELEN 512
#define CHUNKLEN 1024


static char* notFound = "File not found.\n";
static char* notComplete = "Transmission not completed.\n";

static void kidhandler(int signum) {
	/* signal handler for SIGCHLD */
	waitpid(WAIT_ANY, NULL, WNOHANG);
}

static void usage()
{
	extern char * __progname;
	fprintf(stderr, 
	"usage: %s <port number> <documents directory> <logfile directory>\n", 
	__progname);
	exit(1);
}

void createLog(char* log_dir);
char* getTime();
void handle_transmission(char* log_dir, char* doc_dir, int ss, 
			struct sockaddr_in si_client, char *rec_time,
							char buffer[BUFLEN]);

