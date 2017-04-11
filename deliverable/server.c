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
#include "helper.h"

#define PORTAL ("3344")
#define CONNECTION_POOL (10)
#define MAX_NODE_NO (100)
#define CLIENT_PORT (4096)

// server should be given:  number of nodes, address of data nodes
int main(int argc, char *argv[])
{
    int num_of_nodes = atoi(argv[1]);
    struct node_info available_nodes[MAX_NODE_NO];

    // initialize nodes info
    for(int i = 0; i < num_of_nodes; i++){
        strcpy(available_nodes[i].addr,strtok(argv[i + 2], ":"));
        strcpy(available_nodes[i].service,strtok(NULL, " "));
        available_nodes[i].entries = 0;
    }

    int sockfd, new_fd;
    struct sockaddr_storage their_addr;
    socklen_t sin_size;
    struct sigaction sa;

    sockfd = open_listenfd(PORTAL, CONNECTION_POOL);
    printf("server: waiting for connections...\n");

    while(1){
        char* recvbuf = malloc(4096);
        char* key_buffer = malloc(256);
        char* get_buffer = malloc(256);
        char* value_buffer = malloc(256);
        struct info_package *incoming_package = malloc(sizeof(struct info_package));

        sin_size = sizeof(their_addr);
        new_fd = accept(sockfd, (struct sockaddr *)&their_addr, &sin_size);

        if(new_fd == -1){
            perror("accept");
            continue;
        }

        if(recv(new_fd, incoming_package, sizeof(struct info_package), 0) < 0){
            perror("recv error");
            exit(1);
        }

        //receive key
        if(recv(new_fd, key_buffer, incoming_package->key_size, 0) < 0){
            perror("recv error");
            exit(1);
        }

        if(incoming_package->value_size != 0) {
            if(recv(new_fd, value_buffer, incoming_package->value_size, 0) < 0){
                perror("recv error");
                exit(1);
            }
        }   

        sort(available_nodes, num_of_nodes);

        unsigned long key_hashed_value = hash(key_buffer);
        struct node_info *target_node = find_node(available_nodes,
                                                 num_of_nodes,
                                                 key_hashed_value);

        printf("SERVER: received %s %s %s; node: %s:%s\n", 
            to_name(incoming_package->method),
            key_buffer, 
            value_buffer,
            target_node->addr,
            target_node->service);

        if(incoming_package->method == ADD){
            num_of_nodes++;

            char node_addr[256];
            strcpy(node_addr, key_buffer);
            strcpy(available_nodes[num_of_nodes-1].addr,strtok(key_buffer, ":"));
            strcpy(available_nodes[num_of_nodes-1].service, strtok(NULL, ""));

            printf("SERVER new node %s hash %lu\n", 
                node_addr, key_value(available_nodes[num_of_nodes-1]));

            sort(available_nodes, num_of_nodes);
            print_node_info(available_nodes, num_of_nodes);
            if(num_of_nodes != 1){
                struct node_info post_node = find_post_node(available_nodes, num_of_nodes, node_addr);
                reset_node(post_node);
            }
        }

        else if(incoming_package->method == DROP){
            struct node_info *node_to_be_removed = malloc(sizeof(struct node_info));
            strcpy(node_to_be_removed->addr, strtok(key_buffer, ":"));
            strcpy(node_to_be_removed->service, strtok(NULL, ""));

            remove_node(available_nodes, num_of_nodes, node_to_be_removed);
            num_of_nodes--;
            sort(available_nodes, num_of_nodes);

            printf("sorting completed\n");

            reset_node(*node_to_be_removed);
            free(node_to_be_removed);
        }

        else if(incoming_package->method == GET){
            //establish node connection
            incoming_package->value_size = 0;
            talk(target_node,
                 incoming_package, 
                 key_buffer, 
                 value_buffer, 
                 get_buffer);

            send(new_fd, get_buffer, 4096, 0);
        }

        else if (incoming_package->method == PUT){
            //establish node connection
            talk(target_node, 
                 incoming_package, 
                 key_buffer, 
                 value_buffer, 
                 get_buffer);

            send(new_fd, get_buffer, 4096, 0);
            printf("SERVER: send PUT to node: %s success.\n", 
                target_node->service);

        } else {
            printf("method is: %d", incoming_package->method);
            perror("unrecognised method\n");
        }
        free(key_buffer);
        free(get_buffer);
        free(value_buffer);
        free(incoming_package);
        free(recvbuf);
        close(new_fd);
    }
}
