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

#define PORTAL "3344"
#define CONNECTION_POOL 10

struct info_package{
    int method;
    size_t key_size, value_size;
};

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

int main(void)
{
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

	inet_ntop(their_addr.ss_family, get_in_addr((struct sockaddr *) &their_addr), s, sizeof s);
	printf("server: got connection from %s\n", s);

	//receive struct msg
	if(recv(new_fd, &incoming_package, sizeof(struct info_package), 0) < 0){
	    perror("recv error");
	    exit(1);
	}
	printf("successfully received method: %d\nkey_size: %d\nvalue_size: %d\n", incoming_package.method,
		incoming_package.key_size, incoming_package.value_size);

	char key_buffer[incoming_package.key_size], value_buffer[incoming_package.value_size];
	//receive key 
	if(recv(new_fd, key_buffer, incoming_package.key_size, 0) < 0){
	    perror("recv error");
	    exit(1);
	}

	//receive value 
	if(recv(new_fd, value_buffer, incoming_package.value_size, 0) < 0){
	    perror("recv error");
	    exit(1);
	}

	strcpy(recvbuf, key_buffer);
	strcat(recvbuf, "\n");
	strcat(recvbuf, value_buffer);

	if(send(new_fd, recvbuf, 4096, 0) == -1){
	    perror("send");
	}

	close(new_fd);

	/*
	if(!fork()){
	    close(sockfd);
	    if(send(new_fd, "hello, world!", 13, 0) == -1){
		perror("send");
	    }
	    close(new_fd);
	    exit(0);
	}
	*/
    }

    return 0;
}

