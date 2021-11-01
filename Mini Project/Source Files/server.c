#include <stdio.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/stat.h>
#include <time.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include "serverUtils.h"

int main(int argc, char *argv[]){
	struct sockaddr_in server, client;
	int sd, client_addr_len, nsd;
	
	sd = socket(AF_UNIX, SOCK_STREAM, 0);
	
	server.sin_family = AF_UNIX;
	server.sin_addr.s_addr = INADDR_ANY;
	server.sin_port = htons(9004);
	
	if(bind(sd,(struct sockaddr *)(&server),sizeof(server))<0){
		perror("Error at bind\n");
		exit(1);
	}
	
	listen(sd, 5);
	client_addr_len = sizeof(client);
	
	printf("Server started!\n");
	while(1){
		nsd = accept(sd, (struct sockaddr *)&client, &client_addr_len);
		printf("Got new connection request!\n");
		if(!fork()){//child
			close(sd);
			menu(nsd);
			close(nsd);
			exit(0);
		}else{//parent
			close(nsd);
		}
	}
}
