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
#make clean
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
#make clean
make

# Module loading
sudo depmod -a
sudo modprobe mac80211_eurecom
sudo insmod ${OPENAIRITS_DIR}/phy/DRIVERS/ieee80211p.ko

# Interface configuration (interface type, MAC address, IP address, disable ARP)
sudo iw phy phy0 interface add wlan0 type ibss 4addr off
sudo ifconfig wlan0 hw ether 10:11:12:13:14:15
sudo ifconfig wlan0 192.168.1.1 up -arp

# Static ARP table
sudo arp -i wlan0 -s 192.168.1.2 10:21:22:23:24:25
sudo arp -i wlan0 -s 192.168.1.255 FF:FF:FF:FF:FF:FF

# Softmodem interface software test
gcc -W -Wall -o ./ieee80211p-softmodem ./ieee80211p-softmodem.c ./ieee80211p-netlinkapi.c
