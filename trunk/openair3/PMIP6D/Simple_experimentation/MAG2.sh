#!/bin/bash
./MAC_CHANGE.sh
ip -6 addr del 2001:100::3/64 dev eth1
ip -6 addr del 2001:1::2/64 dev eth0
ip -6 tunnel del ip6tnl1
ip -6 tunnel del ip6tnl2

ip -6 addr add 2001:100::3/64 dev eth1
ip -6 addr add 2001:1::2/64 dev eth0

echo "0" > /proc/sys/net/ipv6/conf/all/accept_ra
echo "1" > /proc/sys/net/ipv6/conf/all/forwarding

ip -6 route add to default via 2001:100::1 dev eth1

PMIP6D/pmip6d -m -i -L 2001:100::1 -N 2001:1::2 -E 2001:100::3
