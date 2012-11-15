#!bin/sh

## INSTRUCTIONS
## - check the coherence of the OPENAIRITS_DIR and MOD_DIR with YOUR platform
## - configure the module dependencies by running the following command:
##       sudo depmod -a 
## - install 'iw' by the following command:
##      sudo apt-get install iw
## - allow Ubuntu to reply to a PING in Broadcast by the following command:
##      echo 0 | sudo tee /proc/sys/net/ipv4/icmp_echo_ignore_broadcasts
## run the following shell
## voila !!

#Path configuration
MOD_DIR=/lib/modules/`uname -r`/kernel

#Modules compilation
cd ${OPENAIRITS_DIR}/mac/DOT11/
make clean
make MAC=1

sudo rm ${MOD_DIR}/compat/compat.ko
sudo rm ${MOD_DIR}/net/wireless/cfg80211.ko
sudo rm ${MOD_DIR}/net/mac80211/mac80211_eurecom.ko

sudo mkdir ${MOD_DIR}/compat
sudo mkdir ${MOD_DIR}/net/wireless
sudo mkdir ${MOD_DIR}/net/mac80211

sudo cp ${OPENAIRITS_DIR}/mac/DOT11/compat/compat.ko ${MOD_DIR}/compat/compat.ko
sudo cp ${OPENAIRITS_DIR}/mac/DOT11/net/wireless/cfg80211.ko ${MOD_DIR}/net/wireless/cfg80211.ko
sudo cp ${OPENAIRITS_DIR}/mac/DOT11/net/mac80211/mac80211_eurecom.ko ${MOD_DIR}/net/mac80211/mac80211_eurecom.ko

cd ${OPENAIRITS_DIR}/phy/DRIVERS/
make clean
make

