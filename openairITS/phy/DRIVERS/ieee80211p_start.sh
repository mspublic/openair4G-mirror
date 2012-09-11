#!bin/sh

#Modules compilation
OPENAIRITS_DIR=/home/training/OpenAir4G/openair4G/trunk/openairITS

cd ${OPENAIRITS_DIR}/mac/DOT11/
make clean
make MAC=1

rm /lib/modules/2.6.32.11+drm33.2.openairinterface.bigphys.rtai/updates/compat/compat.ko
rm /lib/modules/2.6.32.11+drm33.2.openairinterface.bigphys.rtai/updates/net/wireless/cfg80211.ko
rm /lib/modules/2.6.32.11+drm33.2.openairinterface.bigphys.rtai/updates/net/mac80211/mac80211_eurecom.ko

cp ${OPENAIRITS_DIR}/mac/DOT11/compat/compat.ko /lib/modules/2.6.32.11+drm33.2.openairinterface.bigphys.rtai/updates/compat/compat.ko
cp ${OPENAIRITS_DIR}/mac/DOT11/net/wireless/cfg80211.ko /lib/modules/2.6.32.11+drm33.2.openairinterface.bigphys.rtai/updates/net/wireless/cfg80211.ko
cp ${OPENAIRITS_DIR}/mac/DOT11/net/mac80211/mac80211_eurecom.ko /lib/modules/2.6.32.11+drm33.2.openairinterface.bigphys.rtai/updates/net/mac80211/mac80211_eurecom.ko

cd ${OPENAIRITS_DIR}/phy/DRIVERS/
make clean
make

# Module loading
modprobe mac80211_eurecom
insmod ${OPENAIRITS_DIR}/phy/DRIVERS/ieee80211p.ko

# Interface configuration (interface type, MAC address, IP address, disable ARP)
iw phy phy0 interface add wlan0 type ibss 4addr off
ifconfig wlan0 hw ether 10:11:12:13:14:15
ifconfig wlan0 192.168.1.1 up -arp

# Static ARP table
arp -i wlan0 -s 192.168.1.2 10:21:22:23:24:25
arp -i wlan0 -s 192.168.1.255 FF:FF:FF:FF:FF:FF

# Softmodem interface software test
gcc -W -Wall -o ./ieee80211p-softmodem ./ieee80211p-softmodem.c ./ieee80211p-netlinkapi.c
