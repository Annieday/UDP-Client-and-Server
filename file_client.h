#include "config.h"
#define BUFLEN 1024

static void usage()
{
	extern char * __progname;
	fprintf(stderr, "Usage: %s <host IP> <UDP port> <filename>\n",
	__progname);
	exit(1);
}

