#!/bin/bash
ip -6 addr del 2001:100::2/64 dev eth0
ip -6 addr del 2001:1::1/64 dev eth1
ip -6 tunnel del ip6tnl1
ip -6 tunnel del ip6tnl2
rmmod ip6_tunnel
rmmod tunnel6

ip -6 addr add 2001:100::2/64 dev eth0
ip -6 addr add 2001:1::1/64 dev eth1

echo "0" > /proc/sys/net/ipv6/conf/all/accept_ra
echo "0" > /proc/sys/net/ipv6/conf/eth1/accept_ra
echo "0" > /proc/sys/net/ipv6/conf/eth0/accept_ra
echo "1" > /proc/sys/net/ipv6/conf/all/forwarding

ip -6 route add to default via 2001:100::1 dev eth0

modprobe ip6_tunnel
modprobe tunnel6

xhost +; export DISPLAY=:0.0; 
rm -f /usr/local/src/mipv6-daemon-umip-0.4/logs/mag12lma.pcap ; sync; wireshark -i eth0 -k -n -w  /usr/local/src/mipv6-daemon-umip-0.4/logs/mag12lma.pcap &
rm -f /usr/local/src/mipv6-daemon-umip-0.4/logs/mag12ap.pcap  ; sync; wireshark -i eth1 -k -n -w  /usr/local/src/mipv6-daemon-umip-0.4/logs/mag12ap.pcap  &

pmip6d -c /usr/local/src/mipv6-daemon-umip-0.4/extras/example-mag1.conf

