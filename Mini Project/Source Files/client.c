#include <stdio.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/stat.h>
#include <time.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdbool.h>
#include "clientUtils.h"

int main(int argc, char *argv[]){
	struct sockaddr_in server;
	int sd;
	char buffer[60];
	sd = socket(AF_UNIX, SOCK_STREAM, 0);
	
	server.sin_family = AF_UNIX;
	server.sin_addr.s_addr = INADDR_ANY;
	server.sin_port = htons(9004);
	
	connect(sd, (struct sockaddr*)(&server), sizeof(server));
	
	menu(sd);
	
	close(sd);
	return 0;
}
