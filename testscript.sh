./node 3001 localhost:3344  >/dev/null&
./node 3002 localhost:3344  >/dev/null&
./node 3003 localhost:3344  >/dev/null&
./node 3004 localhost:3344  >/dev/null&
./node 3005 localhost:3344  >/dev/null&
./node 3006 localhost:3344  >/dev/null&
./node 3007 localhost:3344  >/dev/null&
./node 3008 localhost:3344  >/dev/null&
./node 3009 localhost:3344  >/dev/null&
./node 3010 localhost:3344  >/dev/null&

sleep 1

./server 5 localhost:3001 localhost:3002 localhost:3003 localhost:3004 localhost:3005  >/dev/null &
./client localhost:3344 put beijing china
./client localhost:3344 put paris france
./client localhost:3344 put dehli india
./client localhost:3344 put london unitedkindom
./client localhost:3344 put 3333333333 4444444444
./client localhost:3344 put 12312312313 22222222
./client localhost:3344 put 32123 321323
./client localhost:3344 put theoryCraft unitedkindom
./client localhost:3344 put somethingnew unitedkindom
./client localhost:3344 put pufffffffff unitedkindom
./client localhost:3344 put randomDude unitedkindom

./client localhost:3344 get beijing
./client localhost:3344 get london 
./client localhost:3344 get dehli 

./client localhost:3344 add localhost:3006
sleep 1
./client localhost:3344 add localhost:3007
sleep 1
./client localhost:3344 add localhost:3008
sleep 1
./client localhost:3344 add localhost:3009
sleep 1
./client localhost:3344 add localhost:3010
sleep 1

./client localhost:3344 drop localhost:3001
sleep 1
./client localhost:3344 drop localhost:3002
sleep 1
./client localhost:3344 drop localhost:3003
sleep 1
./client localhost:3344 drop localhost:3004
sleep 1
./client localhost:3344 drop localhost:3005
sleep 1

./client localhost:3344 get 3333333333
./client localhost:3344 get paris
./client localhost:3344 get randomDude

echo "closing .."
sleep 1
pkill -P $$ 
