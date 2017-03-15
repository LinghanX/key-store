#include "helper.h"
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <stdlib.h>
#include <stdio.h>

int open_clientfd(char *hostname, char *port){
    int clientfd;
    struct addrinfo hints, *listp, *p;

    memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_socktype = SOCK_STREAM; //following TCP/IP
    hints.ai_flags = AI_NUMERICSERV; //using a numeric port argument
    hints.ai_flags |= AI_ADDRCONFIG; 

    if((getaddrinfo(hostname, port, &hints, &listp))!= 0){
	printf("failure of getaddrinfo\n");
	exit(1);
    };

    for(p = listp; p; p = p->ai_next){
	if((clientfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) < 0)
	    continue;

	if(connect(clientfd, p->ai_addr, p->ai_addrlen) != -1)
	    break;

	close(clientfd);
    }

    freeaddrinfo(listp);

    if(!p)
	return -1;
    else 
	return clientfd;
}

int open_listenfd(char *port, int listenq){
    struct addrinfo hints, *listp, *p;
    int listenfd, optval = 1;

    memset(&hints, 0, sizeof(struct addrinfo));

    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE | AI_ADDRCONFIG;
    hints.ai_flags |= AI_NUMERICSERV;

    if((getaddrinfo(NULL, port, &hints, &listp))!= 0){
	printf("failure of getaddrinfo\n");
	exit(1);
    };

    for( p = listp; p; p = p->ai_next ){
	if((listenfd = socket( p->ai_family, p->ai_socktype, p->ai_protocol )) < 0)
	    continue;

	if((setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, (const void *)&optval, sizeof(int))) <0){
	    printf("failure to set sockopt\n");
	};

	if(bind(listenfd, p->ai_addr, p->ai_addrlen) == 0)
	    break;
	close(listenfd);
    }

    freeaddrinfo(listp);

    if(!p)
	return -1;

    if(listen(listenfd, listenq) < 0){
	close(listenfd);
	return -1;
    }

    return listenfd;
}
