#!/bin/bash
source params.sh

#Author: Lamia Romdhani
echo Configuring interfaces on mr3

#Modified by Huu-Nghia, Philippe
#ip -6 addr add $MR2_IN_ADDR/64 dev eth2 
ip -6 addr add $MR2_EG_ADDR/64 dev eth0

#echo eth2 is $MR2_IN_ADDR/64
echo eth0 is $MR2_EG_ADDR/64

echo Starting routing ...
echo "1" >/proc/sys/net/ipv6/conf/all/forwarding
#echo 0 >/proc/sys/net/ipv6/conf/all/rp_filter
echo No MPLS debug
echo "0" >/sys/mpls/debug
