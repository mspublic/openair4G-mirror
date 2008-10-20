#!/bin/bash
# Author: Lamia Romdhani, Raymond Knopp

source params.sh
export OPENAIR2_DIR
#echo "0" >/proc/sys/net/ipv6/conf/all/forwarding

echo Installing NASMESH Driver
sudo rmmod -f nasmesh
sudo insmod $OPENAIR2_DIR/NAS/DRIVER/MESH/nasmesh.ko nas_IMEI=0x12345678,0x03000000

echo Classifcation rule for DTCH-Broadcast -reception of Router ADV
$OPENAIR2_DIR/NAS/DRIVER/MESH/RB_TOOL/rb_tool -a -c0 -i0 -z0 -x ff02::1 -y ff02::1 -r 3

echo Configuring interfaces on mr2
sudo route add -net 224.0.0.0 netmask 240.0.0.0 dev eth0

sudo ifconfig nasmesh0 up
sudo ifconfig nasmesh0 10.0.0.3
sudo ip -6 addr add 2001:10:0:1:7856:3412:0:3/64 dev nasmesh0

echo Launching AS simulator
sudo xterm -hold -e sh start_openair2_mr2.sh &

#echo Waiting for Router ADV from CH
#IPv6ADR=`ip addr show dev nasmesh0 | grep -e inet6 | egrep -v fe80 | cut -d " " -f6 | cut -d "/" -f1`
#while [ -z $IPv6ADR ] ; do 
#  sleep 1 
#  IPv6ADR=`ip addr show dev nasmesh0 | grep -e inet6 | egrep -v fe80 | cut -d " " -f6 | cut -d "/" -f1`
#done
#echo Got Router ADV : IPv6 address is $IPv6ADR

echo Classification rules for MR2 - Default DTCH UL for L3 signaling
#$OPENAIR2_DIR/NAS/DRIVER/MESH/RB_TOOL/rb_tool -a -c1 -i0 -z0 -x $IPv6ADR -y $CH_ADDR -r 4
$OPENAIR2_DIR/NAS/DRIVER/MESH/RB_TOOL/rb_tool -a -c1 -i0 -z0 -x 2001:10:0:1:7856:3412:0:3 -y $CH_ADDR -r 4
#$OPENAIR2_DIR/NAS/DRIVER/MESH/RB_TOOL/rb_tool -a -c1 -i0 -z0 -x 2001:10:0:1::9a03 -y 2001:10:0:1::1 -r 4
echo Classification rules for MR2 - MPLS User-plane Bearer
$OPENAIR2_DIR/NAS/DRIVER/MESH/RB_TOOL/rb_tool -a -c1 -i0 -z0 -l $MR2_LABEL_OUT -m $MR2_LABEL_IN -r 5

#echo Starting routing ...
#echo No MPLS debug
#echo "0" >/sys/mpls/debug


