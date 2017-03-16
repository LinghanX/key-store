all: build

build: clean server client node 

server: 
	gcc -o server server.c helper.h helper.c
client:
	gcc -o client client.c helper.h helper.c
node: 
	gcc -o node node.c dict.c helper.h helper.c -lpthread
clean: 
	rm -f client server node
