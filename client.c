/*
 * client sends put(k, v) to server
 */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <netdb.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#define PORT "3344"
#define MAXDATASIZE 4096

struct info_package{
    int method;
    size_t key_size, value_size;
};

void * get_in_addr(struct sockaddr *sa){
    if(sa->sa_family == AF_INET){
	return &(((struct sockaddr_in*) sa) -> sin_addr);
    }

    return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

// argv[1] should be server address
// argv[2] should be method
// argv[3] should be key
// argv[4] should be value, if provided

int main(int argc, char *argv[]){
    int sockfd, numbytes;
    char buf[MAXDATASIZE];
    struct addrinfo hints, *servinfo, *p;
    int rv;
    char s[INET6_ADDRSTRLEN];

    struct info_package user_info;
    int method;
    size_t key_size, value_size;

    if(argc != 5){
	perror("wrong number of argument");
	exit(1);
    }

    if(strcmp(argv[2], "get") == 0){
	method = 1;
    } else if(strcmp(argv[2], "put") == 0){
	method = 2;
    }

    key_size = strlen(argv[3]);
    value_size = strlen(argv[4]);

    char *serv_addr, *serv_service;

    serv_addr = strtok(argv[1], ":");
    serv_service = strtok(NULL, "");

    char key_buffer[key_size], value_buffer[value_size];
    strcpy(key_buffer, argv[3]);
    strcpy(value_buffer, argv[4]);

    user_info.method = method;
    user_info.value_size = value_size;
    user_info.key_size = key_size;

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;

    if((rv = getaddrinfo(argv[1], PORT, &hints, &servinfo)) != 0){
	fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
	return 1;
    }

    for(p = servinfo; p != NULL; p = p-> ai_next){
	if((sockfd = socket(p->ai_family, 
			    p->ai_socktype,
			    p->ai_protocol)) == -1){
	    perror("client: socket");
	    continue;
	}

	if(connect(sockfd, p->ai_addr, p->ai_addrlen) == -1){
	    close(sockfd);
	    perror("client: connect");

	    continue;
	}

	break;
    }

    if(p == NULL){
	fprintf(stderr, "client: failed to connect\n");
	return 2;
    }

    inet_ntop(p->ai_family, get_in_addr((struct sockaddr *)p->ai_addr), s, sizeof s);
    printf("client: connecting to %s\n", s);

    if(send(sockfd, &user_info, sizeof(struct info_package), 0) < 0){
	perror("sending user info");
	exit(1);
    }

    if(send(sockfd, key_buffer, key_size, 0) < 0){
	perror("sending key info");
	exit(1);
    }

    if(send(sockfd, value_buffer, value_size, 0) < 0){
	perror("sending value info");
	exit(1);
    }

    freeaddrinfo(servinfo);

    if((numbytes = recv(sockfd, buf, MAXDATASIZE-1, 0)) == -1){
	perror("recv");
	exit(1);
    }

    buf[numbytes] = '\0';
    printf("client: received '%s' \n", buf);

    close(sockfd);
    return 0;
}
