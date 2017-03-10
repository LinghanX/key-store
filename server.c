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

#define PORTAL "3344"
#define CONNECTION_POOL 10

struct info_package{
    int method; // 1. get; 2. put
    size_t key_size, value_size;
};

struct node_info {
    char *addr, *service;
    int entries;
};

//Division method hasing from cs.yale.edu
int hash(const char *s, int m){
    int h;
    unsigned const char *us;

    us = (unsigned const char *) s;
    h = 0;
    while(*us != '\0'){
	h = (h * 256 + *us) % m;
	us++;
    }

    return h;
}
// end hash

void sigchld_handler(int s)
{
    int saved_errno = errno;

    while(waitpid(-1, NULL, WNOHANG) > 0);

    errno = saved_errno;
}

void *get_in_addr(struct sockaddr *sa)
{
    if(sa->sa_family == AF_INET) {
	return &(((struct sockaddr_in*)sa) -> sin_addr);
    }

    return &(((struct sockaddr_in6*)sa)-> sin6_addr);
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

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;

    if((rv = getaddrinfo(NULL, PORTAL, &hints, &servinfo)) != 0){
	fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
	return 1;
    }

    for(p = servinfo; p != NULL; p = p->ai_next){
	if((sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1){
	    perror("server: socket");
	    continue;
	}

	if(setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1){
	    perror("setsockopt");
	    exit(1);
	}

	if(bind(sockfd, p->ai_addr, p->ai_addrlen) == -1){
	    close(sockfd);
	    perror("server: bind");
	    continue;
	}
	break;
    }

    freeaddrinfo(servinfo);

    if(p == NULL){
	fprintf(stderr, "server: failed to bind\n");
	exit(1);
    }

    if(listen(sockfd, CONNECTION_POOL) == -1){
	perror("listen");
	exit(1);
    }

    sa.sa_handler = sigchld_handler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESTART;

    if(sigaction(SIGCHLD, &sa, NULL) == -1){
	perror("sigaction");
	exit(1);
    }

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

	//receive struct msg
	if(recv(new_fd, &incoming_package, sizeof(struct info_package), 0) < 0){
	    perror("recv error");
	    exit(1);
	}
	printf("successfully received method: %d\nkey_size: %d\nvalue_size: %d\n", incoming_package.method,
		incoming_package.key_size, incoming_package.value_size);

	char key_buffer[incoming_package.key_size], value_buffer[incoming_package.value_size];
	char get_buffer[4096];

	//receive key 
	if(recv(new_fd, key_buffer, incoming_package.key_size, 0) < 0){
	    perror("recv error");
	    exit(1);
	}


	if(incoming_package.method == 1){
	    printf("checkpoint0\n");
	    // find correct node
	    /*
	    int entry_value = hash(key_buffer, num_of_nodes);
	    int target_node = entry_value % num_of_nodes;
	    */
	    int target_node = 0;
	    printf("checkpoint1\n");

	    //establish node connection
	    int node_fd;
	    struct addrinfo node_hints, *nodeinfo, *node;
	    memset(&node_hints, 0, sizeof(node_hints));
	    node_hints.ai_family = AF_UNSPEC;
	    hints.ai_socktype = SOCK_STREAM;
	    printf("checkpoint2\n");


	    printf("%d\n", available_nodes[target_node].entries);
	    printf("%s\n", available_nodes[target_node].addr);

	    if((getaddrinfo(available_nodes[target_node].addr, available_nodes[target_node].service, &hints, &nodeinfo))!= 0){
		perror("node binding error");
		exit(1);
	    }

	    printf("checkpoint3\n");
	    for(node = nodeinfo; node != NULL; node = node-> ai_next){
		if((node_fd = socket(node->ai_family, 
				node->ai_socktype,
				node->ai_protocol)) == -1){
		    perror("client: socket");
		    continue;
		}

		if(connect(node_fd, node->ai_addr, node->ai_addrlen) == -1){
		    close(node_fd);
		    perror("client: connect");

		    continue;
		}
		break;
	    }
	    send(node_fd, &incoming_package, sizeof(struct info_package), 0);
	    send(node_fd, key_buffer, incoming_package.key_size, 0);

	    recv(node_fd, get_buffer, 4096, 0);
	    close(node_fd);
	    send(new_fd, get_buffer, 4096, 0);
	    continue;
	} else if (incoming_package.method == 2){
	    if(recv(new_fd, value_buffer, incoming_package.key_size, 0) < 0){
		perror("recv error");
		exit(1);
	    }
	    printf("checkpoint0\n");
	    // find correct node
	    /*
	    int entry_value = hash(key_buffer, num_of_nodes);
	    int target_node = entry_value % num_of_nodes;
	    */
	    int target_node = 0;
	    printf("checkpoint1\n");

	    //establish node connection
	    int node_fd;
	    struct addrinfo node_hints, *nodeinfo, *node;
	    memset(&node_hints, 0, sizeof(node_hints));
	    node_hints.ai_family = AF_UNSPEC;
	    hints.ai_socktype = SOCK_STREAM;
	    printf("checkpoint2\n");


	    printf("%d\n", available_nodes[target_node].entries);
	    printf("%s\n", available_nodes[target_node].addr);

	    if((getaddrinfo(available_nodes[target_node].addr, available_nodes[target_node].service, &hints, &nodeinfo))!= 0){
		perror("node binding error");
		exit(1);
	    }

	    printf("checkpoint3\n");
	    for(node = nodeinfo; node != NULL; node = node-> ai_next){
		if((node_fd = socket(node->ai_family, 
				node->ai_socktype,
				node->ai_protocol)) == -1){
		    perror("client: socket");
		    continue;
		}

		if(connect(node_fd, node->ai_addr, node->ai_addrlen) == -1){
		    close(node_fd);
		    perror("client: connect");

		    continue;
		}
		break;
	    }
	    send(node_fd, &incoming_package, sizeof(struct info_package), 0);
	    send(node_fd, key_buffer, incoming_package.key_size, 0);
	    send(node_fd, value_buffer, incoming_package.value_size, 0);

	    recv(node_fd, get_buffer, 4096, 0);
	    close(node_fd);
	    send(new_fd, get_buffer, 4096, 0);
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

