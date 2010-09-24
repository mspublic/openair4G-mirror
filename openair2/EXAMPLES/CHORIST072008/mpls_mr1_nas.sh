#!/bin/bash

#Script by Lamia Romdhani 
# June 2008

modprobe mpls6

echo Setting MPLS for mr1

echo 'MN2->MN1'

#expect and pop label 2001
mpls labelspace set dev nasmesh0 labelspace 0
mpls ilm add label gen 2001 labelspace 0 proto ipv6

echo 'MN1->MN2'
#add label 1000 and forward the packet to mr2 on output interface eth1 for destination A2
var=`mpls nhlfe add key 0 instructions push gen 1000 nexthop nasmesh0 ipv6 2001:192:168:31::1 | grep key |cut -c 17-26`

ip -6 route add 2001:660:5502::25/128 via 2001:192:168:31::1 mpls $var

ip -6 route add 2001:660:5502::15/128 dev eth0
