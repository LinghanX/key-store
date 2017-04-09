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

static pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;

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
	printf("node: waiting for connections...\n");

	struct info_package incoming_package;

	char *serv_addr, *serv_service;
    serv_addr = strtok(argv[2], ":");
	serv_service = strtok(NULL, " ");

	while(1){
		char recvbuf[4096];
		sin_size = sizeof(their_addr);
		new_fd = accept(sockfd, (struct sockaddr *)&their_addr, &sin_size);
		if(new_fd == -1){
			perror("accept error");
			exit(0);
		}
		//receive struct msg
		if(recv(new_fd, &incoming_package, sizeof(struct info_package), 0) < 0){
			perror("recv error");
			continue;
		}
		printf("successfully received method: %d\nkey_size: %d\nvalue_size: %d\n", (int)incoming_package.method,
			   (int)incoming_package.key_size, (int)incoming_package.value_size);

		char key_buffer[incoming_package.key_size], value_buffer[incoming_package.value_size];
		if(incoming_package.method == GET){
			if(recv(new_fd, key_buffer, incoming_package.key_size, 0) < 0){
				perror("recv error");
				exit(1);
			}
			const char *ret = DictSearch(d, key_buffer);
			if(ret == NULL){
				send(new_fd, "no result", 10, 0);
				continue;
			} else {
				printf("%s \n", ret);
				printf("length of str is %d\n", (int)strlen(ret));
				if(send(new_fd, ret, strlen(ret) + 1, 0) < 0){
					perror("return value error");
					exit(1);
				}
				continue;
			}

		} else if (incoming_package.method == PUT){
			if(recv(new_fd, key_buffer, incoming_package.key_size, 0) < 0){
				perror("recv error");
				exit(1);
			}
			if(recv(new_fd, value_buffer, incoming_package.value_size, 0) < 0){
				perror("recv error");
				exit(1);
			}

			pthread_mutex_lock(&lock);
			DictInsert(d, key_buffer, value_buffer);
			pthread_mutex_unlock(&lock);

			send(new_fd, "successful!", 10, 0);
			continue;
		} else if(incoming_package.method == DROP) {
			pthread_mutex_lock(&lock);
			while(DictSize(d) > 0) {
				char* key = DictNextKey(d);
				char* ret = DictSearch(d, key);


				printf("key to be removed is: %s\n", key);

				// send request to put the key-value pair
				struct info_package request;
				request.method = PUT;
				request.key_size = strlen(key);
				request.value_size = strlen(ret);
				char key_buffer[request.key_size], value_buffer[request.value_size];
				strcpy(key_buffer, key);
				strcpy(value_buffer, ret);

				sockfd = open_clientfd(serv_addr, serv_service);

                printf("sending method: %d", request.method);
				if(send(sockfd, &request, sizeof(struct info_package), 0) < 0){
					perror("sending request info");
					exit(1);
				}

				if(send(sockfd, key_buffer, request.key_size, 0) < 0){
					perror("sending key info");
					exit(1);
				}

				if(send(sockfd, value_buffer, request.value_size, 0) < 0){
					perror("sending value info");
					exit(1);
				}
				printf("succesfully send put request\n");
				DictDelete(d, key);
//				if((numbytes = recv(sockfd, buf, MAXDATASIZE-1, 0)) == -1){
//					perror("recv");
//					exit(1);
//				}
//
//				buf[numbytes] = '\0';
//				printf("client: received '%s' \n", buf);

				close(sockfd);

			}
			pthread_mutex_unlock(&lock);
			printf("successfully unload data\n");
			continue;
		} else {
			perror("unrecognised method\n");
			exit(1);
			continue;
		}

		close(new_fd);
	}

	return 0;
}

