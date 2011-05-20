#!/bin/bash
ip -6 route del 2001:1::1/64
ip -6 route del 2001:1::2/64
ip -6 addr del 2001:100::1/64 dev eth1
ip -6 tunnel del ip6tnl1
ip -6 tunnel del ip6tnl2

ip -6 addr add 2001:100::1/64 dev eth1
ip -6 addr add 2001:2::1/64 dev eth0
ip -6 route add 2001:1::1/64 via 2001:100::2 dev eth1
ip -6 route add 2001:1::2/64 via 2001:100::3 dev eth1

echo "0" > /proc/sys/net/ipv6/conf/all/accept_ra
echo "1" > /proc/sys/net/ipv6/conf/all/forwarding

PMIP6D/pmip6d -c -i -L 2001:100::1
