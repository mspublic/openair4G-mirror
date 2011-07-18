#!/bin/bash  

 beginScript=`date`
 
 ################### 
 # Global Variables 
 ################### 
 
# Distributed machines IPs
ipMachine[0]= 

# Copy files to the servers
scp genneratedTopologyFile0.tof  root@${ipMachine[0]}:/root/openplatform/genneratedTopologyFile.tof
scp interfCreationFile0.sh root@${ipMachine[0]}:/root/openplatform/interfCreationFile.sh
scp cleanFile0.sh root@${ipMachine[0]}:/root/openplatform/cleanFile.sh

# Run nas mesh structure
ssh  root@${ipMachine[0]}:/root/openplatform/interfCreationFile.sh
/root/openplatform/SIMULATION/USER_TOOLS/mac_sim 0
# Stop experiment and clean
if [ "$1" -eq "clean" ]
then
   ssh  root@${ipMachine[0]}:/root/openplatform/cleanFile.sh
fi

