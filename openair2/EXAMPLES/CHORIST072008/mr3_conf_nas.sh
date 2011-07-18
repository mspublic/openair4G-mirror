#!/bin/bash
# Author: Lamia Romdhani, Raymond Knopp

echo "1" >/proc/sys/net/ipv6/conf/all/forwarding

rmmod -f nasmesh
echo Installing NASMESH Driver
insmod /root/openair2/NAS/DRIVER/MESH/nasmesh.ko

echo Classification rules for MR2 - Default DTCH UL for L3 signaling
/root/openair2/NAS/DRIVER/MESH/RB_TOOL/rb_tool -a -c0 -i0 -z0 -x 2001:192:168:31::3 -y 2001:192:168:31::1 -r 4
echo Classification rules for MR2 - MPLS User-plane Bearer
/root/openair2/NAS/DRIVER/MESH/RB_TOOL/rb_tool -a -c0 -i0 -z0 -l 2000 -m 1001 -r 5

echo Configuring interfaces on mr2

ifconfig eth2 192.168.31.3
route add -net 224.0.0.0 netmask 240.0.0.0 dev eth2

ifconfig nasmesh0 up
ip -6 addr add 2001:660:5502::20/64 dev eth0
ip -6 addr add 2001:192:168:31::3/64 dev nasmesh0

echo eth0 is 2001:660:5502::20/64
echo nasmesh0 is 2001:192:168:31::3/64
echo Starting routing ...
echo "1" >/proc/sys/net/ipv6/conf/all/forwarding
#echo 0 >/proc/sys/net/ipv6/conf/all/rp_filter
echo No MPLS debug
echo "0" >/sys/mpls/debug

echo Cleaning MPLS information
sh del_mpls.sh

echo Configuring MPLS
sh mpls_mr3_nas.sh

echo Launching AS Simulator
cd /root/openair2/SIMULATION/USER_TOOLS/LAYER2_SIM
./mac_sim 2
