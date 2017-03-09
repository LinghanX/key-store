#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <unistd.h>
#include <search.h>
#include <pthread.h>
#include <arpa/inet.h>
#include "request.h"

void* make_request(int flag, char* k, char* v) {
    struct request *r = malloc(sizeof(struct request));
    r-> flag = flag;
    r-> ksize = (k == NULL)? 0 : sizeof(char) * strlen(k);
    r-> vsize = (v == NULL)? 0 : sizeof(char) * strlen(v);
    return r;
}
void get_handler(int conn, char* k) {
	ENTRY e, *ep;
    e.key = k;
    ep = hsearch(e, FIND);
    if(ep != NULL) {
    	//printf("GET: result %s\n", ep->data);
    	size_t vsize = sizeof(char) * strlen(ep->data);
    	size_t msize = sizeof(struct request) + vsize;
    	struct request *r = malloc(msize);
    	r->flag = 4;
    	r->ksize = 0;
    	r->vsize = vsize;
    	memcpy((char*)r, r, sizeof(struct request));
    	memcpy((char*)r+RSIZE, ep->data, vsize);
    	send(conn, r, msize, 0);
    } else {
    	send(conn, make_request(5, NULL, NULL), RSIZE, 0);
    }
}
void set_handler(int conn, char* k, char* v) {
	ENTRY e, *ep;
	e.key = k;
	e.data = v;
	ep = hsearch(e, ENTER);
	if(ep == NULL) {fprintf(stderr, "entry failed\n"); exit(1);}
	//printf("set handler successful.\n");
	// char * client_message = "this is the reply.";
 //    write(conn , client_message , strlen(client_message));
	send(conn ,make_request(3, NULL, NULL) , RSIZE , 0);
}

void handler(int conn) {
	//printf("handling request from %d\n", conn);
	FILE *fptr = fdopen(conn, "rw");
    struct request *r = malloc(sizeof(struct request));
    fread(r, sizeof(struct request), 1, fptr); 

	char* k;
	if(r->ksize != 0) {
		k = malloc(r->ksize);
		int read = fread(k, r->ksize, 1, fptr);
		//printf("%s\n", k);
	}
	char* v;
	if(r->vsize != 0) {
		v = malloc(r->vsize);
		int read = fread(v, r->vsize, 1, fptr);
		//printf("%s\n", v);
	}
    // get or set
	switch (r->flag) {
		case 1: 
			get_handler(conn, k);
			break;
		case 2: 
			set_handler(conn, k, v);
			break;
		default: printf("Error handling request\n");
	}
    // return result

  	fclose (fptr);
  	close(conn);
   	return;
}


int main(int argc, char *argv[]) {
	//printf("server: ip, addr %s:%s\n", argv[1],argv[2]);
	int sock_fd;
	struct sockaddr_in *addr, *client_addr;
	int err;
	int conn;
	// create a hashtable
	hcreate(3);

	// create a socket
	sock_fd = socket(AF_INET, SOCK_STREAM, 0);
	if(sock_fd == -1) {
		printf("Error creating socket\n");
	}
	// bind the socket to addr
	addr = (struct sockaddr_in*)calloc(1, sizeof(struct sockaddr_in));
    addr->sin_family = AF_INET;
    int port;
	sscanf(argv[2], "%d", &port);
	//printf("server on add and port:%s, %d\n", argv[1], port);
    addr->sin_port = htons(port);
    addr->sin_addr.s_addr = inet_addr(argv[1]);

    // allow resuse socket
	int yes=1;

	if (setsockopt(sock_fd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes)) == -1) {
	    perror("setsockopt");
	    exit(1);
	}
    err = bind(sock_fd, (struct sockaddr *)addr, sizeof(struct sockaddr));
	


    free(addr);
    if (err == -1) {
        printf("server bind error: %d\n", errno);
    }

    err = listen(sock_fd, 1);
    if (err == -1) {
        printf("listen error: %d\n", errno);
    }

	while(1) {
	    client_addr = (struct sockaddr_in*)malloc(sizeof(struct sockaddr_in));
        socklen_t client_addr_size = sizeof(struct sockaddr);
        conn = accept(
                sock_fd,
                (struct sockaddr *)client_addr,
                &client_addr_size);
        free(client_addr);
        handler(conn);
	}

}