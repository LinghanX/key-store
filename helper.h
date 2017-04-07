#ifndef HELPER_H
#define HELPER_H

#include <stdlib.h>

struct info_package {
    int method;
    size_t key_size, value_size;
};

struct node_info {
    char *addr, *service;
    int entries;
};

unsigned long hash(unsigned char *s){
    unsigned long hash = 5381;
    int c;

    while( c = *s++ ){
        hash = ((hash <<5) + hash) + c;
    }
    return hash;
}
/*
 * The idea of isolating open_clientfd and open_listenfd ogirinated from
 * CSAPP: 2nd edition, section 11.4
 */

int open_clientfd(char *hostname, char *port);
int open_listenfd(char *port, int listenq);

void sort(struct node_info *nodes, int num_of_nodes);
struct node_info find_node(struct node_info* available_nodes,
                           int size,
                           unsigned long key_hashed_value);
#endif
