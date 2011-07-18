#!/bin/bash  

 beginScript=`date`
 
 ################### 
 # Global Variables 
 ################### 
 
#-
####
insmod NAS/DRIVER/MESH/nasmesh.ko 

# Clusterheads 
#CH 0
ifconfig nasmesh0 10.0.1.1 netmask 255.255.255.0 broadcast 10.0.1.0

# UE nodes 
#UE 8
ifconfig nasmesh8 10.0.9.9 netmask 255.255.255.0 broadcast 10.0.9.0
#UE 9
ifconfig nasmesh9 10.0.10.10 netmask 255.255.255.0 broadcast 10.0.10.0

# Nas driver config 
NAS/DRIVER/MESH/RB_TOOL/rb_tool -a -c0 -i0 -z0 -s 10.0.1.1 -t 10.0.1.9 -r 12
NAS/DRIVER/MESH/RB_TOOL/rb_tool -a -c0 -i1 -z0 -s 10.0.9.9 -t 10.0.9.1 -r 4 
NAS/DRIVER/MESH/RB_TOOL/rb_tool -a -c1 -i0 -z0 -s 10.0.1.1 -t 10.0.1.10 -r 20
NAS/DRIVER/MESH/RB_TOOL/rb_tool -a -c0 -i2 -z0 -s 10.0.10.10 -t 10.0.10.1 -r 4 

