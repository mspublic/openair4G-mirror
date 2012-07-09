#!bin/sh

#Module loading
modprobe mac80211_eurecom
insmod /home/thales/openair4G/openairITS/openair1/DRIVERS/ieee80211p.o

#Interface configuration
iw dev wlan0 dev set type ibss
ifconfig wlan0 192.168.1.2 up -arp

#Static ARP table
arp -i wlan0 -s 192.168.1.2 10:11:12:13:14:15
arp -i wlan0 -s 192.168.1.255 FF:FF:FF:FF:FF:FF
