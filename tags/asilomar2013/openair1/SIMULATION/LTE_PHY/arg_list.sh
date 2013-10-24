#!/bin/bash

MCS="0 1 2 3 4 5 6 7 8 9 10 11 12 13 14 15 16 17 18 19 20 21 22 23 24 25 26 27";
NSIMUS=10000

for M in $MCS
do
echo "-x1 -y1 -z1 -a -R1 -s$(($M-7)) -S7 -m$M -n$NSIMUS -f.2"
done