#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <signal.h>
#include <search.h>
#include "dict.h"
#include "helper.h"
#include <pthread.h>

#define CONNECTION_POOL (10)
#define GET (1)
#define PUT (2)
#define DROP (4)


// initialize node with the port number of this node and the address of the server
// % ./node [port number] localhost:[port number]
int main(int argc, char *argv[])
{
	Dict d;
	d = DictCreate();

	char *port = argv[1];

	int sockfd, new_fd;
	struct sockaddr_storage their_addr;
	socklen_t sin_size;

	sockfd = open_listenfd(port, CONNECTION_POOL);
	printf("NODE-%s listening\n", port);

	char *serv_addr, *serv_service;
    serv_addr = strtok(argv[2], ":");
	serv_service = strtok(NULL, " ");

	while(1){
		struct info_package incoming_package;
		incoming_package.key_size = 0;
		incoming_package.value_size = 0;
		incoming_package.method = -1;

		sin_size = sizeof(their_addr);
		new_fd = accept(sockfd, (struct sockaddr *)&their_addr, &sin_size);
		if(new_fd == -1){
			perror("accept error");
			exit(0);
		}
		//receive struct msg
		if(recv(new_fd, &incoming_package, sizeof(struct info_package), 0) < 0){
			perror("recv error");
		}
		char key_buffer[incoming_package.key_size];
		char value_buffer[incoming_package.value_size];

		if(incoming_package.key_size > 0) {
			if(recv(new_fd, key_buffer, incoming_package.key_size, 0) < 0){
				perror("recv error");
				exit(1);
			}
		}
		if(incoming_package.value_size > 0) {
			if(recv(new_fd, value_buffer, incoming_package.value_size, 0) < 0){
				perror("recv error");
				exit(1);
			}
		}
		printf("NODE-%s receive: %s %s %s\n",
			port,
			to_name(incoming_package.method),
			key_buffer,
			value_buffer);
		
		
		if(incoming_package.method == GET){
			const char *ret = DictSearch(d, key_buffer);
			printf("NODE-%s send GET result: %s\n",
				port,
				ret);

			if(ret == NULL){
				send(new_fd, "no result", 10, 0);
			} else {
				printf("%s \n", ret);
				printf("length of str is %d\n", (int)strlen(ret));
				if(send(new_fd, ret, strlen(ret) + 1, 0) < 0){
					perror("return value error");
					exit(1);
				}
			}

		} else if (incoming_package.method == PUT){

			printf("NODE-%s send PUT result: %s\n",
				port,
				"successful!");

			DictInsert(d, key_buffer, value_buffer);

			send(new_fd, "successful!", 10, 0);
		
		} else if(incoming_package.method == DROP) {
			printf("NODE-%s: try to relocate %d kv pairs\n", port, DictSize(d));

			while(DictSize(d) > 0) {
				char* key = DictNextKey(d);
				const char* ret = DictSearch(d, key);

				// send request to put the key-value pair
				struct info_package request;
				request.method = PUT;
				request.key_size = strlen(key);
				request.value_size = strlen(ret);
				char key_buffer[request.key_size], value_buffer[request.value_size];
				strcpy(key_buffer, key);
				strcpy(value_buffer, ret);

				int new_sock_fd = open_clientfd(serv_addr, serv_service);
				
				printf("NODE-%s send PUT request: %s %s\n",
					port,
					key_buffer,
					value_buffer);

				if(send(new_sock_fd, &request, sizeof(struct info_package), 0) < 0){
					perror("sending request info");
					exit(1);
				}

				if(send(new_sock_fd, key_buffer, request.key_size, 0) < 0){
					perror("sending key info");
					exit(1);
				}

				if(send(new_sock_fd, value_buffer, request.value_size, 0) < 0){
					perror("sending value info");
					exit(1);
				}
				DictDelete(d, key);
                close(new_sock_fd);
			}
			printf("NODE-%s: finish relocate.\n", port);
		} else {
			perror("unrecognised method\n");
			exit(1);
		}

		close(new_fd);
	}

	return 0;
}

