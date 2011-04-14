#!/bin/bash  

 beginScript=`date`
 
 ################### 
 # Global Variables 
 ################### 
 

 sharedDirectory=/tmp/simulation
 vzctl exec 1 openairinterface/macsim/openair_widens/ch_mac_sim 0 3 0

 vzctl exec 2 openairinterface/macsim/openair_widens/ue_mac_sim 8 1 1 8

 vzctl exec 3 openairinterface/macsim/openair_widens/ue_mac_sim 9 1 1 8
 
 echo " "
 echo "Script started : " $beginScript 
 echo "Script ended   : " `date` 
 echo "---- End Startup ----" 
 
