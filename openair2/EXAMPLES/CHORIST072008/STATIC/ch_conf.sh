#!/bin/bash

# Author: Lamia Romdhani

echo Configuring interfaces on mr2

#ifconfig eth0 down
#ifconfig eth4 down

ip -6 addr add 2001:192:168:31::1/64 dev eth0
echo eth0 is 2001:192:168:31::1/64

echo Starting routing ...
echo "1" >/proc/sys/net/ipv6/conf/all/forwarding
#echo 0 >/proc/sys/net/ipv6/conf/all/rp_filter
echo No MPLS debug
echo "0" >/sys/mpls/debug
