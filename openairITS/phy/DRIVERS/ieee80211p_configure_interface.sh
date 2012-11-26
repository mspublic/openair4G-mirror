# Interface configuration (interface type, MAC address, IP address, disable ARP)
iw phy phy4 interface add wlan0 type ibss 4addr off
ifconfig wlan0 hw ether 10:11:12:13:14:15
ifconfig wlan0 192.168.1.1 up -arp

# Static ARP table
arp -i wlan0 -s 192.168.1.2 10:21:22:23:24:25
arp -i wlan0 -s 192.168.1.255 FF:FF:FF:FF:FF:FF

