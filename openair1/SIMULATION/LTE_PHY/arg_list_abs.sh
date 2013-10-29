#!/bin/bash

MCS1="0 1 2 3 4 5 6 7 8 9"
MCS2="10 11 12 13 14 15"
MCS3="16 17 18 19 20 21 22 23 24 25 26 27";
NSIMUS=200
NCHAN=200

for M in $MCS1
do
echo "-x1 -y1 -z1 -gC -R1 -s$(($M-5)) -S7 -m$M -n$NSIMUS -N$NCHAN -M -f.2"
done
for M in $MCS2
do
echo "-x1 -y1 -z1 -gC -R1 -s$(($M-7)) -S7 -m$M -n$NSIMUS -N$NCHAN -M -f.2"
done
for M in $MCS3
do
echo "-x1 -y1 -z1 -gC -R1 -s$(($M-9)) -S7 -m$M -n$NSIMUS -N$NCHAN -M -f.2"
done
