#!/bin/bash

# load rtai modules
MODULES_HOME=/usr/realtime/modules
insmod $MODULES_HOME/rtai_hal.ko
insmod $MODULES_HOME/rtai_lxrt.ko
insmod $MODULES_HOME/rtai_fifos.ko
insmod $MODULES_HOME/rtai_sem.ko
insmod $MODULES_HOME/rtai_msg.ko

sudo rmmod nasmesh
sudo insmod $OPENAIR2_DIR/NAS/DRIVER/MESH/nasmesh.ko
# ip forward
iptables -A FORWARD -o eth0 -i oai0 -s 10.0.1.0/24 -m conntrack --ctstate NEW -j ACCEPT
iptables -A FORWARD -m conntrack --ctstate ESTABLISHED,RELATED -j ACCEPT
iptables -t nat -F POSTROUTING
iptables -t nat -A POSTROUTING -o eth0 -j MASQUERADE

