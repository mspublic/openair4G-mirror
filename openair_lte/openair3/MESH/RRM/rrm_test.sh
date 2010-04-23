#!/bin/bash
killall emul_int
killall graph
killall rrm

xterm -T CMM -hold -e ./emul_int  &

sleep 2

#xterm -T RRM1 -hold -e 
./rrm -i 5 #2> log.txt

#sleep 1
#./emul_int  &


