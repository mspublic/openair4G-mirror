#!/bin/bash
# PBS Options
#PBS -m abe
#PBS -d /homes/kaltenbe/Devel/openair_trunk/openair1/SIMULATION/PHY/CHBCH

# print this script to stdout for later reference (in the chbch_sim.sh.oxxxxx file)

cat chbch_sim.sh

# Launch work (compile, execute, etc)

N_TX=2
N_RX=2
FRAMES=10000
ERRORS=5000
RICE=0
AOA=2
FREQ=10
BETA_DB="-6 -3 0 3 6 9"
#BETA_DB="-5 0 5 10"
S1_DB="35 40 45"
#S2_DB="55 60"
N0="30"
ICFLAG=0

for S1 in $S1_DB
do
  for BETA in $BETA_DB 
    do
    chbch_sim $S1 $N0 $FRAMES $N_TX $N_RX $ERRORS $RICE $AOA $FREQ $ICFLAG $(($S1-$BETA))
  done
done
