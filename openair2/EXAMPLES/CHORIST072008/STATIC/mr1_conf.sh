#!/bin/bash
# Author: Nghia, Philippe 11 July 2008

source params.sh
echo Configuring interfaces on mr1
ip -6 addr add $MR1_EG_ADDR/64 dev eth0
ip -6 addr add $MR1_IN_ADDR/64 dev eth4

echo eth0 is $MR1_EG_ADDR/64
echo eth4 is $MR1_IN_ADDR/64
echo Starting routing ...
echo "1" >/proc/sys/net/ipv6/conf/all/forwarding
#echo 0 >/proc/sys/net/ipv6/conf/all/rp_filter
echo No MPLS debug
echo "0" >/sys/mpls/debug
