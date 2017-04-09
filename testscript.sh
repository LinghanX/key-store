./node 3001 localhost:3344 &
./node 3002 localhost:3344 &
./node 3003 localhost:3344 &
./node 3004 localhost:3344 &
./server 3 localhost:3001 localhost:3002 localhost:3003 &
./client localhost:3344 put beijing china
./client localhost:3344 put paris france
./client localhost:3344 put dehli india
./client localhost:3344 put london unitedkindom

./client localhost:3344 get beijing
./client localhost:3344 get london 
./client localhost:3344 get dehli 

./client localhost:3344 add localhost:3004
./client localhost:3344 drop localhost:3001
./client localhost:3344 drop localhost:3002
./client localhost:3344 drop localhost:3003

./client localhost:3344 get beijing
./client localhost:3344 get beijing
./client localhost:3344 get beijing
./client localhost:3344 get beijing
./client localhost:3344 get beijing
./client localhost:3344 get beijing
./client localhost:3344 get beijing
./client localhost:3344 get beijing
./client localhost:3344 get beijing

sleep 3
pkill -P $$
