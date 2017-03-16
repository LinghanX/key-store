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

#define CONNECTION_POOL (10)
#define GET (1)
#define PUT (2)

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

    while(1){
	char recvbuf[4096];
	sin_size = sizeof(their_addr);
	new_fd = accept(sockfd, (struct sockaddr *)&their_addr, &sin_size);
	if(new_fd == -1){
	    perror("accept");
	    continue;
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

	    DictInsert(d, key_buffer, value_buffer);
	    send(new_fd, "successful!", 10, 0);
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

