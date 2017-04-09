#include <string.h>

int main(int argc , char *argv[]) {
    int no_nodes = 10;
    if(argc == 2) {
        no_nodes = atoi(argv[1]);
    }
    int i;
    int port = 3001;
    for(i = 0; i < no_nodes; i++) {
        if(!fork()) {
            // new node process
            char *args[] = {"node", port + i, 0};
            char *env[] = { 0 };
            execve('./node', args, env);

            printf("error exec.\n");
            break;
        }
    }
    char strBuf[265];
    if(!fork()) {
        // new server process
        char *args[2 + no_nodes];
        args[0] = "server";
        args[1] = no_nodes;
        for(i = 0; i < no_nodes; i ++) {
            char addr[10];
            sprintf(addr, "%d", port + i);
            strcpy(strBuf, "localhost:");
            strcat(strBuf, addr);

            printf("address of the node is: %s", strBuf);
            args[2 + i] = strBuf;
        }

        char *env[] = { 0 };
        execve('./server', args, env);

        printf("error exec.\n");
        break;
    }
}