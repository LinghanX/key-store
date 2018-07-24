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
echo "start a kv store server with 5 nodes"
./server 5 localhost:3001 localhost:3002 localhost:3003 localhost:3004 localhost:3005  >/dev/null &
./client localhost:3344 put NewYork	NewYork	
./client localhost:3344 put LosAngeles California	
./client localhost:3344 put Chicago	Illinois	
./client localhost:3344 put Houston	Texas	
./client localhost:3344 put Philadelphia Pennsylvania	
./client localhost:3344 put Phoenix	Arizona	
./client localhost:3344 put SanAntonio Texas	
./client localhost:3344 put SanDiego California	
./client localhost:3344 put Dallas Texas	
./client localhost:3344 put SanJose California	
./client localhost:3344 put Austin Texas	
./client localhost:3344 put Jacksonville Florida	
./client localhost:3344 put SanFrancisco California	
./client localhost:3344 put Indianapolis Indiana	
./client localhost:3344 put Columbus Ohio	
./client localhost:3344 put FortWorth Texas	
./client localhost:3344 put Charlotte NorthCarolina	
./client localhost:3344 put Seattle	Washington	
./client localhost:3344 put Detroit	Michigan	
./client localhost:3344 put Denver Colorado	
./client localhost:3344 put Washington DistrictofColumbia	
./client localhost:3344 put Memphis Tennessee	
./client localhost:3344 put Boston Massachusetts	
./client localhost:3344 put Nashville Tennessee	
./client localhost:3344 put Baltimore Maryland	
./client localhost:3344 put OklahomaCity Oklahoma	
./client localhost:3344 put VirginiaBeach Virginia	

./client localhost:3344 get SanFrancisco 
./client localhost:3344 get Charlotte 
./client localhost:3344 get Columbus
./client localhost:3344 get LosAngeles 
./client localhost:3344 get FortWorth 
./client localhost:3344 get SanAntonio
./client localhost:3344 get SanAntonio


echo "add 5 nodes"
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
echo "drop 5 nodes"
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

./client localhost:3344 get SanFrancisco 
./client localhost:3344 get Charlotte 
./client localhost:3344 get Columbus
./client localhost:3344 get LosAngeles 
./client localhost:3344 get FortWorth 
./client localhost:3344 get SanAntonio
./client localhost:3344 get SanAntonio


echo "Closing server and nodes.."
sleep 1
pkill -P $$ 
