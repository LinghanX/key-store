#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include "request.h"
char *router_ip;
int router_port;

char* send_request(struct request* request, char* k, char* v, char* addr, int port) {
    int sock;
    struct sockaddr_in server;
    int err;
    
    // create socket
    sock = socket(AF_INET , SOCK_STREAM , 0);
    if (sock == -1) {
        printf("Error creating socket.\n");
    }
    server.sin_addr.s_addr = inet_addr(addr);
    server.sin_family = AF_INET;
    server.sin_port = htons(port);

    //connect to server
    if (connect(sock , (struct sockaddr *)&server , sizeof(server)) < 0) {
        perror("Error connecting to server");
        return "ERROR";
    }
    
    //printf("Connected\n");
        
    //Send data
    size_t ksize = (k != NULL)? sizeof(char) * strlen(k) : 0;
    size_t vsize = (v != NULL)? sizeof(char) * strlen(v) : 0;
    size_t msize = sizeof(struct request) + ksize + vsize;
    struct request *r = malloc(msize);

    memcpy((char*)r, request, sizeof(struct request));
    memcpy((char*)r + sizeof(struct request), k, ksize);
    memcpy((char*)r + sizeof(struct request) + ksize, v, vsize);

    err = send(sock , r , msize , 0);
    if(err < 0) {
        printf("Error sending request to server\n");
    }
     
    //Receive a reply from the server
    struct request *reply = malloc(RSIZE);
    err = recv(sock , reply , RSIZE , 0);
    if (err < 0) {
        printf("Error receiving message.\n");
    }
    char* reply_value;
    if(reply_value != 0) {
        reply_value = malloc(reply->vsize);
        err = recv(sock, reply_value, reply->vsize, 0);
    }
    switch(reply->flag) {
        case 0: {
            // char* reply_value = malloc(reply->vsize);
            // err = recv(sock, reply_value, reply->vsize, 0);
            printf("FIND key = %s, server address = %s\n", k, reply_value);
            break;
        }
        case 3:
            printf("SET key = %s, value = %s\n", k, v);
            reply_value = "SUCCESS";
            break;
        case 4: {
            // char* reply_value = malloc(reply->vsize);
            // err = recv(sock, reply_value, reply->vsize, 0);
            printf("GET key = %s, value = %s\n", k, reply_value);
            break;
        }
        case 5: 
             printf("GET key = %s, value = NULL\n", k);
             reply_value = "NULL";
             break;
        case 6: 
            printf("operation failed.\n");
            reply_value = "FAIL";
            break;
        default: 
            printf("Error receiving message.\n");
            reply_value = "ERROR";
    }
    close(sock);
    return reply_value;
}
void* make_request(int flag, char* k, char* v) {
    struct request *r = malloc(sizeof(struct request));
    r-> flag = flag;
    r-> ksize = (k == NULL)? 0 : sizeof(char) * strlen(k);
    r-> vsize = (v == NULL)? 0 :  sizeof(char) * strlen(v);
    return r;
}
void get(char* key) {
    char* saddr = send_request(make_request(FIND, key, NULL), key, NULL, router_ip, router_port);
    //printf("saddr = %s\n", saddr);
    char *ip_addr = strtok(saddr, ":");
    char* p = strtok(NULL, " ");
    int port;
    sscanf(p, "%d", &port);
    send_request(make_request(GET, key, NULL), key, NULL,  ip_addr, port);
}
void set(char* key, char* value) {
    char* saddr = send_request(make_request(FIND, key, NULL), key, NULL, router_ip, router_port);
    //printf("saddr = %s\n", saddr);
    char *ip_addr = strtok(saddr, ":");
    char* p = strtok(NULL, " ");
    int port;
    sscanf(p, "%d", &port);
    send_request(make_request(SET, key, value), key, value, ip_addr, port);
}
char *key[]={"Copenhagen", "Columbus", "Cochabamba", "Christchurch", "Chicago", 
            "Chelyabinsk", "Casablanca", "Cairo", "Tokyo", "New York", "Sao Paulo", "Seoul",
            "Mexico City", "Osaka", "Manila", "Delhi", "Lagos", "Los Angeles", "Johannesburg"};
char *value[]={"Denmark", "Ohio", "Bolivia", "New Zealand", "Illinois",
             "Russia", "Morocco", "Egypt", "Japan", "USA", "Sao Paulo", "Korea",
             "Mexico", "Japan", "Philipine", "India", "Nigeria", "USA", "South Africa"};

int main(int argc, char *argv[]) {
    int port;
    router_ip = argv[1];
    sscanf(argv[2], "%d", &router_port);
    //printf("router is on add and port:%s, %d\n", router_ip, router_port);

    //struct request *r = make_request(2, key[0], value[0]);
    int i;
    for(i = 0; i < sizeof(key)/sizeof(*key); i++) {
        set(key[i], value[i]);
    }
    for(i = 0; i < sizeof(key)/sizeof(*key); i++) {
        get(key[i]);
    }
    get("Paris");
    return 0;
}

