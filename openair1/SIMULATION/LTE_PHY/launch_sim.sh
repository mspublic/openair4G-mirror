#!/bin/bash
# PBS Options 
#PBS -m abe 
#PBS -d /homes/kaltenbe/Devel/openair_lte/openair1/SIMULATION/LTE_PHY

MCS=" 5 " 

#QPSK
#MCS="0 1 2 3 4 5 6 7 8 9"
#16QAM
#MCS="10 11 12 13 14 15 16"
#64QAM
#MCS="17 18 19 20 21 22"
#MCS="23 24 25 26 27 28"

for M in $MCS
do
    ./dlsim   -m$M -ga -s-1 -x1 -y1 -z1  -M1 -N20 -f0.2
done
