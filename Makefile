all: build

build: clean server client node 

server: server.c helper.c
	gcc -o server server.c helper.c
client: client.c helper.c
	gcc -o client client.c helper.c
node:   
	gcc -o node node.c dict.c helper.c -lpthread
clean: 
	rm -f client server node
