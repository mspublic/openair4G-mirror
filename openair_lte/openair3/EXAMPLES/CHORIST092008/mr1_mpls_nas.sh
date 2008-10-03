#!/bin/bash
# Script by Lamia Romdhani, Nghia, Philippe 08 july 2008	 
# June 2008

source params.sh
sudo modprobe mpls6
echo Setting MPLS for mr1

sudo sh -c 'echo 0 > /sys/mpls/debug'

echo 'MN2->MN1'
sudo mpls labelspace set dev nasmesh0 labelspace 0
sudo mpls ilm add label gen $MR1_LABEL_IN labelspace 0 proto ipv6

echo 'MN1->MN2'
var=`mpls nhlfe add key 0 instructions push gen $MR1_LABEL_OUT nexthop nasmesh0 ipv6 $CH1_IN6_ADDR | grep key |cut -c 17-26`

echo "Creating routes"
sudo ip -6 route add $MN2_IN6_ADDR/128 via $CH1_IN6_ADDR mpls $var
sudo ip -6 route add $MN1_IN6_ADDR/128 dev eth2



