#!/bin/bash
# Author: Lamia Romdhani, Raymond Knopp

echo "1" >/proc/sys/net/ipv6/conf/all/forwarding

echo Installing NASMESH Driver
rmmod -f nasmesh
insmod $OPENAIR2_DIR/NAS/DRIVER/MESH/nasmesh.ko

echo Classification rules for MR1 - Default DTCH UL for L3 signaling
$OPENAIR2_DIR/NAS/DRIVER/MESH/RB_TOOL/rb_tool -a -c0 -i0 -z0 -x 2001:192:168:31::2 -y 2001:192:168:31::1 -r 4

echo Classification rules for MR1 - MPLS User-plane Bearer
$OPENAIR2_DIR/NAS/DRIVER/MESH/RB_TOOL/rb_tool -a -c0 -i0 -z0 -l 1000 -m 2001 -r 5

echo Configuring interfaces on mr1 (eth4 for emulation, eth0 for IPv6/MPLS forwarding, nasmesh0 for MPLS+IPv6 signaling)
ifconfig eth4 192.168.31.2
route add -net 224.0.0.0 netmask 240.0.0.0 dev eth4
ifconfig nasmesh0 up
ip -6 addr add 2001:660:5502::10/64 dev eth0
ip -6 addr add 2001:192:168:31::2/64 dev nasmesh0

echo eth0 is 2001:660:5502::10/64
echo nasmesh0 is 2001:192:168:31::2/64
echo Starting routing ...
echo No MPLS debug
echo "0" >/sys/mpls/debug

echo MPLS setup
sh del_mpls.sh
sh mpls_mr1_nas.sh

echo Launching AS simulator
cd $OPENAIR2_DIR/SIMULATION/USER_TOOLS/LAYER2_SIM
./mac_sim 1
