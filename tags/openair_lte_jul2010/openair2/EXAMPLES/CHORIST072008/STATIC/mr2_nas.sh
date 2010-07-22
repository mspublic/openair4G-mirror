#!/bin/bash
#Philippe huu nghia, 8 july 08
source params.sh

sysctl -w net.ipv6.conf.all.forwarding=1
$OPENAIR3_SCRIPTS_PATH/mr2_del_mpls.sh
$OPENAIR3_SCRIPTS_PATH/mr2_conf_nas.sh 
$OPENAIR3_SCRIPTS_PATH/mr2_mpls_nas.sh

#/etc/init.d/radvd status
#/etc/init.d/radvd start
#radvdump 

sysctl -w net.ipv6.conf.eth0.proxy_ndp=1
sysctl -w net.ipv6.conf.nasmesh0.proxy_ndp=1

ifconfig eth0 promisc
ip -6 addr add $MR2_EG_ADDR/64 dev eth0
$OPENAIR3_PMIP6D_PATH/pmip6d -m -s -L $CH_ADDR -N $MR2_EG_ADDR -E $MR2_IN_ADDR
$OPENAIR3_SCRIPTS_PATH/mr2_del_mpls.sh

#telnet localhost 7777 and then type pmip to see all binding entries
#ip -6 rule
#ip -6 route

