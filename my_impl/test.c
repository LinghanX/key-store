#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include "request.h"
#include <unistd.h>
#include <signal.h>

#define SERVER_PORT 8060
#define ROUTER_PORT 8000
#define IP_ADDR "127.0.0.1"
int main(int argc, char *argv[]) {
	// create servers
	int i;
	pid_t pid;
	int n; //number of servers
	if(argc > 1) {
		sscanf(argv[1], "%d", &n);
	} else {
		n = 10;
	}
	printf("test: number of servers %d\n", n);
	pid_t pids[n+2];
	char* saddrs[n];

	for(i = 0; i < n; i++) {

		int port = SERVER_PORT + i;
		char str[10] = "";
		sprintf(str, "%d", port);

		char server_addr[50] = "";
       	strcat(server_addr, IP_ADDR);
       	strcat(server_addr, ":");
       	strcat(server_addr, str);
       	saddrs[i] = malloc(sizeof(char) * strlen(server_addr));
       	strncpy(saddrs[i], server_addr, strlen(server_addr));
		
		//printf("new server addr %s\n", saddrs[i]);
		pid = fork();
		pids[i] = pid;
		if(pid < 0) {
      		perror("fork failed.\n");
        	exit(1);
    	}
	    if(pid == 0) {
	        //child process
	        char *args[] = {"server", IP_ADDR, str, 0 }; 
    		char *env[] = { 0 }; 
			execve("./server", args, env);

			printf("error exec.\n");

			break;
	    } 
	}

	// printf("created %d servers.\n", i);

	// create a rounter
	pid = fork();
	pids[n] = pid;
	if(pid < 0) {
  		perror("fork failed.\n");
    	exit(1);
	}
    if(pid == 0) {
        //child process
		char str[10];
		sprintf(str, "%d", ROUTER_PORT);
        char *args[n+3];
        args[0] = "router";
        args[n+2] = 0;
       	char router_addr[50];
       	strcat(router_addr, IP_ADDR);
       	strcat(router_addr, ":");
       	strcat(router_addr, str);

        args[1] = router_addr;
        for(i = 2; i < n + 2; i++) {
        	args[i] = saddrs[i-2];
        }

		char *env[] = { 0 }; 
		execve("./router", args, env);

		printf("error exec.\n");
    } 

	// create the client
	pid = fork();
	pids[n+1] = pid;
	if(pid < 0) {
  		perror("fork failed.\n");
    	exit(1);
	}
    if(pid == 0) {
        //child process
		char str[10];
		sprintf(str, "%d", ROUTER_PORT);

        char *args[4];
        args[0] = "client";
        args[1] = IP_ADDR;
        args[2] = str;
        args[3] = 0;

		char *env[] = { 0 }; 
		execve("./client", args, env);

		printf("error exec.\n");
    }
    sleep(1000);
    for(i = 0; i < n+2; i++) {
    	kill(pids[i], SIGKILL);
    }

}