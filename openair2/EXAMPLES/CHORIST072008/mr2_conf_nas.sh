#!/bin/bash

# Author: Lamia Romdhani, Raymond Knopp


echo "1" >/proc/sys/net/ipv6/conf/all/forwarding
Installing NASMESH driver
rmmod -f nasmesh
insmod /root/openair2/NAS/DRIVER/MESH/nasmesh.ko
echo Classification rules for CH

#CH Broadcast
/root/openair2/NAS/DRIVER/MESH/RB_TOOL/rb_tool -a -c1 -i0 -z0 -x 2001:192:168:31::1 -y FF02::1 -r 3

#CH<-> MR1 IP Signaling
/root/openair2/NAS/DRIVER/MESH/RB_TOOL/rb_tool -a -c1 -i0 -z0 -x 2001:192:168:31::1 -y 2001:192:168:31::2 -r 12
#CH<-> MR1 MPLS user-plane bearer
/root/openair2/NAS/DRIVER/MESH/RB_TOOL/rb_tool -a -c1 -i0 -z0 -l 2001 -m 1000 -r 13

#CH<-> MR2 (IP Signaling)
/root/openair2/NAS/DRIVER/MESH/RB_TOOL/rb_tool -a -c2 -i0 -z0 -x 2001:192:168:31::1 -y 2001:192:168:31::3 -r 20
#CH<-> MR2 (MPLS user-plane bearer)
/root/openair2/NAS/DRIVER/MESH/RB_TOOL/rb_tool -a -c2 -i0 -z0 -l 1001 -m 2000 -r 21

echo Configuring interfaces on CH

# Setup IPv4 multicast route for openair emulation
ifconfig eth0 192.168.31.1
route add -net 224.0.0.0 netmask 240.0.0.0 dev eth0

# Bring up openair NASMESH device and set IPv6 address
ifconfig nasmesh0 up
ip -6 addr add 2001:192:168:31::1/64 dev nasmesh0
echo nasmesh0 is 2001:192:168:31::1/64
echo No MPLS debug
echo "0" >/sys/mpls/debug

sleep 1
echo Configuring MPLS
sh del_mpls.sh
sh mpls_mr2_nas.sh

sleep 1
echo Launching AS simulator

cd /root/openair2/SIMULATION/USER_TOOLS/LAYER2_SIM
./mac_sim 0
