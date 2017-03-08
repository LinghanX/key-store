#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <netdb.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>

#define PORT "3344"
#define MAXDATASIZE 100

void * get_in_addr(struct sockaddr *sa){
    if(sa->sa_family == AF_INET){
	return &(((struct sockaddr_in*) sa) -> sin_addr);
    }

    return &(((struct sockaddr_in6*)sa)->sin6_addr);
}
