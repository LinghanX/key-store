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
/*
 * The idea of isolating open_clientfd and open_listenfd ogirinated from
 * CSAPP: 2nd edition, section 11.4
 */

int open_clientfd(char *hostname, char *port);
int open_listenfd(char *port, int listenq);

#endif
