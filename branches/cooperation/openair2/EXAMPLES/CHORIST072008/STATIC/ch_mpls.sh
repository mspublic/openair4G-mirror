#!/bin/bash

#Script by Lamia Romdhani
#June 2008


echo Setting MPLS for CH 

modprobe mpls6

echo 'MN1->MN2'
#switching incoming label 1000 to 1001 and then forwarding the packet to mr3
#dev infce_of_recep
mpls labelspace set dev eth0 labelspace 0
mpls ilm add label gen 1000 labelspace 0 proto ipv6

#var=`mpls nhlfe add key 0 instructions push gen 1001 nexthop name_next_if_out ipv6 @next_if_dest |grep key | cut -c 17-26`

var=`mpls nhlfe add key 0 instructions push gen 1001 nexthop eth0 ipv6 2001:192:168:31::3 |grep key | cut -c 17-26`
mpls xc add ilm_label gen 1000 ilm_labelspace 0 nhlfe_key $var

#var=`mpls nhlfe add key 0 instructions push gen 1001 nexthop eth0 ipv6 2001::8 |grep key | cut -c 17-26`
#mpls xc add ilm_label gen 1000 ilm_labelspace 0 nhlfe_key $var

echo 'MN2->MN1'
#switching incoming label 2000 to 2001 and then forwarding the packet to mr1.
#mpls labelspace set dev eth4 labelspace 0

mpls labelspace set dev eth0 labelspace 0
mpls ilm add label gen 2000 labelspace 0 proto ipv6

var=`mpls nhlfe add key 0 instructions push gen 2001 nexthop eth0 ipv6 2001:192:168:31::2 | grep key |cut -c 17-26`
mpls xc add ilm_label gen 2000 ilm_labelspace 0 nhlfe_key $var
