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
 osTemplate=debian-4.0-i386-minimal
 sharedDirectory=/tmp/simulation


if [ -d $sharedDirectory ]; then 
 echo "$sharedDirectory exists" 
else 
 echo "Directory: '$sharedDirectory' does not exist!!" 
 mkdir -p $sharedDirectory 
 sleep 10 
 if [ -d $sharedDirectory ]; then 
  echo "Directory created successfuly" 
 else 
 echo "ERROR: Could not create '$sharedDirectory' directory" 
 fi 
fi 
 
  
 ################### 
 # Local Variables 
 ################### 
 virtualMachineId=1 
 virtualInterfaceName=veth1
 mac1=00:12:34:56:71:1B
 mac2=00:12:34:56:71:1A

 ############### 
 # Main Code 
 ############### 
  
 # Creating and configuring virtual machine 
 echo "Creating machine $virtualMachineId from a template"  
 vzctl create $virtualMachineId  $osTemplate 
  
 echo "Adding the machine name" 
 vzctl set $virtualMachineId --nameserver $nameServer --save 
  
 echo "Adding the interface" 
 vzctl set $virtualMachineId --netif_add $virtualMachineInterface,$mac1,$virtualInterfaceName,$mac2 --save 
  
 echo "Saving old interfaces config" 
 cp /var/lib/vz/private/$virtualMachineId/etc/network/interfaces /var/lib/vz/private/$virtualMachineId/etc/network/interfaces.orig 
  
 echo "Creating the new interface" 
 echo -e "auto $virtualMachineInterface lo\n iface lo inet loopback \n iface $virtualMachineInterface  inet static\n    address $networkRootAdd$virtualMachineId\n    netmask $netMask\n  up route add default gw $defaultGw\n" > /var/lib/vz/private/$virtualMachineId/etc/network/interfaces 
  
 # Configuring the virtual interface 
 echo "Starting the virtual machine" 
 vzctl start $virtualMachineId 
 sleep 3 
  
 echo "Configuring the interface" 
 ifconfig $virtualInterfaceName 0 
 echo /proc/sys/net/ipv4/conf/$virtualInterfaceName/forwarding 
 echo 1 > /proc/sys/net/ipv4/conf/$virtualInterfaceName/forwarding 
 echo 1 > /proc/sys/net/ipv4/conf/$virtualInterfaceName/proxy_arp 
 echo 1 > /proc/sys/net/ipv4/conf/eth0/forwarding 
 echo 1 > /proc/sys/net/ipv4/conf/eth0/proxy_arp 
 ip route add $networkRootAdd$virtualMachineId dev $virtualInterfaceName 
  
 echo "Stopping the machine" 
 vzctl stop $virtualMachineId 
  
  
  
 ################### 
 # Local Variables 
 ################### 
 virtualMachineId=2 
 virtualInterfaceName=veth2
 mac1=00:12:34:56:71:2B
 mac2=00:12:34:56:71:2A

 ############### 
 # Main Code 
 ############### 
  
 # Creating and configuring virtual machine 
 echo "Creating machine $virtualMachineId from a template"  
 vzctl create $virtualMachineId  $osTemplate 
  
 echo "Adding the machine name" 
 vzctl set $virtualMachineId --nameserver $nameServer --save 
  
 echo "Adding the interface" 
 vzctl set $virtualMachineId --netif_add $virtualMachineInterface,$mac1,$virtualInterfaceName,$mac2 --save 
  
 echo "Saving old interfaces config" 
 cp /var/lib/vz/private/$virtualMachineId/etc/network/interfaces /var/lib/vz/private/$virtualMachineId/etc/network/interfaces.orig 
  
 echo "Creating the new interface" 
 echo -e "auto $virtualMachineInterface lo\n iface lo inet loopback \n iface $virtualMachineInterface  inet static\n    address $networkRootAdd$virtualMachineId\n    netmask $netMask\n  up route add default gw $defaultGw\n" > /var/lib/vz/private/$virtualMachineId/etc/network/interfaces 
  
 # Configuring the virtual interface 
 echo "Starting the virtual machine" 
 vzctl start $virtualMachineId 
 sleep 3 
  
 echo "Configuring the interface" 
 ifconfig $virtualInterfaceName 0 
 echo /proc/sys/net/ipv4/conf/$virtualInterfaceName/forwarding 
 echo 1 > /proc/sys/net/ipv4/conf/$virtualInterfaceName/forwarding 
 echo 1 > /proc/sys/net/ipv4/conf/$virtualInterfaceName/proxy_arp 
 echo 1 > /proc/sys/net/ipv4/conf/eth0/forwarding 
 echo 1 > /proc/sys/net/ipv4/conf/eth0/proxy_arp 
 ip route add $networkRootAdd$virtualMachineId dev $virtualInterfaceName 
  
 echo "Stopping the machine" 
 vzctl stop $virtualMachineId 
  
  
  
 ################### 
 # Local Variables 
 ################### 
 virtualMachineId=3 
 virtualInterfaceName=veth3
 mac1=00:12:34:56:71:3B
 mac2=00:12:34:56:71:3A

 ############### 
 # Main Code 
 ############### 
  
 # Creating and configuring virtual machine 
 echo "Creating machine $virtualMachineId from a template"  
 vzctl create $virtualMachineId  $osTemplate 
  
 echo "Adding the machine name" 
 vzctl set $virtualMachineId --nameserver $nameServer --save 
  
 echo "Adding the interface" 
 vzctl set $virtualMachineId --netif_add $virtualMachineInterface,$mac1,$virtualInterfaceName,$mac2 --save 
  
 echo "Saving old interfaces config" 
 cp /var/lib/vz/private/$virtualMachineId/etc/network/interfaces /var/lib/vz/private/$virtualMachineId/etc/network/interfaces.orig 
  
 echo "Creating the new interface" 
 echo -e "auto $virtualMachineInterface lo\n iface lo inet loopback \n iface $virtualMachineInterface  inet static\n    address $networkRootAdd$virtualMachineId\n    netmask $netMask\n  up route add default gw $defaultGw\n" > /var/lib/vz/private/$virtualMachineId/etc/network/interfaces 
  
 # Configuring the virtual interface 
 echo "Starting the virtual machine" 
 vzctl start $virtualMachineId 
 sleep 3 
  
 echo "Configuring the interface" 
 ifconfig $virtualInterfaceName 0 
 echo /proc/sys/net/ipv4/conf/$virtualInterfaceName/forwarding 
 echo 1 > /proc/sys/net/ipv4/conf/$virtualInterfaceName/forwarding 
 echo 1 > /proc/sys/net/ipv4/conf/$virtualInterfaceName/proxy_arp 
 echo 1 > /proc/sys/net/ipv4/conf/eth0/forwarding 
 echo 1 > /proc/sys/net/ipv4/conf/eth0/proxy_arp 
 ip route add $networkRootAdd$virtualMachineId dev $virtualInterfaceName 
  
 echo "Stopping the machine" 
 vzctl stop $virtualMachineId 
  
  
 echo " "
 echo "Script started : " $beginScript 
 echo "Script ended   : " `date` 
 echo "---- End configuration ----" 
 
