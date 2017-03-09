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
#include "request.h"
#include <arpa/inet.h>

void* make_request(int flag, char* k, char* v) {
    struct request *r = malloc(sizeof(struct request));
    r-> flag = flag;
    r-> ksize = (k == NULL)? 0 : sizeof(char) * strlen(k);
    r-> vsize = (v == NULL)? 0 : sizeof(char) * strlen(v);
    return r;
}
size_t hash(char const *input) { 

    const int ret_size = 32;
    size_t ret = 0x0;
    const int per_char = 7;
    while (*input) { 
        ret ^= *input++;
        ret = (ret << per_char) | (ret >> (ret_size - per_char));
   }
   return ret;
}

void handler(int conn, int n, char* argv[]) {
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

	//printf("the key is %s\n", k);
	size_t hash_value = hash(k);
	//printf("hash %zu\n", hash(k));
	switch(r->flag) {
		case (0): {
			// return adress of the server
			int server_num = hash_value % n;
			//printf("router: direct to server %d\n", server_num);
			char* addr = argv[server_num + 2];
			size_t vsize = sizeof(char) * strlen(addr);
    		size_t msize = sizeof(struct request) + vsize;
    		struct request *r = malloc(msize);
    		r->flag = FIND;
    		r->ksize = 0;
    		r->vsize = vsize;
    		memcpy((char*)r, r, sizeof(struct request));
    		memcpy((char*)r+RSIZE, addr, vsize);
    		send(conn, r, msize, 0);
			break;
		}
		default: 
			printf("Error routing.");
	}
    // return result

  	fclose (fptr);
   	return;
}


int main(int argc, char *argv[]) {
	int sock_fd;
	struct sockaddr_in *addr, *client_addr;
	int err;
	int conn;

	printf("\n//////////////////////////////////////////////\n");
	printf("router address: %s\n", argv[1]);
	printf("number of servers: %d\n", argc -2);
	printf("//////////////////////////////////////////////\n");

	char *ip_addr = strtok(argv[1], ":");
    char* p = strtok(NULL, " ");
    int port;
    sscanf(p, "%d", &port);
	// create a socket
	sock_fd = socket(AF_INET, SOCK_STREAM, 0);
	if(sock_fd == -1) {
		printf("Error creating socket\n");
	}
	// bind the socket to addr
	addr = (struct sockaddr_in*)calloc(1, sizeof(struct sockaddr_in));
    addr->sin_family = AF_INET;
    addr->sin_port = htons(port);
    addr->sin_addr.s_addr = inet_addr(ip_addr);

    // allow socket reuse
    int yes=1;

	if (setsockopt(sock_fd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes)) == -1) {
	    perror("setsockopt");
	    exit(1);
	}
    err = bind(sock_fd, (struct sockaddr *)addr, sizeof(struct sockaddr));
    free(addr);
    if (err == -1) {
        printf("router bind error: %d\n", errno);
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
        handler(conn, argc-2, argv);
	}

}