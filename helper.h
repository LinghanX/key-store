#ifndef HELPER_H
#define HELPER_H

#include <stdlib.h>

#define GET (1)
#define PUT (2)
#define ADD (3)
#define DROP (4)

struct info_package {
    int method;
    size_t key_size, value_size;
};

struct node_info {
    char addr[256];
    char service[256];
    int entries;
};

unsigned long hash(char *s);
unsigned long key_value(struct node_info node);
/*
 * The idea of isolating open_clientfd and open_listenfd ogirinated from
 * CSAPP: 2nd edition, section 11.4
 */
char* to_name(int method);

void print_node_info(struct node_info *available_nodes, int num_of_nodes);
struct node_info find_post_node(struct node_info *available_nodes,
                                int num_of_nodes,
                                char* address);
void remove_node(struct node_info *nodes,
                 int num_of_nodes,
                 struct node_info *target_node);
void talk(struct node_info *target_node,
          struct info_package *outcoming_package,
          char* key_buffer,
          char* value_buffer,
          char* get_buffer);
void reset_node(struct node_info target_node);
int open_clientfd(char *hostname, char *port);
int open_listenfd(char *port, int listenq);

void sort(struct node_info *nodes, int num_of_nodes);
struct node_info* find_node(struct node_info* available_nodes,
                           int size,
                           unsigned long key_hashed_value);
#endif
