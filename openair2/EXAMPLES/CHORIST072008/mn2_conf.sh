#!/bin/bash

echo setting interfaces on MN2
#ifconfig eth0 down

#Philippe 26 june 2008
ip -6 addr add 2001:660:5502::25/64 dev eth0 
#ifconfig eth0 add 2003::8/64 up
echo eth0 is 2001:660:5502::25/64
#/sbin/ip -6 route add (distination link @) via (attached mr interface @) dev (interface name of MN)

ip -6 route add 2001:660:5502::10/64 via 2001:660:5502::20 dev eth0
echo "route from MN2 to MN1 2001:660:5503::20/64"
#ip route add 172.16.10.0/24 via 172.16.20.4 dev eth2
