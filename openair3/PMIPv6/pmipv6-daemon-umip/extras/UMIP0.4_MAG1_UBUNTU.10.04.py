#!/usr/bin/python
# -*- coding: utf-8 -*-

import sys
import os
import subprocess
import getopt

from subprocess  import *
from netaddr import *

g_path = os.getcwd()
g_path = os.path.dirname(sys.argv[0]) + "/.."
g_path = os.path.abspath(g_path)

try:
    opts, args = getopt.getopt(sys.argv[1:], "prd", ["pcap=", "runversion=", "pmipdir="])
except getopt.GetoptError, err:
    # print help information and exit:
    print str(err) # will print something like "option -a not recognized"
    sys.exit(2)

g_pcap = "no"
g_run_version = "1"

for o,p in opts:
  if o in ('-p','--pcap'):
     g_pcap = p
  elif o in ['-r','--runversion']:
     g_run_version = str(p)
  elif o in ['-d','--pmipdir']:
     g_path = p

############################################################################################
g_file_config=g_path+"/extras/example-mag1.conf"
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
    elif 'LmaPmipNetworkAddress' in line:
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

# just to resolve, avoid ping6 since it is used for sync
command = "ssh -6 root@[" + g_LmaAddress.format() + "] \"ls /root\""
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

if g_pcap == "yes":
	command = "xhost +; export DISPLAY=:0.0; sync; wireshark -i eth0 -k -n -w  "+ g_path + "/logs/mag12lma."+g_run_version+".pcap &"
	value = os.system(command)
	print value

	command = "xhost +; export DISPLAY=:0.0; sync; wireshark -i eth1 -k -n -w  "+ g_path + "/logs/mag12ap."+g_run_version+".pcap  &"
	value = os.system(command)
	print value


command = '/usr/local/sbin/pmip6d -c ' + g_file_config
subprocess.call(command, shell=True)

