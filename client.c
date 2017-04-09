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
#include "helper.h"

#define PORT ("3344")
#define MAXDATASIZE (4096)

void * get_in_addr(struct sockaddr *sa){
    if(sa->sa_family == AF_INET){
        return &(((struct sockaddr_in*) sa) -> sin_addr);
    }

    return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

/*
 * method 1: -> GET "./client localhost:3344 get hello"
 * method 2: -> PUT "./client localhost:3344 put hello world"
 * method 3: -> ADD "./client localhost:3344 add localhost:3344"
 * method 4: -> DROP "./client localhost:3344 drop localhost:3344"
 *
 */

int main(int argc, char *argv[]){
    int sockfd, numbytes;
    char buf[MAXDATASIZE];

    struct info_package user_info;
    int method;
    size_t key_size, value_size;

    //process user request info
    if(strcmp(argv[2], "get") == 0){
        method = 1;
    }
    if(strcmp(argv[2], "put") == 0){
        method = 2;
    }
    if(strcmp(argv[2], "add") == 0){
        method = 3;
    }
    if(strcmp(argv[2], "drop") == 0){
        method = 4;
    }

    key_size = strlen(argv[3]);
    value_size = argv[4] ? strlen(argv[4]) : 5; // if no value is given, set default "null"
    char key_buffer[key_size], value_buffer[value_size];
    strcpy(key_buffer, argv[3]);
    strcpy(value_buffer, argv[4]? argv[4] : "null");

    user_info.method = method;
    user_info.value_size = value_size;
    user_info.key_size = key_size;

    char *serv_addr, *serv_service;
    serv_addr = strtok(argv[1], ":");
    serv_service = strtok(NULL, "");

    sockfd = open_clientfd(serv_addr, serv_service);

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

    if((numbytes = recv(sockfd, buf, MAXDATASIZE-1, 0)) == -1){
        perror("recv");
        exit(1);
    }

    buf[numbytes] = '\0';
    printf("client: received '%s' \n", buf);

    close(sockfd);
    return 0;
}
