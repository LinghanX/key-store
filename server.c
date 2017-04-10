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
#define CLIENT_PORT (4096)

void print_node_info(struct node_info *available_nodes, int num_of_nodes) {
    int i = 0;
    printf("---start---\n");
    for(i = 0; i < num_of_nodes; i++) {
        printf("node-%d at %s:%s\n",
            i, available_nodes[i].addr, available_nodes[i].service);
    }
    printf("---end---\n");
}

struct circle {
    struct node_info node;
    struct circle* next;
};


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
            break;

        }
    } 

    if(i+1 == num_of_nodes) {
        return available_nodes[0];
    }
    return available_nodes[i+1];

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
//  
void talk(struct node_info *target_node, 
          struct info_package *outcoming_package, 
          char* key_buffer, 
          char* value_buffer, 
          char* get_buffer){

    printf("SERVER: send %s %s %s to node: %s:%s\n", 
    to_name(outcoming_package->method),
    key_buffer, 
    value_buffer,
    target_node->addr,
    target_node->service);

    int node_fd = open_clientfd(target_node->addr,
                                 target_node->service);

    send(node_fd, outcoming_package, sizeof(struct info_package), 0);
    send(node_fd, key_buffer, outcoming_package->key_size, 0);
    if(outcoming_package->value_size > 0)
    send(node_fd, value_buffer, outcoming_package->value_size, 0);

    recv(node_fd, get_buffer, 4096, 0);
    close(node_fd);

}

void reset_node(struct node_info target_node){
    struct info_package reset_info;
    reset_info.method = DROP;
    reset_info.key_size = 0;
    reset_info.value_size = 0;

    int node_fd = open_clientfd(target_node.addr,
                                target_node.service);
    send(node_fd, &reset_info, sizeof(struct info_package), 0);
    close(node_fd);
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


    int sockfd, new_fd;
    struct sockaddr_storage their_addr;
    socklen_t sin_size;
    struct sigaction sa;

    sockfd = open_listenfd(PORTAL, CONNECTION_POOL);
    printf("server: waiting for connections...\n");


    while(1){

        char recvbuf[4096];
        sin_size = sizeof(their_addr);

        new_fd = accept(sockfd, (struct sockaddr *)&their_addr, &sin_size);

        if(new_fd == -1){
            perror("accept");
            continue;
        }
        struct info_package incoming_package;

        // everytime the server received a request, create a fork to process the request
        //receive struct msg
        if(recv(new_fd, &incoming_package, sizeof(struct info_package), 0) < 0){
            perror("recv error");
            exit(1);
        }

        char* key_buffer = malloc(100);
        char get_buffer[4096];
        char value_buffer[100];

        printf("***before recv key\n");
        print_node_info(available_nodes, num_of_nodes);
        printf("***\n");

        //receive key
        if(recv(new_fd, key_buffer, incoming_package.key_size, 0) < 0){
            perror("recv error");
            exit(1);
        }
        printf("***before recv value\n");
        print_node_info(available_nodes, num_of_nodes);
        printf("***\n");

        if(incoming_package.value_size != 0) {
            if(recv(new_fd, value_buffer, incoming_package.value_size, 0) < 0){
                perror("recv error");
                exit(1);
            }
        }   
        printf("***before sorting\n");
        print_node_info(available_nodes, num_of_nodes);
        printf("***\n");

        sort(available_nodes, num_of_nodes);
        printf("***after sorting\n");
        print_node_info(available_nodes, num_of_nodes);
        printf("***\n");

        unsigned long key_hashed_value = hash(key_buffer);
        struct node_info *target_node = find_node(available_nodes,
                                                 num_of_nodes,
                                                 key_hashed_value);
        printf("***after find_node\n");
        print_node_info(available_nodes, num_of_nodes);
        printf("***\n");

        printf("SERVER: received %s %s %s; node: %s:%s\n", 
            to_name(incoming_package.method),
            key_buffer, 
            value_buffer,
            target_node->addr,
            target_node->service);
        // after recieve whatever, close fd;
        //close(new_fd);

        if(incoming_package.method == ADD){
            num_of_nodes++;

            char node_addr[256];
            strcpy(node_addr, key_buffer);

            available_nodes[num_of_nodes-1].addr = strtok(key_buffer, ":");
            available_nodes[num_of_nodes-1].service= strtok(NULL, "");
            printf("SERVER new node %s hash %lu\n", 
                node_addr, key_value(available_nodes[num_of_nodes-1]));

            sort(available_nodes, num_of_nodes);
            print_node_info(available_nodes, num_of_nodes);
            if(num_of_nodes != 1){
                struct node_info post_node = find_post_node(available_nodes, num_of_nodes, node_addr);
                reset_node(post_node);
            }
        }

        else if(incoming_package.method == DROP){
            struct node_info node_to_be_removed;
            node_to_be_removed.addr = strtok(key_buffer, ":");
            node_to_be_removed.service = strtok(NULL, "");

            remove_node(available_nodes, num_of_nodes, node_to_be_removed);
            num_of_nodes--;
            sort(available_nodes, num_of_nodes);

            printf("sorting completed\n");

            reset_node(node_to_be_removed);
        }

        else if(incoming_package.method == GET){
            // printf("SERVER: send GET %s to node: %s\n", 
            //     key_buffer, 
            //     //value_buffer,
            //     target_node.service);

            //establish node connection
            incoming_package.value_size = 0;
            // int node_fd = open_clientfd(target_node.addr,
            //                             target_node.service);

            // send(node_fd, &incoming_package, sizeof(struct info_package), 0);
            // send(node_fd, key_buffer, incoming_package.key_size, 0);

            // recv(node_fd, get_buffer, 4096, 0);
            // close(node_fd);
            talk(target_node, 
                 &incoming_package, 
                 key_buffer, 
                 value_buffer, 
                 get_buffer);


            send(new_fd, get_buffer, 4096, 0);
        }

        else if (incoming_package.method == PUT){
            // printf("SERVER: send PUT %s %s to node: %s\n", 
            //     key_buffer, 
            //     value_buffer,
            //     target_node.service);

            //establish node connection
            talk(target_node, 
                 &incoming_package, 
                 key_buffer, 
                 value_buffer, 
                 get_buffer);

            // int node_fd = open_clientfd(target_node.addr,
            //                             target_node.service);
            // send(node_fd, &incoming_package, sizeof(struct info_package), 0);
            // send(node_fd, key_buffer, incoming_package.key_size, 0);
            // send(node_fd, value_buffer, incoming_package.value_size, 0);

            // recv(node_fd, get_buffer, 4096, 0);
            // close(node_fd);
            send(new_fd, get_buffer, 4096, 0);
            printf("SERVER: send PUT to node: %s success.\n", 
                target_node->service);

        } else {
            printf("method is: %d", incoming_package.method);
            perror("unrecognised method\n");
        }
        free(key_buffer);
        close(new_fd);

    }
}
