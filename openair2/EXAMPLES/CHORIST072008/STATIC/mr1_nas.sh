#Phil 25 July 08

source params.sh
sysctl -w net.ipv6.conf.all.forwarding=1
$OPENAIR3_SCRIPTS_PATH/mr1_conf_nas.sh 
$OPENAIR3_SCRIPTS_PATH/mr1_mpls_nas.sh

sysctl -w net.ipv6.conf.eth0.proxy_ndp=1
sysctl -w net.ipv6.conf.nasmesh0.proxy_ndp=1

ifconfig eth0 promisc
ip -6 addr add $MR1_EG_ADDR/64 dev eth0
$OPENAIR3_PMIP6D_PATH/pmip6d -m -s -L $CH_ADDR -N $MR1_EG_ADDR -E $MR1_IN_ADDR 
$OPENAIR3_SCRIPTS_PATH/mr1_del_mpls.sh

