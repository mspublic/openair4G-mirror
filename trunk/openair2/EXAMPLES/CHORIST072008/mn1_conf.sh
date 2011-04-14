#!/bin/bash
# Lamia 28 june 2008

echo setting the interfaces on MN1

#ip -6 addr add 2000::9/64 dev eth0
ip -6 addr add 2001:660:5502::15/64 dev eth0


echo "eth0 is 2001:660:5502::15/64"

#lower MTU because video broadcasting sends pretty big packets
#ifconfig eth1 add 2000::9/64 mtu 1400 up


#/sbin/ip -6 route add (distination link @) via (attached mr interface @) dev (interface name of MN)

ip -6 route add host 2001:660:5502::25/128 via 2001:660:5502::10 dev eth0

#ip -6 addr add 2000::9/64 dev eth0

#ip -6 route add 2003::8/64 via 2000::3 dev eth0


