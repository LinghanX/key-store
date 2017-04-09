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
#define GET (1)
#define PUT (2)
#define ADD (3)
#define DROP (4)
#define MAX_NODE_NO (100)

struct circle {
    struct node_info node;
    struct circle* next;
};

void reset_node(struct node_info target_node){
    struct info_package reset_info;
    reset_info.method = DROP;
    reset_info.key_size = 8;
    reset_info.value_size = 8;

    int node_fd = open_clientfd(target_node.addr,
                                target_node.service);
    send(node_fd, &reset_info, sizeof(struct info_package), 0);
}

struct node_info find_post_node(struct node_info* available_nodes,
                                int num_of_nodes,
                                char* address){
    struct node_info node;
    node.addr = strtok(address, ":");
    node.service= strtok(NULL, "");
    unsigned long node_hash_value = key_value(node);

    int i;
    for(i = 0; i < num_of_nodes; i++){
        if(key_value(available_nodes[i]) == node_hash_value){
            if( i+1 == num_of_nodes )
                return available_nodes[0];
            return available_nodes[i+1];
        }
    }
}
void remove_node(struct node_info *nodes, int num, struct node_info node) {
    int i = 0;
    for(i = 0; i < num; i ++) {
        if(key_value(nodes[i]) == key_value(node)) {
            // found node to be removed
            if(i == num - 1) return;
            else {
                nodes[i] = nodes[num - 1];
                return;
            }
        }
    }
}

// server should be given:  number of nodes, address of data nodes
int main(int argc, char *argv[])
{
    int num_of_nodes = atoi(argv[1]);
    struct node_info available_nodes[MAX_NODE_NO];

    // initialize nodes info
    for(int i = 0; i < num_of_nodes; i++){
        available_nodes[i].addr = strtok(argv[i + 2], ":");
        available_nodes[i].service = strtok(NULL, " ");
        available_nodes[i].entries = 0;
    }


    printf("available nodes: %s\n%s\n%d", available_nodes[0].addr, available_nodes[0].service, available_nodes[0].entries);

    int sockfd, new_fd;
    struct sockaddr_storage their_addr;
    socklen_t sin_size;
    struct sigaction sa;

    sockfd = open_listenfd(PORTAL, CONNECTION_POOL);
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

        // everytime the server received a request, create a fork to process the request
        if(!fork()){
            close(sockfd);
            //receive struct msg
            if(recv(new_fd, &incoming_package, sizeof(struct info_package), 0) < 0){
                perror("recv error");
                exit(1);
            }
            printf("successfully received method: %d\nkey_size: %d\nvalue_size: %d\n", incoming_package.method,
                   (int)incoming_package.key_size, (int)incoming_package.value_size);

            char key_buffer[incoming_package.key_size], value_buffer[incoming_package.value_size];
            char get_buffer[4096];

            //receive key
            if(recv(new_fd, key_buffer, incoming_package.key_size, 0) < 0){
                perror("recv error");
                exit(1);
            }

            sort(available_nodes, num_of_nodes);
            unsigned long key_hashed_value = hash(key_buffer);
            struct node_info target_node = find_node(available_nodes,
                                                     num_of_nodes,
                                                     key_hashed_value);

            if(incoming_package.method == ADD){
                num_of_nodes++;

                available_nodes[num_of_nodes-1].addr = strtok(key_buffer, ":");
                available_nodes[num_of_nodes-1].service= strtok(NULL, "");

                printf("the added node's addr is: %s\n service is: %s\n",
                       available_nodes[num_of_nodes-1].addr,
                       available_nodes[num_of_nodes-1].service);
                printf("the original node's addr is: %s\n", key_buffer);
                sort(available_nodes, num_of_nodes);

                if(num_of_nodes != 1){
                    struct node_info post_node = find_post_node(available_nodes, num_of_nodes, key_buffer);
                    reset_node(post_node);
                }
            }

            if(incoming_package.method == DROP){
                struct node_info node_to_be_removed;
                node_to_be_removed.addr = strtok(key_buffer, ":");
                node_to_be_removed.service = strtok(NULL, "");

                remove_node(available_nodes, num_of_nodes, node_to_be_removed);
                num_of_nodes--;
                sort(available_nodes, num_of_nodes);

                printf("****************************************************************\n");
                printf("sorting completed\n");
                printf("****************************************************************\n");

                reset_node(node_to_be_removed);
            }

            if(incoming_package.method == GET){
                size_t entry_value =(size_t) hash(key_buffer);
                //int target_node = entry_value % num_of_nodes;

                //establish node connection
                int node_fd = open_clientfd(target_node.addr,
                                            target_node.service);

                send(node_fd, &incoming_package, sizeof(struct info_package), 0);
                send(node_fd, key_buffer, incoming_package.key_size, 0);

                recv(node_fd, get_buffer, 4096, 0);
                close(node_fd);

                send(new_fd, get_buffer, 4096, 0);
                exit(0);
            }

            if (incoming_package.method == PUT){
                if(recv(new_fd, value_buffer, incoming_package.value_size, 0) < 0){
                    perror("recv error");
                    exit(1);
                }

                size_t entry_value =(size_t) hash(key_buffer);

                //establish node connection
                int node_fd = open_clientfd(target_node.addr,
                                            target_node.service);
                send(node_fd, &incoming_package, sizeof(struct info_package), 0);
                send(node_fd, key_buffer, incoming_package.key_size, 0);
                send(node_fd, value_buffer, incoming_package.value_size, 0);

                recv(node_fd, get_buffer, 4096, 0);
                close(node_fd);
                send(new_fd, get_buffer, 4096, 0);
                exit(0);
            } else {
                perror("unrecognised method\n");
                exit(1);
            }
            close(new_fd);
        }
        close(new_fd);
    }
}
