#include "helper.h"
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <stdlib.h>
#include <stdio.h>
char* to_name(int method) {
    switch(method) {
        case 1:
            return "GET";
        case 2:
            return "PUT";
        case 3:
            return "ADD";
        case 4:
            return "DROP";
        default:
            perror("undefined method");
    }
    return "";
}
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

unsigned long hash(char *s){
    unsigned long hash = 5381;
    int c;

    while( (c = *s++) ){
        hash = ((hash <<5) + hash) + c;
    }
    return hash;

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

unsigned long key_value(struct node_info node){
    char node_content[256];
    strcpy(node_content, node.addr);
    strcat(node_content, node.service);
    return hash(node_content);
}

void sort(struct node_info *nodes, int num_of_nodes){
    for(int i = 1; i < num_of_nodes - 1; i++){
        struct node_info key = nodes[i];
        int j = i - 1;
        while(j >=0 && key_value(nodes[j]) > key_value(key)){
            nodes[j + 1] = nodes[j];
            j--;
        }

        nodes[j+1] = key;
    }
}

struct node_info find_node(struct node_info* available_nodes,
                           int size,
                           unsigned long key_hashed_value){
    struct node_info candidate;
    candidate = available_nodes[0];
    for(int i = 0; i < size; i++){
        if(key_hashed_value > key_value(candidate)){
            return candidate;
        }

        candidate = available_nodes[i];
    }
    return candidate;
}

/*
void test(int argc, char* argv[]){
    struct node_info nodes[3];
    struct node_info node_1;
    node_1.service = "localhost:";
    node_1.addr = "9999";
    nodes[0] = node_1;

    struct node_info node_2;
    node_2.service = "localhost:";
    node_2.addr = "6666";
    nodes[1] = node_2;

    struct node_info node_3;
    node_3.service = "localhost:";
    node_3.addr = "3335";
    nodes[2] = node_3;

    sort(nodes, 3);

    printf("The sorted nodes are: %s, %s, %s", nodes[0].addr,
           nodes[1].addr, nodes[2].addr);
}
 */
