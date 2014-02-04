#!/bin/bash

#############################################################
#Setting the EXTERNAL and INTERNAL interfaces for the network
#############################################################
declare EXTIF="eth0"
declare INTIF="eth3"

IPTABLES=/sbin/iptables
DEPMOD=/sbin/depmod
MODPROBE=/sbin/modprobe

###########################################################
THIS_SCRIPT_PATH=$(dirname $(readlink -f $0))
source $THIS_SCRIPT_PATH/utils.bash
###########################################################
test_command_install_package "vconfig"  "vlan" "--force-yes"
test_command_install_package "iptables" "iptables"
test_command_install_package "iperf"    "iperf"  "--force-yes"
test_command_install_package "ip"       "iproute"




echo "   External Interface:  $EXTIF"
echo "   Internal Interface:  $INTIF"

echo -en "   loading modules: "

# Need to verify that all modules have all required dependencies
#
echo "  - Verifying that all kernel modules are ok"
$DEPMOD -a

echo "----------------------------------------------------------------------"

#Load the main body of the IPTABLES module - "iptable"
#  - Loaded automatically when the "iptables" command is invoked
#
#  - Loaded manually to clean up kernel auto-loading timing issues
#
echo -en "ip_tables, "
$MODPROBE ip_tables


#Load the IPTABLES filtering module - "iptable_filter"
#  - Loaded automatically when filter policies are activated


#Load the stateful connection tracking framework - "ip_conntrack"
#
# The conntrack  module in itself does nothing without other specific
# conntrack modules being loaded afterwards such as the "ip_conntrack_ftp"
# module
#
#  - This module is loaded automatically when MASQ functionality is
#    enabled
#
#  - Loaded manually to clean up kernel auto-loading timing issues
#
echo -en "ip_conntrack, "
$MODPROBE ip_conntrack


#Load the FTP tracking mechanism for full FTP tracking
#
# Enabled by default -- insert a "#" on the next line to deactivate
#
echo -en "ip_conntrack_ftp, "
$MODPROBE ip_conntrack_ftp


#Load the IRC tracking mechanism for full IRC tracking
#
# Enabled by default -- insert a "#" on the next line to deactivate
#
echo -en "ip_conntrack_irc, "
$MODPROBE ip_conntrack_irc


#Load the general IPTABLES NAT code - "iptable_nat"
#  - Loaded automatically when MASQ functionality is turned on
#
#  - Loaded manually to clean up kernel auto-loading timing issues
#
echo -en "iptable_nat, "
$MODPROBE iptable_nat


#Loads the FTP NAT functionality into the core IPTABLES code
# Required to support non-PASV FTP.
#
# Enabled by default -- insert a "#" on the next line to deactivate
#
echo -en "ip_nat_ftp, "
$MODPROBE ip_nat_ftp

#Clearing any previous configuration
#
#  Unless specified, the defaults for INPUT and OUTPUT is ACCEPT
#    The default for FORWARD is DROP (REJECT is not a valid policy)
#
#   Isn't ACCEPT insecure?  To some degree, YES, but this is our testing
#   phase.  Once we know that IPMASQ is working well, I recommend you run
#   the rc.firewall-*-stronger rulesets which set the defaults to DROP but
#   also include the critical additional rulesets to still let you connect to
#   the IPMASQ server, etc.
#
echo "   Clearing any existing rules and setting default policy.."
bash_exec "iptables -P INPUT ACCEPT"
bash_exec "iptables -F INPUT"
bash_exec "iptables -P OUTPUT ACCEPT"
bash_exec "iptables -F OUTPUT"
bash_exec "iptables -P FORWARD ACCEPT"
bash_exec "iptables -F FORWARD"
bash_exec "iptables -t nat -F"
bash_exec "iptables -t mangle -F"
bash_exec "iptables -t filter -F"
bash_exec "iptables -t raw -F"

bash_exec "ip route flush cache"


echo "   Enabling forwarding"
bash_exec "sysctl -w net.ipv4.ip_forward=1"
assert "  `sysctl -n net.ipv4.ip_forward` -eq 1" $LINENO

# Dynamic IP users:
#
#   If you get your IP address dynamically from SLIP, PPP, or DHCP,
#   enable this following option.  This enables dynamic-address hacking
#   which makes the life with Diald and similar programs much easier.
#
echo "   Enabling DynamicAddr.."
bash_exec "sysctl -w net.ipv4.ip_dynaddr=1"
assert "  `sysctl -n net.ipv4.ip_dynaddr` -eq 1" $LINENO

bash_exec "sysctl -w net.ipv4.conf.all.log_martians=1"
assert "  `sysctl -n net.ipv4.conf.all.log_martians` -eq 1" $LINENO


echo "   Disabling reverse path filtering"
bash_exec "sysctl -w net.ipv4.conf.all.rp_filter=0"
assert "  `sysctl -n net.ipv4.conf.all.rp_filter` -eq 0" $LINENO
bash_exec "sysctl -w net.ipv4.conf.$EXTIF.rp_filter=0"
assert "  `sysctl -n net.ipv4.conf.$EXTIF.rp_filter` -eq 0" $LINENO
bash_exec "sysctl -w net.ipv4.conf.$INTIF.rp_filter=0"
assert "  `sysctl -n net.ipv4.conf.$INTIF.rp_filter` -eq 0" $LINENO


echo "0" > /proc/sys/net/ipv4/conf/all/proxy_arp
echo "1" > /proc/sys/net/ipv4/conf/$EXTIF/proxy_arp
echo "1" > /proc/sys/net/ipv4/conf/$INTIF/proxy_arp

echo "   FWD: Allow all connections OUT and only existing and related ones IN"
bash_exec "iptables -A FORWARD -i $EXTIF -o $INTIF -m state --state ESTABLISHED,RELATED -j ACCEPT"
bash_exec "iptables -A FORWARD -i $INTIF -o $EXTIF -m state --state NEW,ESTABLISHED,RELATED,INVALID -j ACCEPT"

bash_exec "modprobe 8021q"

for i in 5 6 7 8 9 10 11 12 13 14 15
do
    ifconfig $INTIF.$i down > /dev/null 2>&1
    sync
    vconfig rem $INTIF.$i > /dev/null 2>&1
    sync
done

for i in 5 6 7 8 9 10 11 12 13 14 15
do
    bash_exec "vconfig add $INTIF $i"
    sync
    NET=$(( $i + 200 ))
    CIDR='10.0.'$NET'.1/8'
    bash_exec "ip -4 addr add $CIDR dev $INTIF.$i"
    bash_exec "iptables -A FORWARD -i $EXTIF -o $INTIF.$i -m state --state ESTABLISHED,RELATED -j ACCEPT"
    bash_exec "iptables -A FORWARD -i $INTIF.$i -o $EXTIF -m state --state NEW,ESTABLISHED,RELATED,INVALID -j ACCEPT"
    bash_exec "echo 1 > /proc/sys/net/ipv4/conf/$INTIF.$i/proxy_arp"
    bash_exec "echo 0 > /proc/sys/net/ipv4/conf/$INTIF.$i/rp_filter"
done
for i in 5 6 7 8 9 10 11 12 13 14 15
do
    bash_exec "iptables  -t mangle -A PREROUTING -i $INTIF.$i -j CONNMARK --restore-mark"
    bash_exec "iptables  -t mangle -A PREROUTING -i $INTIF.$i -m mark --mark 0 -j MARK --set-mark $i"
    bash_exec "iptables  -t mangle -A PREROUTING -i $INTIF.$i -j CONNMARK --save-mark"


    fgrep  vlan$i /etc/iproute2/rt_tables
    if [ $? -ne 0 ]
    then
        base=200
        num=$(( $i + $base ))
        echo "$num vlan$i" >>  /etc/iproute2/rt_tables
        echo "Updating /etc/iproute2/rt_tables with table vlan$i id $num"
    fi
    ip rule del from all iif $EXTIF  fwmark $i table vlan$i > /dev/null
    bash_exec "ip rule add iif $EXTIF fwmark $i table vlan$i"
    bash_exec "ip route add default dev $INTIF.$i table vlan$i"
done
#bash_exec "iptables  -t mangle -A OUTPUT -m mark ! --mark 0 -j CONNMARK --save-mark"
#iptables -I INPUT      -i $INTIF.5 -j LOG --log-ip-options --log-prefix "INPUT CHAIN:"
#iptables -I FORWARD  -t mangle  -i $INTIF.5 -j LOG --log-ip-options --log-prefix "FORWARD CHAIN(mangle):"
#iptables -I FORWARD  -t filter  -i $INTIF.5 -j LOG --log-ip-options --log-prefix "FORWARD CHAIN(filter):"
#iptables -I PREROUTING -t raw -i $INTIF.5 -j LOG --log-ip-options --log-prefix "PREROUTING (raw):"
#iptables -I PREROUTING -t mangle -i $INTIF.5 -j LOG --log-ip-options --log-prefix "PREROUTING (mangle):"
#iptables -I PREROUTING -t nat -i $INTIF.5 -j LOG --log-ip-options --log-prefix "PREROUTING (nat):"


echo "   Enabling SNAT (MASQUERADE) functionality on $EXTIF"
bash_exec "iptables -t nat -A POSTROUTING -o $EXTIF -j MASQUERADE"

echo -e "\nrc.firewall-iptables v$FWVER done.\n"
iptables  -t mangle -nvL
iptables  -t filter -nvL
