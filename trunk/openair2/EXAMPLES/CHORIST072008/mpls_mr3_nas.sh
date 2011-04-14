#!/bin/bash

# Author: Lamia Romdhani, Raymond Knopp
#June 2008

modprobe mpls6

echo Setting MPLS for mr3

echo 'MN2->MN1'
# add label 2000 and forward the packets to mr2 , for destination MN1 


var=`mpls nhlfe add key 0 instructions push gen 2000 nexthop nasmesh0 ipv6 2001:192:168:31::1 |grep key | cut -c 17-26`

#ip route add 2001:660:5502::15/64 via 2001:192:168:31::1 mpls $var
ip -6 route add 2001:660:5502::15/128 via 2001:192:168:31::1 mpls $var

ip -6 route add 2001:660:5502::25/128 dev eth0

#var=`mpls nhlfe add key 0 instructions push gen 2000 nexthop nasmesh0 ipv6 2001::5 |grep key | cut -c 17-26`

#ip route add 2000::/64 via 2001::5 mpls $var

echo 'MN1->MN2'
#pop label 1001 and do Ip lookup.

mpls labelspace set dev nasmesh0 labelspace 0
mpls ilm add label gen 1001 labelspace 0 proto ipv6 

