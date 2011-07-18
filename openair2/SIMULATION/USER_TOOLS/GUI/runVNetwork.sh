#!/bin/bash  

 beginScript=`date`
 
 ################### 
 # Global Variables 
 ################### 
 nameServer=192.168.12.100
 defaultGw=192.168.12.100
 networkRootAdd=10.0.1. 
 virtualMachineInterface=eth0
 netMask=255.255.255.0
 bridgeName=vbr0 

 #Verify if the bridge isn't already defined 
 bridgeDefined=$(ifconfig | grep $bridgeName | wc -l) 
  
 if [ $bridgeDefined -lt 1 ]; then 
   echo "Creating the bridge $bridgeName" 
   brctl addbr $bridgeName 
   ifconfig $bridgeName 0 
   echo 1 > /proc/sys/net/ipv4/conf/$bridgeName/forwarding 
   echo 1 > /proc/sys/net/ipv4/conf/$bridgeName/proxy_arp 
 fi; 
  
  
  
 ################### 
 # Local Variables 
 ################### 
 virtualMachineId=1 
 virtualInterfaceName=veth1

 ############### 
 # Main Code 
 ############### 
  
 # starting the virtual machine 
 echo "Starting the machine $virtualMachineId" 
 vzctl start $virtualMachineId 
  
 #configuring the interface 
 echo "Configuring the interface" 
 ifconfig $virtualInterfaceName 0 
 echo /proc/sys/net/ipv4/conf/$virtualInterfaceName/forwarding 
 echo 1 > /proc/sys/net/ipv4/conf/$virtualInterfaceName/forwarding 
 echo 1 > /proc/sys/net/ipv4/conf/$virtualInterfaceName/proxy_arp 
 echo 1 > /proc/sys/net/ipv4/conf/eth0/forwarding 
 echo 1 > /proc/sys/net/ipv4/conf/eth0/proxy_arp 
  
  
 #removes the routes, if any, for the old interface 
 hasRoute=$(route -n|grep $virtualInterfaceName|grep $networkRootAdd$virtualMachineId | wc -l) 
  
 if [ $hasRoute -gt 0 ]; then 
   echo "removing route" 
   route del -host $networkRootAdd$virtualMachineId dev $virtualInterfaceName 
 fi; 
  
 # adding the interface to the bridge 
 echo "adding the interface to the bridge" 
 brctl addif $bridgeName $virtualInterfaceName 
  
 # adding the route to the bridge 
 echo "Creating route to the machine through the bridge" 
 route add $networkRootAdd$virtualMachineId dev $bridgeName 
  
  
 ################### 
 # Local Variables 
 ################### 
 virtualMachineId=2 
 virtualInterfaceName=veth2

 ############### 
 # Main Code 
 ############### 
  
 # starting the virtual machine 
 echo "Starting the machine $virtualMachineId" 
 vzctl start $virtualMachineId 
  
 #configuring the interface 
 echo "Configuring the interface" 
 ifconfig $virtualInterfaceName 0 
 echo /proc/sys/net/ipv4/conf/$virtualInterfaceName/forwarding 
 echo 1 > /proc/sys/net/ipv4/conf/$virtualInterfaceName/forwarding 
 echo 1 > /proc/sys/net/ipv4/conf/$virtualInterfaceName/proxy_arp 
 echo 1 > /proc/sys/net/ipv4/conf/eth0/forwarding 
 echo 1 > /proc/sys/net/ipv4/conf/eth0/proxy_arp 
  
  
 #removes the routes, if any, for the old interface 
 hasRoute=$(route -n|grep $virtualInterfaceName|grep $networkRootAdd$virtualMachineId | wc -l) 
  
 if [ $hasRoute -gt 0 ]; then 
   echo "removing route" 
   route del -host $networkRootAdd$virtualMachineId dev $virtualInterfaceName 
 fi; 
  
 # adding the interface to the bridge 
 echo "adding the interface to the bridge" 
 brctl addif $bridgeName $virtualInterfaceName 
  
 # adding the route to the bridge 
 echo "Creating route to the machine through the bridge" 
 route add $networkRootAdd$virtualMachineId dev $bridgeName 
  
  
 ################### 
 # Local Variables 
 ################### 
 virtualMachineId=3 
 virtualInterfaceName=veth3

 ############### 
 # Main Code 
 ############### 
  
 # starting the virtual machine 
 echo "Starting the machine $virtualMachineId" 
 vzctl start $virtualMachineId 
  
 #configuring the interface 
 echo "Configuring the interface" 
 ifconfig $virtualInterfaceName 0 
 echo /proc/sys/net/ipv4/conf/$virtualInterfaceName/forwarding 
 echo 1 > /proc/sys/net/ipv4/conf/$virtualInterfaceName/forwarding 
 echo 1 > /proc/sys/net/ipv4/conf/$virtualInterfaceName/proxy_arp 
 echo 1 > /proc/sys/net/ipv4/conf/eth0/forwarding 
 echo 1 > /proc/sys/net/ipv4/conf/eth0/proxy_arp 
  
  
 #removes the routes, if any, for the old interface 
 hasRoute=$(route -n|grep $virtualInterfaceName|grep $networkRootAdd$virtualMachineId | wc -l) 
  
 if [ $hasRoute -gt 0 ]; then 
   echo "removing route" 
   route del -host $networkRootAdd$virtualMachineId dev $virtualInterfaceName 
 fi; 
  
 # adding the interface to the bridge 
 echo "adding the interface to the bridge" 
 brctl addif $bridgeName $virtualInterfaceName 
  
 # adding the route to the bridge 
 echo "Creating route to the machine through the bridge" 
 route add $networkRootAdd$virtualMachineId dev $bridgeName 
  
 echo " "
 echo "Script started : " $beginScript 
 echo "Script ended   : " `date` 
 echo "---- End Startup ----" 
 
