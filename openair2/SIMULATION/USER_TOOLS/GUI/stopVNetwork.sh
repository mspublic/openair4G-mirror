#!/bin/bash  

 beginScript=`date`
 
 ################### 
 # Global Variables 
 ################### 
 
 networkRootAdd=10.0.1
 bridgeName=vbr0 

 #Verify if the bridge is defined 
 bridgeDefined=$( brctl show | grep $bridgeName | wc -l) 
 if [ $bridgeDefined -gt 0 ]; then 
   echo "Removing bridge $bridgeName" 
   ifconfig $bridgeName down 
   brctl delbr $bridgeName 
 fi; 
  
  
 ################### 
 # Local Variables 
 ################### 
 virtualMachineId=1 
 virtualInterfaceName=veth1

 #removes the routes, if any, for the old interface 
 hasRoute=$(route -n|grep $virtualInterfaceName|grep $networkRootAdd$virtualMachineId | wc -l) 
  
 if [ $hasRoute -gt 0 ]; then 
    echo "removing route for $virtualMachineId machine" 
    route del -host $networkRootAdd$virtualMachineId dev $virtualInterfaceName 
 fi; 
 
 # Stopping the openVZ virtual machine 
 
 echo " Stopping virtual machine $virtualMachineId" 
 vzctl stop $virtualMachineId 
  
  
 ################### 
 # Local Variables 
 ################### 
 virtualMachineId=2 
 virtualInterfaceName=veth2

 #removes the routes, if any, for the old interface 
 hasRoute=$(route -n|grep $virtualInterfaceName|grep $networkRootAdd$virtualMachineId | wc -l) 
  
 if [ $hasRoute -gt 0 ]; then 
    echo "removing route for $virtualMachineId machine" 
    route del -host $networkRootAdd$virtualMachineId dev $virtualInterfaceName 
 fi; 
 
 # Stopping the openVZ virtual machine 
 
 echo " Stopping virtual machine $virtualMachineId" 
 vzctl stop $virtualMachineId 
  
  
 ################### 
 # Local Variables 
 ################### 
 virtualMachineId=3 
 virtualInterfaceName=veth3

 #removes the routes, if any, for the old interface 
 hasRoute=$(route -n|grep $virtualInterfaceName|grep $networkRootAdd$virtualMachineId | wc -l) 
  
 if [ $hasRoute -gt 0 ]; then 
    echo "removing route for $virtualMachineId machine" 
    route del -host $networkRootAdd$virtualMachineId dev $virtualInterfaceName 
 fi; 
 
 # Stopping the openVZ virtual machine 
 
 echo " Stopping virtual machine $virtualMachineId" 
 vzctl stop $virtualMachineId 
  
 echo " "
 echo "Script started : " $beginScript 
 echo "Script ended   : " `date` 
 echo "---- End Stopping machines ----" 
 
