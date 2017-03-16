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

#define PORTAL ("3344")
#define CONNECTION_POOL (10)
#define GET (1)
#define PUT (2)

struct info_package{
    int method; // 1. get; 2. put
    size_t key_size, value_size;
};

struct node_info {
    char *addr, *service;
    int entries;
};

unsigned long hash(unsigned char *s){
    unsigned long hash = 5381;
    int c;

    while( c = *s++ ){
	hash = ((hash <<5) + hash) + c;
    }
    return hash;
}

// server should be given:  number of nodes, address of data nodes
int main(int argc, char *argv[])
{
    //process nodes info
    if(argc < 3){
	perror("wrong argument number");
	exit(1);
    }
    int num_of_nodes = atoi(argv[1]);
    struct node_info available_nodes[num_of_nodes];

    // initialize nodes info
    for(int i = 0; i < num_of_nodes; i++){
	available_nodes[i].addr = strtok(argv[i + 2], ":");
	available_nodes[i].service = strtok(NULL, " ");
	available_nodes[i].entries = 0;
    }

    printf("available nodes: %s\n%s\n%d", available_nodes[0].addr, available_nodes[0].service, available_nodes[0].entries);

    int sockfd, new_fd;
    struct addrinfo hints, *servinfo, *p;
    struct sockaddr_storage their_addr;
    socklen_t sin_size;
    struct sigaction sa;
    int yes = 1;
    char s[INET6_ADDRSTRLEN];
    int rv;

    sockfd = open_listenfd(PORTAL, CONNECTION_POOL);
    printf("server: waiting for connections...\n");

    struct info_package incoming_package;

    while(1){
	char recvbuf[4096];
	sin_size = sizeof(their_addr);
	new_fd = accept(sockfd, (struct sockaddr *)&their_addr, &sin_size);
	if(new_fd == -1){
	    perror("accept");
	    continue;
	}

	// everytime the server received a request, create a fork to process the request
	if(!fork()){
	    close(sockfd);
	    //receive struct msg
	    if(recv(new_fd, &incoming_package, sizeof(struct info_package), 0) < 0){
		perror("recv error");
		exit(1);
	    }
	    printf("successfully received method: %d\nkey_size: %d\nvalue_size: %d\n", incoming_package.method,
		    (int)incoming_package.key_size, (int)incoming_package.value_size);

	    char key_buffer[incoming_package.key_size], value_buffer[incoming_package.value_size];
	    char get_buffer[4096];

	    //receive key 
	    if(recv(new_fd, key_buffer, incoming_package.key_size, 0) < 0){
		perror("recv error");
		exit(1);
	    }

	    if(incoming_package.method == GET){
		size_t entry_value =(size_t) hash(key_buffer);
		int target_node = entry_value % num_of_nodes;

		//establish node connection
		int node_fd = open_clientfd(available_nodes[target_node].addr,
			                    available_nodes[target_node].service); 

		send(node_fd, &incoming_package, sizeof(struct info_package), 0);
		send(node_fd, key_buffer, incoming_package.key_size, 0);

		recv(node_fd, get_buffer, 4096, 0);
		close(node_fd);

		send(new_fd, get_buffer, 4096, 0);
		exit(0);
	    } else if (incoming_package.method == PUT){
		if(recv(new_fd, value_buffer, incoming_package.value_size, 0) < 0){
		    perror("recv error");
		    exit(1);
		}

		size_t entry_value =(size_t) hash(key_buffer);
		int target_node = entry_value % num_of_nodes;

		//establish node connection
		int node_fd = open_clientfd(available_nodes[target_node].addr,
			                    available_nodes[target_node].service);
		send(node_fd, &incoming_package, sizeof(struct info_package), 0);
		send(node_fd, key_buffer, incoming_package.key_size, 0);
		send(node_fd, value_buffer, incoming_package.value_size, 0);

		recv(node_fd, get_buffer, 4096, 0);
		close(node_fd);
		send(new_fd, get_buffer, 4096, 0);
		exit(0);
	    } else {
		perror("unrecognised method\n");
		exit(1);
	    }
	    close(new_fd);
	}
	close(new_fd);
    }
    return 0;
}
