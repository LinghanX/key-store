all: build

build: clean server client node 

server: 
	gcc -o server server.c
client:
	gcc -o client client.c 
node: 
	gcc -o node node.c dict.c
clean: 
	rm -f client server node
