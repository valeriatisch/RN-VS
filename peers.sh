#!/bin/bash
ID1=1
ID2=2
ID3=3
ID4=4

IP1=127.0.0.1
IP2=127.0.0.1
IP3=127.0.0.1
IP4=127.0.0.1

PORT1=1400
PORT2=1402
PORT3=1405
PORT4=1407

gnome-terminal --geometry=50x20 -- ./peer $ID1 $IP1 $PORT1 $ID4 $IP4 $PORT4 $ID2 $IP2 $PORT2
sleep 1
gnome-terminal --geometry=50x20 -- ./peer $ID2 $IP2 $PORT2 $ID1 $IP1 $PORT1 $ID3 $IP3 $PORT3
sleep 1
gnome-terminal --geometry=50x20 -- ./peer $ID3 $IP3 $PORT3 $ID2 $IP2 $PORT2 $ID4 $IP4 $PORT4
sleep 1
gnome-terminal --geometry=50x20 -- ./peer $ID4 $IP1 $PORT4 $ID3 $IP3 $PORT3 $ID1 $IP1 $PORT1