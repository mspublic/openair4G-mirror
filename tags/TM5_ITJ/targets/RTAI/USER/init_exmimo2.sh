#!/bin/bash

PCI=`lspci -m | grep Xilinx`
if [ -z "$PCI" ]; then
 echo "No card found. Stopping!"
 return
fi

sudo rmmod openair_rf
sudo insmod $OPENAIR_TARGETS/ARCH/EXMIMO/DRIVER/eurecom/openair_rf.ko
sleep 1

if [ ! -e /dev/openair0 ]; then 
 sudo mknod /dev/openair0 c 127 0
 sudo chmod a+rw /dev/openair0
fi

DEVICE=`echo $PCI | awk -F\" '{print $(NF-1)}' | awk '{print $2}'`
if [ $(($DEVICE%100)) == 8 ]; then
 echo "Using firmware version 8"
 $OPENAIR_TARGETS/ARCH/EXMIMO/USERSPACE/OAI_FW_INIT/updatefw -s 0x43fffff0 -b -f $OPENAIR_TARGETS/ARCH/EXMIMO/USERSPACE/OAI_FW_INIT/sdr_expressmimo2
else 
 if [ $(($DEVICE%100)) == 9 ]; then
  echo "Using firmware version 9"
  $OPENAIR_TARGETS/ARCH/EXMIMO/USERSPACE/OAI_FW_INIT/updatefw -s 0x43fffff0 -b -f $OPENAIR0_DIR/express-mimo/software/sdr/exmimo2/sdr_expressmimo2
  #$OPENAIR_TARGETS/ARCH/EXMIMO/USERSPACE/OAI_FW_INIT/updatefw -s 0x43fffff0 -b -f $OPENAIR_TARGETS/ARCH/EXMIMO/USERSPACE/OAI_FW_INIT/sdr_expressmimo2_v9
 else
  echo 'No corresponding firmware found'
  return
 fi
fi

sudo rmmod nasmesh
sudo insmod $OPENAIR2_DIR/NAS/DRIVER/MESH/nasmesh.ko

if [ "$1" = "eNB" ]; then 
     echo "bring up oai0 interface for enb"
     sudo ifconfig oai0 10.0.1.1 netmask 255.255.255.0 broadcast 10.0.1.255
     $OPENAIR2_DIR/NAS/DRIVER/MESH/RB_TOOL/rb_tool -a -c0 -i0 -z0 -s 10.0.1.1 -t 10.0.1.9 -r 3
else
    if [ "$1" = "UE" ]; then 
	echo "bring up oai0 interface for UE"
	sudo ifconfig oai0 10.0.1.9 netmask 255.255.255.0 broadcast 10.0.1.255
	$OPENAIR2_DIR/NAS/DRIVER/MESH/RB_TOOL/rb_tool -a -c0 -i0 -z0 -s 10.0.1.9 -t 10.0.1.1 -r 3
    fi
fi  
