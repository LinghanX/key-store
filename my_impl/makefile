all: build

build: clean server client router test

server: 
	gcc -o server server.c
client:
	gcc -o client client.c
router:
	gcc -o router router.c
test:
	gcc -o test test.c
clean:
	rm -f client server test router
check: 
	./test $(server)