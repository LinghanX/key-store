## a distributed key-value store system

This is an attempt to implement a distributed key-value store system, with a goal of maintain high availability and reliability.

The `server` is listenning on port: 3344


Use 
```
nc 0.0.0.0 3344
```

to send signal to server for testing


use:
```
./client "localhost"
```

to send signal to client for testing

## new edition 3/10
1. setup server using ./server 1 localhost:3345
2. setup node using ./node 
3. use client: ./client localhost:3344 get hello world
