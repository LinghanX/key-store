#ifndef HELPER_H
#define HELPER_H

/*
 * The idea of isolating open_clientfd and open_listenfd ogirinated from
 * CSAPP: 2nd edition, section 11.4
 */

int open_clientfd(char *hostname, char *port);
int open_listenfd(char *port, int listenq);

#endif
