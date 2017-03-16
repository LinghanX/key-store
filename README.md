## a distributed key-value store system
This is an implementation of a distributed key value store system. When a `put` request is generated by the `client`, `server` will generate a randomised algorithm to decide which node to store the data in, the idea is that due to randomization, the load of local storage should be distributed evenly accross all available nodes. 

After the target `node` is decided, `server` will send both `key` and `value` to the `node` and expect to get a `successful` response. 
The `get` request works in a similar way. The algorithm implementation ensures that for each unique `key`, the target `node` remains the same.

A `pthread_mutex_lock` was employed in the `put` section to protect the hash table when contention happens

## Set up server, node, and client

0. `$ make`
1. First bring the `node` online since it's where all the data is supposed to be by `./node {port}`, eg. `./node 3345`, `./node 3346`, `./node 3347`
2. Then bring `server` online by putting how many nodes are available and their ip addressed, for example `$ ./server 3 localhost:3345 localhost:3346 localhost:3347`
3. Then the `client` could test using `./client localhost:3344 put key value`, note that the `localhost:3344` is the IP address of server

## helpful resources that made this happen
1. [Bryant & O'HALLARON]: Computer Systems: a programmer's perspective
2. Beej's guide for network programming
3. Operating systems: three easy pieces
4. cs.yale.edu for the handy hashing method
