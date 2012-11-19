#!/usr/bin/python
# -*- coding: utf-8 -*-

import sys
import os
import subprocess

from subprocess  import *
from netaddr import *
############################################################################################
g_file_config="/usr/local/src/PMIPv6_v0.4.1/pmipv6-daemon-umip-0.4/extras/example-mag2.conf"
############################################################################################


g_RFC5213FixedMAGLinkLocalAddressOnAllAccessLinks = IPAddress('0::0')
g_RFC5213FixedMAGLinkLayerAddressOnAllAccessLinks = " "
g_LmaAddress                                      = IPAddress('0::0')
g_MagAddressIngress                               = IPAddress('0::0')
g_MagAddressEgress                                = IPAddress('0::0')
g_MagDeviceIngress                                = " "
g_MagDeviceEgress                                 = " "


g_fhandle = open(g_file_config, 'r')
g_fcontent = g_fhandle.read()
g_fhandle.close()

lines = g_fcontent.splitlines()
for line in lines:
    line = line.rstrip().lstrip()
    line = line.rstrip(';')
    split = line.split(' ')
    element = split[-1]
    element = element.strip('"')
    if 'RFC5213FixedMAGLinkLocalAddressOnAllAccessLinks' in line:
        print line
        g_RFC5213FixedMAGLinkLocalAddressOnAllAccessLinks = IPAddress(element)
        
    elif 'RFC5213FixedMAGLinkLayerAddressOnAllAccessLinks' in line:
        print line
        g_RFC5213FixedMAGLinkLayerAddressOnAllAccessLinks = element
    elif 'LmaAddress' in line:
        print line
        g_LmaAddress = IPAddress(element)
    elif 'MagAddressIngress' in line:
        print line
        g_MagAddressIngress = IPAddress(element)
    elif 'MagAddressEgress' in line:
        print line
        g_MagAddressEgress = IPAddress(element)
    elif 'MagDeviceIngress' in line:
        print line
        g_MagDeviceIngress = element
    elif 'MagDeviceEgress' in line:
        print line
        g_MagDeviceEgress = element

command = "ifconfig " + g_MagDeviceIngress + " down"
print command
os.system(command)

command = "macchanger -m " +  g_RFC5213FixedMAGLinkLayerAddressOnAllAccessLinks + " " + g_MagDeviceIngress
print command
os.system(command)

command = "ifconfig " + g_MagDeviceIngress + " up"
print command
os.system(command)

command = "ip -6 addr del " + g_MagAddressEgress.format() + "/64 dev " + g_MagDeviceEgress
print command
os.system(command)

command = "ip -6 addr del " + g_MagAddressIngress.format() + "/64 dev " + g_MagDeviceIngress
print command
os.system(command)


for i in range (1 , 255):
    command = "ip -6 tunnel del ip6tnl" + str(i) + " >/dev/null 2>&1"
    os.system(command)

command = "rmmod ip6_tunnel"
print command
os.system(command)
command = "rmmod tunnel6"
print command
os.system(command)

command = "ip -6 addr add " + g_MagAddressEgress.format() + "/64 dev " + g_MagDeviceEgress
print command
os.system(command)

command = "ip -6 addr add " + g_MagAddressIngress.format() + "/64 dev " + g_MagDeviceIngress
print command
os.system(command)

command = "echo \"0\" > /proc/sys/net/ipv6/conf/all/accept_ra"
print command
os.system(command)
command = "echo \"0\" > /proc/sys/net/ipv6/conf/eth1/accept_ra"
print command
os.system(command)
command = "echo \"0\" > /proc/sys/net/ipv6/conf/eth0/accept_ra"
print command
os.system(command)
command = "echo \"1\" > /proc/sys/net/ipv6/conf/all/forwarding"
print command
os.system(command)

command = "ip -6 route add to default via " + g_LmaAddress.format() + " dev " + g_MagDeviceEgress
print command
os.system(command)

command = "modprobe ip6_tunnel"
print command
os.system(command)
command = "modprobe tunnel6"
print command
os.system(command)

command = "pkill -9 pmip6d"
print command
os.system(command)

command = '/usr/local/sbin/pmip6d -c ' + g_file_config
subprocess.call(command, shell=True)

