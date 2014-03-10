cidr2mask() {
  local i mask=""
  local full_octets=$(($1/8))
  local partial_octet=$(($1%8))

  for ((i=0;i<4;i+=1)); do
    if [ $i -lt $full_octets ]; then
      mask+=255
    elif [ $i -eq $full_octets ]; then
      mask+=$((256 - 2**(8-$partial_octet)))
    else
      mask+=0
    fi
    test $i -lt 3 && mask+=.
  done

  echo $mask
}


black='\E[30m'
red='\E[31m'
green='\E[32m'
yellow='\E[33m'
blue='\E[34m'
magenta='\E[35m'
cyan='\E[36m'
white='\E[37m'
reset_color='\E[00m'

ROOT_UID=0
E_NOTROOT=67

HOSTNAME=$(hostname -f)
IPTABLES=`which iptables`

trim ()
{
    echo "$1" | sed -n '1h;1!H;${;g;s/^[ \t]*//g;s/[ \t]*$//g;p;}'
}

trim2()
{
    local var=$@
    var="${var#"${var%%[![:space:]]*}"}"   # remove leading whitespace characters
    var="${var%"${var##*[![:space:]]}"}"   # remove trailing whitespace characters
    echo -n "$var"
}

cecho()   # Color-echo
# arg1 = message
# arg2 = color
{
    local default_msg="No Message."
    message=${1:-$default_msg}
    color=${2:-$green}
    echo -e -n "$color$message$reset_color"
    echo
    return
}

echo_error() {
    local my_string=""
    until [ -z "$1" ]
    do
        my_string="$my_string$1"
        shift
        done
        cecho "$my_string" $red
}

echo_fatal() {
    local my_string=""
    until [ -z "$1" ]
    do
        my_string="$my_string$1"
        shift
        done
        echo_error "$my_string"
    exit -1
}

echo_warning() {
    local my_string=""
    until [ -z "$1" ]
    do
        my_string="$my_string$1"
        shift
    done
    cecho "$my_string" $yellow
}

echo_success() {
    local my_string=""
    until [ -z "$1" ]
    do
        my_string="$my_string$1"
        shift
    done
    cecho "$my_string" $green
}

bash_exec() {
    output=$($1 2>&1)
    result=$?
    if [ $result -eq 0 ]
    then
        echo_success "$1"
    else
        echo_error "$1: $output"
    fi
}

extract() {
    if [ -f $1 ] ; then
        case $1 in
            *.tar.bz2)    tar xvjf $1        ;;
            *.tar.gz)     tar xvzf $1        ;;
            *.bz2)        bunzip2  $1        ;;
            *.rar)        unrar    $1        ;;
            *.gz)         gunzip   $1        ;;
            *.tar)        tar xvf  $1        ;;
            *.tbz2)       tar xvjf $1        ;;
            *.tgz)        tar xvzf $1        ;;
            *.zip)        unzip    $1        ;;
            *.Z)          uncompress $1      ;;
            *.7z)         7z x     $1        ;;
            *)            echo_error "'$1' cannot be extracted via >extract<" ; return 1;;
        esac
    else
        echo_error "'$1' is not a valid file"
        return 1
    fi
    return 0
}

rotate_log_file () {
    if [ -f $1 ]; then
        TIMESTAMP=`date +%Y-%m-%d.%Hh_%Mm_%Ss`
        NEWLOGFILE=$1.$TIMESTAMP
        mv $1 $NEWLOGFILE
        cat /dev/null > $1
        sync
        nohup gzip -f -9 $NEWLOGFILE &
    fi
}

set_openair() {
    path=`pwd`
    declare -i length_path
    declare -i index
    length_path=${#path}

    for i in 'openair1' 'openair2' 'openair3' 'openair-cn' 'targets'
    do
        index=`echo $path | grep -b -o $i | cut -d: -f1`
        #echo ${path%$token*}
        if [[ $index -lt $length_path  && index -gt 0 ]]
           then
               declare -x OPENAIR_DIR
               index=`expr $index - 1`
               openair_path=`echo $path | cut -c1-$index`
               #openair_path=`echo ${path:0:$index}`
               export OPENAIR_DIR=$openair_path
               export OPENAIR_HOME=$openair_path
               export OPENAIR1_DIR=$openair_path/openair1
               export OPENAIR2_DIR=$openair_path/openair2
               export OPENAIR3_DIR=$openair_path/openair3
               export OPENAIRCN_DIR=$openair_path/openair-cn
               export OPENAIR_TARGETS=$openair_path/targets
               return 0
           fi
    done
    return -1
}

wait_process_started () {
    if  [ -z "$1" ]
    then
        echo_error "WAITING FOR PROCESS START: NO PROCESS"
        return 1
    fi
    ps -C $1 > /dev/null 2>&1
    while [ $? -ne 0 ]; do
        echo_warning "WAITING FOR $1 START"
        sleep 2
        ps -C $1 > /dev/null 2>&1
    done
    echo_success "PROCESS $1 STARTED"
    return 0
}

is_process_started () {
    if  [ -z "$1" ]
    then
        echo_error "WAITING FOR PROCESS START: NO PROCESS"
        return 1
    fi
    ps -C $1 > /dev/null 2>&1
    if [ $? -ne 0 ]
    then
        echo_success "PROCESS $1 NOT STARTED"
        return 1
    fi
    echo_success "PROCESS $1 STARTED"
    return 0
}

assert() {
    # If condition false
    # exit from script with error message
    E_PARAM_ERR=98
    E_PARAM_FAILED=99

    if [ -z "$2" ] # Not enought parameters passed.
    then
        return $E_PARAM_ERR
    fi

    lineno=$2
    if [ ! $1 ]
    then
        echo_error "Assertion failed:  \"$1\""
        echo_fatal "File \"$0\", line $lineno"
    fi
}

test_command_install_lib() {
  # usage: test_command_install_package searched_binary package_to_be_installed_if_binary_not_found optional_option_to_apt_get_install
  if [ ! -f $1 ]; then
      echo_warning "$2 seems to be not installed, trying..."
      apt-get install $3 $2 -y
      if [ ! -f $1 ]; then
          echo_fatal "$1 unavailable"
      fi
  fi
  echo_success "$1 available"
}


test_command_install_package() {
  # usage: test_command_install_package searched_binary package_to_be_installed_if_binary_not_found optional_option_to_apt_get_install
  if [ $# -eq 2 ]; then
      command -v $1 >/dev/null 2>&1 || { echo_warning "Program $1 is not installed. Trying installing it." >&2; apt-get install $2 -y; command -v $1 >/dev/null 2>&1 || { echo_fatal "Program $1 is not installed. Aborting." >&2;};}
  else
      if [ $# -eq 3 ]; then
          command -v $1 >/dev/null 2>&1 || { echo_warning "Program $1 is not installed. Trying installing it (apt-get install $3 $2)." >&2; apt-get install $3 $2 -y; command -v $1 >/dev/null 2>&1 || { echo_fatal "Program $1 is not installed. Aborting." >&2; };}
      else
          echo_fatal "test_command_install_package: BAD PARAMETER"
      fi
  fi
  echo_success "$1 available"
}

test_command_install_script() {
  # usage: test_command_install_script searched_binary script_to_be_invoked_if_binary_not_found
  command -v $1 >/dev/null 2>&1 || { echo_warning "Program $1 is not installed. Trying installing it." >&2; bash $2; command -v $1 >/dev/null 2>&1 || { echo_fatal "Program $1 is not installed. Aborting." >&2; };}
  echo_success "$1 available"
}

start_openswitch_daemon() {
  rmmod -s bridge
  if [[ -e "/lib/modules/`uname -r`/extra/openvswitch.ko" ]] ; then
      bash_exec "insmod /lib/modules/`uname -r`/extra/openvswitch.ko" > /dev/null 2>&1
  else
      echo_fatal "/lib/modules/`uname -r`/extra/openvswitch.ko not found, exiting"
  fi
  is_process_started "ovsdb-server"
  if [ $? -ne 0 ]
  then
      ovsdb-server --remote=punix:/usr/local/var/run/openvswitch/db.sock --remote=db:Open_vSwitch,manager_options --pidfile --detach
      wait_process_started "ovsdb-server"
  fi
  # To be done after installation
  # ovs-vsctl    --no-wait init
  is_process_started "ovs-vswitchd"
  if [ $? -ne 0 ]
  then
      ovs-vswitchd --pidfile --detach
      wait_process_started "ovs-vswitchd"
  fi
}

stop_openswitch_daemon() {
    pkill ovs-vswitchd
    pkill ovsdb-server
    sync
    if  ! is_process_started ovs-vswitchd  ; then
        pkill -9 ovs-vswitchd
    fi
    if ! is_process_started ovsdb-server ; then
        pkill -9 ovsdb-server
    fi
    rmmod -f openvswitch
}

check_for_epc_executable() {
    if [ ! -f $OPENAIR3_DIR/OPENAIRMME/objs/OAI_EPC/oai_epc ]
        then
        echo_error "Cannot find oai_epc executable object in directory $OPENAIR3_DIR/OPENAIRMME/objs/OAI_EPC/"
        echo_fatal "Please make sure you have compiled OAI EPC with --enable-standalone-epc option"
        fi
}

check_for_sgw_executable() {
    if [ ! -f $OPENAIR3_DIR/OPENAIRMME/objs/OAI_SGW/oai_sgw ]
    then
        echo_error "Cannot find oai_sgw executable object in directory $OPENAIR3_DIR/OPENAIRMME/objs/OAI_SGW/"
        echo_fatal "Please make sure you have compiled OAI EPC without --enable-standalone-epc option"
    fi
}

check_for_mme_executable() {
    if [ ! -f $OPENAIR3_DIR/OPENAIRMME/objs/OAISIM_MME/oaisim_mme ]
    then
        echo_error "Cannot find oai_sgw executable object in directory $OPENAIR3_DIR/OPENAIRMME/objs/OAISIM_MME/"
        echo_fatal "Please make sure you have compiled OAI EPC without --enable-standalone-epc option"
    fi
}

check_for_root_rights() {
    if [[ $EUID -ne 0 ]]; then
        echo_fatal "This script must be run as root" 1>&2
    fi
}

is_openvswitch_interface() {
   if [ "a$1" == "a" ]; then
       return 0
   fi
   if [ "a${1:0:3}" == "aeth" ]; then
       return 0;
   else 
       if [ "a${1:0:4}" == "awlan" ]; then
           return 0;
       else
           if [ "a${1:0:4}" == "awifi" ]; then
               return 0;
           else
               if [ "a${1:0:4}" == "anone" ]; then
                   return 0;
               fi
           fi
       fi
   fi
   return 1;
}

is_real_interface() {
   if [ "a$1" == "a" ]; then
       return 0
   fi
   IF=`cat /etc/udev/rules.d/70-persistent-net.rules | grep $1 | sed 's/^.*NAME=//' | tr -d '"'`
   if [ "$IF" == "$1" ]; then
       if [ "a${1:0:3}" == "aeth" ]; then
           return 1;
       else 
           if [ "a${1:0:4}" == "awlan" ]; then
               return 1;
           else
               if [ "a${1:0:4}" == "awifi" ]; then
                   return 1;
               fi
           fi
       fi
   fi
   return 0
}

is_vlan_interface() {
   if [ "a$1" == "a" ]; then
       return 0
   fi
   if [[ $1 == *.* ]]
   then
       interface_name=`echo $1 | cut -f1 -d '.'`
       vlan=`echo $1 | cut -f2 -d '.'`
       IF=`cat /etc/udev/rules.d/70-persistent-net.rules | grep $interface_name | sed 's/^.*NAME=//' | tr -d '"'`
       if [ "$IF" == "$interface_name" ]; then
           if [ "a${interface_name:0:3}" == "aeth" ]; then
               return 1;
           else 
               if [ "a${interface_name:0:4}" == "awlan" ]; then
                   return 1;
               else
                   if [ "a${interface_name:0:4}" == "awifi" ]; then
                       return 1;
                   fi
               fi
           fi
       fi
   fi
   return 0
}


delete_openvswitch_interface() {
  is_openvswitch_interface $1 
  if [ $? -eq 1 ]; then
      ifconfig $1 down  > /dev/null 2>&1
      tunctl -d $1      > /dev/null 2>&1
  fi
}

create_openvswitch_interface() {
  is_openvswitch_interface $1
  if [ $? -eq 1 ]; then
      bash_exec "tunctl -t $1"
  fi
}

build_enb_vlan_network() {
    # create vlan interface
    is_vlan_interface $ENB_INTERFACE_NAME_FOR_S1_MME
    if [ $? -eq 1 ]; then
        interface_name=`echo $ENB_INTERFACE_NAME_FOR_S1_MME | cut -f1 -d '.'`
        vlan=`echo $ENB_INTERFACE_NAME_FOR_S1_MME | cut -f2 -d '.'`
        ifconfig    $ENB_INTERFACE_NAME_FOR_S1_MME down > /dev/null 2>&1
        vconfig rem $ENB_INTERFACE_NAME_FOR_S1_MME      > /dev/null 2>&1
        sync
        bash_exec "vconfig add $interface_name $vlan"
        sync
        bash_exec "ifconfig  $ENB_INTERFACE_NAME_FOR_S1_MME up"
        sync
        bash_exec "ip -4 addr add  $ENB_IPV4_ADDRESS_FOR_S1_MME/$ENB_IPV4_NETMASK_FOR_S1_MME dev $ENB_INTERFACE_NAME_FOR_S1_MME"
    else
        echo_fatal "BAD INTERFACE NAME FOR ENB S1-MME $ENB_INTERFACE_NAME_FOR_S1_MME"' (waiting for ethx.y, wlanx.y or wifix.y)'
    fi;
    
    is_vlan_interface $ENB_INTERFACE_NAME_FOR_S1U
    if [ $? -eq 1 ]; then
        interface_name=`echo $ENB_INTERFACE_NAME_FOR_S1U | cut -f1 -d '.'`
        vlan=`echo $ENB_INTERFACE_NAME_FOR_S1U | cut -f2 -d '.'`
        ifconfig    $ENB_INTERFACE_NAME_FOR_S1U down > /dev/null 2>&1
        vconfig rem $ENB_INTERFACE_NAME_FOR_S1U      > /dev/null 2>&1
        sync
        bash_exec "vconfig add $interface_name $vlan"
        sync
        bash_exec "ifconfig $ENB_INTERFACE_NAME_FOR_S1U up"
        sync
        bash_exec "ip -4 addr add  $ENB_IPV4_ADDRESS_FOR_S1U/$ENB_IPV4_NETMASK_FOR_S1U dev $ENB_INTERFACE_NAME_FOR_S1U"
        sync
    else
        echo_fatal "BAD INTERFACE NAME FOR ENB S1U $ENB_INTERFACE_NAME_FOR_S1U"' (waiting for ethx.y, wlanx.y or wifix.y)'
    fi;
}

clean_enb_vlan_network() {
    is_vlan_interface $ENB_INTERFACE_NAME_FOR_S1_MME
    if [ $? -eq 1 ]; then
        ifconfig    $ENB_INTERFACE_NAME_FOR_S1_MME down > /dev/null 2>&1
        vconfig rem $ENB_INTERFACE_NAME_FOR_S1_MME      > /dev/null 2>&1
    fi;
    
    is_vlan_interface $ENB_INTERFACE_NAME_FOR_S1U
    if [ $? -eq 1 ]; then
        ifconfig    $ENB_INTERFACE_NAME_FOR_S1U down > /dev/null 2>&1
        vconfig rem $ENB_INTERFACE_NAME_FOR_S1U > /dev/null 2>&1
    fi;
    sync;
}

test_enb_vlan_network() {
    # TEST INTERFACES
    #ping -q -c 1 $MME_IPV4_ADDRESS_FOR_S1_MME > /dev/null 2>&1
    #if [ $? -ne 0 ]; then echo_fatal "PING MME S1_MME ERROR, ADDRESS IS $MME_IPV4_ADDRESS_FOR_S1_MME"; fi;
    #ping -q -c 1 $SGW_IPV4_ADDRESS_FOR_S1U_S12_S4_UP > /dev/null 2>&1
    #if [ $? -ne 0 ]; then echo_fatal "PING SGW S1U ERROR, ADDRESS IS $SGW_IPV4_ADDRESS_FOR_S1U_S12_S4_UP"; fi;
    return 0
}


build_mme_spgw_vlan_network() {
    #                                                                           hss.eur
    #                                                                             |
    #        +-----------+                                +-----------+           v   +----------+
    #        |  eNB      +------+            VLAN 1+------+    MME    +----+      +---+   HSS    |
    #        |           |ethx.1+------------------+ethy.1|           |    +------+   |          |
    #        |           +------+                  +------+           +----+      +---+          |
    #        |           |ethx.2+-------+                 |           |               +----------+
    #        |           +------+       |                 +-+-------+-+
    #        |           |              |                   | s11mme|    
    #        |           |              |                   +---+---+    
    #        |           |              |             (optional)|   VLAN 3
    #        +-----------+              |                   +---+---+    
    #                                   |                   | s11sgw|            router.eur
    #                                   |                 +-+-------+-+              |   +--------------+
    #                                   |                 |  S+P-GW   |              v   |   ROUTER     |
    #                                   |  VLAN2   +------+           +-------+     +----+              +----+
    #                                   +----------+ethy.2|           |sgi    +-...-+    |              |    +---...Internet
    #                                              +------+           +-------+     +----+              +----+
    #                                                     |           |      11 VLANS    |              |
    #                                                     +-----------+   ids=[5..15]    +--------------+
    cat $OPENAIRCN_DIR/$OBJ_DIR/Makefile | grep CFLAGS\ \=\  | grep DENABLE_USE_NETFILTER_FOR_SGI
    if [ $? -ne 0 ]
    then
        export ENABLE_USE_NETFILTER_FOR_SGI=0
    else
        export ENABLE_USE_NETFILTER_FOR_SGI=1
    fi

    cat $OPENAIRCN_DIR/$OBJ_DIR/Makefile | grep CFLAGS\ \=\  | grep DENABLE_USE_RAW_FOR_SGI
    if [ $? -ne 0 ]
    then
        export ENABLE_USE_RAW_FOR_SGI=0
    else
        export ENABLE_USE_RAW_FOR_SGI=1
    fi
    
    bash_exec "modprobe 8021q"
    
    # create vlan interface
    is_vlan_interface $MME_INTERFACE_NAME_FOR_S1_MME
    if [ $? -eq 1 ]; then
        interface_name=`echo $MME_INTERFACE_NAME_FOR_S1_MME | cut -f1 -d '.'`
        vlan=`echo $MME_INTERFACE_NAME_FOR_S1_MME | cut -f2 -d '.'`
        ifconfig    $MME_INTERFACE_NAME_FOR_S1_MME down > /dev/null 2>&1
        vconfig rem $MME_INTERFACE_NAME_FOR_S1_MME      > /dev/null 2>&1
        sync
        bash_exec "vconfig add $interface_name $vlan"
        sync
        bash_exec "ifconfig  $MME_INTERFACE_NAME_FOR_S1_MME up"
        sync
        bash_exec "ip -4 addr add  $MME_IPV4_ADDRESS_FOR_S1_MME/$MME_IPV4_NETMASK_FOR_S1_MME dev $MME_INTERFACE_NAME_FOR_S1_MME"
    else
        echo_fatal "BAD INTERFACE NAME FOR SGW S1-MME $MME_INTERFACE_NAME_FOR_S1_MME"' (waiting for ethx.y, wlanx.y or wifix.y)'
    fi;
    
    is_vlan_interface $SGW_INTERFACE_NAME_FOR_S1U_S12_S4_UP
    if [ $? -eq 1 ]; then
        interface_name=`echo $SGW_INTERFACE_NAME_FOR_S1U_S12_S4_UP | cut -f1 -d '.'`
        vlan=`echo $SGW_INTERFACE_NAME_FOR_S1U_S12_S4_UP | cut -f2 -d '.'`
        ifconfig    $SGW_INTERFACE_NAME_FOR_S1U_S12_S4_UP down > /dev/null 2>&1
        vconfig rem $SGW_INTERFACE_NAME_FOR_S1U_S12_S4_UP      > /dev/null 2>&1
        sync
        bash_exec "vconfig add $interface_name $vlan"
        sync
        bash_exec "ifconfig  $SGW_INTERFACE_NAME_FOR_S1U_S12_S4_UP up"
        sync
        bash_exec "ip -4 addr add  $SGW_IPV4_ADDRESS_FOR_S1U_S12_S4_UP/$SGW_IPV4_NETMASK_FOR_S1U_S12_S4_UP dev $SGW_INTERFACE_NAME_FOR_S1U_S12_S4_UP"
        sync
    else
        echo_fatal "BAD INTERFACE NAME FOR SGW S1U $SGW_INTERFACE_NAME_FOR_S1U_S12_S4_UP"' (waiting for ethx.y, wlanx.y or wifix.y)'
    fi;
    
    #
    is_real_interface $PGW_INTERFACE_NAME_FOR_SGI
    if [ $? -eq 1 ]; then
        ping -c 1 router.eur > /dev/null || { echo_fatal "router.eur does not respond to ping" >&2 ; }
        IP_ROUTER=`python -c 'import socket; print socket.gethostbyname("router.eur")'`
        export MAC_ROUTER=`ip neigh show | grep $IP_ROUTER | cut -d ' '  -f5 | tr -d ':'`
        echo_success "ROUTER MAC ADDRESS= $MAC_ROUTER"

        if [ $ENABLE_USE_NETFILTER_FOR_SGI -eq 1 ]; then

            bash_exec "modprobe nf_conntrack"
            bash_exec "modprobe nf_conntrack_ftp"

            ######################################################
            # PREROUTING
            ######################################################
            # We restore the mark following the CONNMARK mark. In fact, it does a simple MARK=CONNMARK
            # where MARK is the standard mark (usable by tc)
            # In French: Cette option de cible restaure le paquet marqué dans la marque de connexion
            # comme défini par CONNMARK. Un masque peut aussi être défini par l'option --mask.
            # Si une option mask est placée, seules les options masquées seront placées.
            # Notez que cette option de cible n'est valide que dans la table mangle.
            bash_exec "$IPTABLES -t mangle -A PREROUTING -j CONNMARK --restore-mark"

            # TEST bash_exec "$IPTABLES -t mangle -A PREROUTING -m mark --mark 0 -i $PGW_INTERFACE_NAME_FOR_SGI -j MARK --set-mark 15"
            # We set the mark of the initial packet as value of the conntrack mark for all the packets of the connection.
            # This mark will be restore for the other packets by the first rule of POSTROUTING --restore-mark).
            bash_exec "$IPTABLES -t mangle -A PREROUTING -j CONNMARK --save-mark"


            ######################################################
            # POSTROUTING
            ######################################################

            # MARK=CONNMARK
            bash_exec "iptables -A POSTROUTING -t mangle -o tap0 -j CONNMARK --restore-mark"
            # If we’ve got a mark no need to get further[
            bash_exec "iptables -A POSTROUTING -t mangle -o tap0 -m mark ! --mark 0 -j ACCEPT"

            #bash_exec "iptables -A POSTROUTING -p tcp --dport 21 -t mangle -j MARK --set-mark 1"
            #bash_exec "iptables -A POSTROUTING -p tcp --dport 80 -t mangle -j MARK --set-mark 2"

            # We set the mark of the initial packet as value of the conntrack mark for all the packets
            # of the connection. This mark will be restore for the other packets by the first rule
            # of POSTROUTING (–restore-mark).
            bash_exec "iptables -A POSTROUTING -t mangle -j CONNMARK --save-mark"

            bash_exec "iptables -A PREROUTING  -t mangle -j CONNMARK --restore-mark"

            # We restore the mark following the CONNMARK mark.
            # In fact, it does a simple MARK=CONNMARK where MARK is the standard mark (usable by tc)
            #bash_exec "$IPTABLES -A OUTPUT -t mangle -m mark ! --mark 0 -j CONNMARK --restore-mark"

            # If we’ve got a mark no need to get further[1]
            #TEST bash_exec "$IPTABLES -A OUTPUT -t mangle -p icmp -j MARK --set-mark 14"
            #bash_exec "$IPTABLES -A OUTPUT -t mangle -m mark ! --mark 0 -j ACCEPT"


            # We set the mark of the initial packet as value of the conntrack mark for all the packets of the connection.
            # This mark will be restore for the other packets by the first rule of OUTPUT (–restore-mark).
            #bash_exec "$IPTABLES -A OUTPUT -t mangle -j CONNMARK --save-mark"

            ######################################################
            # NETFILTER QUEUE
            ######################################################
            bash_exec "$IPTABLES -t mangle -A PREROUTING -i $PGW_INTERFACE_NAME_FOR_SGI -m connmark  --mark 5 -j NFQUEUE --queue-num 1"
            bash_exec "$IPTABLES -t mangle -A PREROUTING -i $PGW_INTERFACE_NAME_FOR_SGI -m connmark  --mark 6 -j NFQUEUE --queue-num 1"
            bash_exec "$IPTABLES -t mangle -A PREROUTING -i $PGW_INTERFACE_NAME_FOR_SGI -m connmark  --mark 7 -j NFQUEUE --queue-num 1"
            bash_exec "$IPTABLES -t mangle -A PREROUTING -i $PGW_INTERFACE_NAME_FOR_SGI -m connmark  --mark 8 -j NFQUEUE --queue-num 1"
            bash_exec "$IPTABLES -t mangle -A PREROUTING -i $PGW_INTERFACE_NAME_FOR_SGI -m connmark  --mark 9 -j NFQUEUE --queue-num 1"
            bash_exec "$IPTABLES -t mangle -A PREROUTING -i $PGW_INTERFACE_NAME_FOR_SGI -m connmark  --mark 10 -j NFQUEUE --queue-num 1"
            bash_exec "$IPTABLES -t mangle -A PREROUTING -i $PGW_INTERFACE_NAME_FOR_SGI -m connmark  --mark 11 -j NFQUEUE --queue-num 1"
            bash_exec "$IPTABLES -t mangle -A PREROUTING -i $PGW_INTERFACE_NAME_FOR_SGI -m connmark  --mark 12 -j NFQUEUE --queue-num 1"
            bash_exec "$IPTABLES -t mangle -A PREROUTING -i $PGW_INTERFACE_NAME_FOR_SGI -m connmark  --mark 13 -j NFQUEUE --queue-num 1"
            bash_exec "$IPTABLES -t mangle -A PREROUTING -i $PGW_INTERFACE_NAME_FOR_SGI -m connmark  --mark 14 -j NFQUEUE --queue-num 1"
            bash_exec "$IPTABLES -t mangle -A PREROUTING -i $PGW_INTERFACE_NAME_FOR_SGI -m connmark  --mark 15 -j NFQUEUE --queue-num 1"

            #echo 0 > /proc/sys/net/bridge/bridge-nf-call-iptables #To disable Iptables in the bridge.
            #Raw table: Some years ago appeared a new tables in Iptables.
            #This table can be used to avoid packets (connection really) to enter the NAT table:
            # iptables -t raw -I PREROUTING -i BRIDGE -s x.x.x.x -j NOTRACK.
            #bash_exec "$IPTABLES -t nat -A POSTROUTING -o $PGW_INTERFACE_NAME_FOR_SGI -j SNAT --to-source $PGW_IP_ADDR_FOR_SGI"
        else
            # # get ipv4 address from PGW_INTERFACE_NAME_FOR_SGI
            #IP_ADDR=`ifconfig $PGW_INTERFACE_NAME_FOR_SGI | awk '/inet addr/ {split ($2,A,":"); print A[2]}' | tr '\n' ' ' | sed -n '1h;1!H;${;g;s/^[ \t]*//g;s/[ \t]*$//g;p;}'`

            #NETWORK=`echo $IP_ADDR | cut -d . -f 1,2,3`

            bash_exec "modprobe 8021q"

            for i in 5 6 7 8 9 10 11 12 13 14 15
            do
                # create vlan interface
                ifconfig    $PGW_INTERFACE_NAME_FOR_SGI.$i down > /dev/null 2>&1
                vconfig rem $PGW_INTERFACE_NAME_FOR_SGI.$i > /dev/null 2>&1
                sync
                bash_exec "vconfig add $PGW_INTERFACE_NAME_FOR_SGI $i"
                sync
                bash_exec "ifconfig  $PGW_INTERFACE_NAME_FOR_SGI.$i up"
                sync
                # configure vlan interface
                #CIDR=$NETWORK'.'$i'/24'
                base=200
                NET=$(( $i + $base ))
                CIDR='10.0.'$NET'.2/8'
                bash_exec "ip -4 addr add  $CIDR dev $PGW_INTERFACE_NAME_FOR_SGI.$i"
            done
        fi

        bash_exec "ip link set $PGW_INTERFACE_NAME_FOR_SGI promisc on"
    else
        echo_warning "SGI interface disabled by config file"
    fi
}

clean_epc_vlan_network() {
    bash_exec "modprobe tun"
    bash_exec "modprobe ip_tables"
    bash_exec "modprobe iptable_nat"
    bash_exec "modprobe x_tables"

    bash_exec "$IPTABLES -P INPUT ACCEPT"
    bash_exec "$IPTABLES -F INPUT"
    bash_exec "$IPTABLES -P OUTPUT ACCEPT"
    bash_exec "$IPTABLES -F OUTPUT"
    bash_exec "$IPTABLES -P FORWARD ACCEPT"
    bash_exec "$IPTABLES -F FORWARD"
    bash_exec "$IPTABLES -t raw    -F"
    bash_exec "$IPTABLES -t nat    -F"
    bash_exec "$IPTABLES -t mangle -F"
    bash_exec "$IPTABLES -t filter -F"

    bash_exec "ip route flush cache"

    echo "   Disabling forwarding"
    bash_exec "sysctl -w net.ipv4.ip_forward=0"
    assert "  `sysctl -n net.ipv4.ip_forward` -eq 0" $LINENO

    echo "   Enabling DynamicAddr.."
    bash_exec "sysctl -w net.ipv4.ip_dynaddr=1"
    assert "  `sysctl -n net.ipv4.ip_dynaddr` -eq 1" $LINENO

    bash_exec "sysctl -w net.ipv4.conf.all.log_martians=1"
    assert "  `sysctl -n net.ipv4.conf.all.log_martians` -eq 1" $LINENO


    echo "   Disabling reverse path filtering"
    bash_exec "sysctl -w net.ipv4.conf.all.rp_filter=0"
    assert "  `sysctl -n net.ipv4.conf.all.rp_filter` -eq 0" $LINENO

    bash_exec "modprobe 8021q"

    ifconfig    $MME_INTERFACE_NAME_FOR_S1_MME down > /dev/null 2>&1
    vconfig rem $MME_INTERFACE_NAME_FOR_S1_MME      > /dev/null 2>&1

    ifconfig    $SGW_INTERFACE_NAME_FOR_S1U_S12_S4_UP down > /dev/null 2>&1
    vconfig rem $SGW_INTERFACE_NAME_FOR_S1U_S12_S4_UP      > /dev/null 2>&1


    for i in 5 6 7 8 9 10 11 12 13 14 15
    do
        # delete vlan interface
        ifconfig    $PGW_INTERFACE_NAME_FOR_SGI.$i down > /dev/null 2>&1
        vconfig rem $PGW_INTERFACE_NAME_FOR_SGI.$i      > /dev/null 2>&1
    done
    #ip link set $PGW_INTERFACE_NAME_FOR_SGI down > /dev/null 2>&1
}

build_openvswitch_network() {
    start_openswitch_daemon
    # REMINDER:
    #                                                                           hss.eur
    #                                                                             |
    #        +-----------+          +------+              +-----------+           v   +----------+
    #        |  eNB      +------+   |  ovs | VLAN 1+------+    MME    +----+      +---+   HSS    |
    #        |           |cpenb0+------------------+cpmme0|           |    +------+   |          |
    #        |           +------+   |bridge|       +------+           +----+      +---+          |
    #        |           |upenb0+-------+  |              |           |               +----------+
    #        |           +------+   |   |  |              +-+-------+-+
    #        |           |          |   |  +----------------| s11mme|---+
    #        |           |          |   |                   +---+---+   |
    #        |           |          |   |             (optional)|       |ovs bridge is optional
    #        +-----------+          |   |                   +---+---+   |
    #                               +---|------------------ | s11sgw|---+        router.eur
    #                                   |                 +-+-------+-+              |   +--------------+
    #                                   |                 |  S+P-GW   |              v   |   ROUTER     |
    #                                   |  VLAN2   +------+           +-------+     +----+              +----+
    #                                   +----------+upsgw0|           |sgi    +-...-+    |              |    +---...Internet
    #                                              +------+           +-------+     +----+              +----+
    #                                                     |           |      11 VLANS    |              |
    #                                                     +-----------+   ids=[5..15]    +--------------+
    #
    ##################################################
    # build bridge between eNB and MME/SPGW
    ##################################################
    create_openvswitch_interface $ENB_INTERFACE_NAME_FOR_S1_MME
    create_openvswitch_interface $ENB_INTERFACE_NAME_FOR_S1U
    create_openvswitch_interface $MME_INTERFACE_NAME_FOR_S1_MME
    create_openvswitch_interface $SGW_INTERFACE_NAME_FOR_S1U_S12_S4_UP
    create_openvswitch_interface $MME_INTERFACE_NAME_FOR_S11_MME
    create_openvswitch_interface $SGW_INTERFACE_NAME_FOR_S11
    
    bash_exec "ovs-vsctl add-br       $BRIDGE"
    bash_exec "ovs-vsctl add-port     $BRIDGE $ENB_INTERFACE_NAME_FOR_S1_MME        tag=1"
    bash_exec "ovs-vsctl add-port     $BRIDGE $MME_INTERFACE_NAME_FOR_S1_MME        tag=1"
    bash_exec "ovs-vsctl add-port     $BRIDGE $ENB_INTERFACE_NAME_FOR_S1U           tag=2"
    bash_exec "ovs-vsctl add-port     $BRIDGE $SGW_INTERFACE_NAME_FOR_S1U_S12_S4_UP tag=2"
    bash_exec "ovs-vsctl add-port     $BRIDGE $MME_INTERFACE_NAME_FOR_S11_MME       tag=3"
    bash_exec "ovs-vsctl add-port     $BRIDGE $SGW_INTERFACE_NAME_FOR_S11           tag=3"

    bash_exec "ifconfig $MME_INTERFACE_NAME_FOR_S1_MME promisc up"
    bash_exec "ifconfig $MME_INTERFACE_NAME_FOR_S1_MME $MME_IPV4_ADDRESS_FOR_S1_MME netmask `cidr2mask $MME_IPV4_NETMASK_FOR_S1_MME` promisc up"
    bash_exec "ifconfig $SGW_INTERFACE_NAME_FOR_S1U_S12_S4_UP promisc up"
    bash_exec "ifconfig $SGW_INTERFACE_NAME_FOR_S1U_S12_S4_UP $SGW_IPV4_ADDRESS_FOR_S1U_S12_S4_UP netmask `cidr2mask $SGW_IPV4_NETMASK_FOR_S1U_S12_S4_UP` promisc up"

    bash_exec "ifconfig $ENB_INTERFACE_NAME_FOR_S1_MME promisc up"
    bash_exec "ifconfig $ENB_INTERFACE_NAME_FOR_S1_MME $ENB_IPV4_ADDRESS_FOR_S1_MME netmask `cidr2mask $ENB_IPV4_NETMASK_FOR_S1_MME` promisc up"
    bash_exec "ifconfig $ENB_INTERFACE_NAME_FOR_S1U promisc up"
    bash_exec "ifconfig $ENB_INTERFACE_NAME_FOR_S1U $ENB_IPV4_ADDRESS_FOR_S1U netmask `cidr2mask $ENB_IPV4_NETMASK_FOR_S1U` promisc up"

    bash_exec "ifconfig $MME_INTERFACE_NAME_FOR_S11_MME promisc up"
    bash_exec "ifconfig $MME_INTERFACE_NAME_FOR_S11_MME $MME_IPV4_ADDRESS_FOR_S11_MME netmask `cidr2mask $MME_IPV4_NETMASK_FOR_S11_MME` promisc up"
    bash_exec "ifconfig $SGW_INTERFACE_NAME_FOR_S11 promisc up"
    bash_exec "ifconfig $SGW_INTERFACE_NAME_FOR_S11 $SGW_IPV4_ADDRESS_FOR_S11 netmask `cidr2mask $SGW_IPV4_NETMASK_FOR_S11` promisc up"
}

test_openvswitch_network() {

    # TEST INTERFACES
    ping -q -c 1 $MME_IPV4_ADDRESS_FOR_S1_MME > /dev/null 2>&1
    if [ $? -ne 0 ]; then echo_fatal "PING INTERFACE $MME_INTERFACE_NAME_FOR_S1_MME ERROR, ADDRESS IS $MME_IPV4_ADDRESS_FOR_S1_MME"; fi;
    ping -q -c 1 $SGW_IPV4_ADDRESS_FOR_S1U_S12_S4_UP > /dev/null 2>&1
    if [ $? -ne 0 ]; then echo_fatal "PING INTERFACE $SGW_INTERFACE_NAME_FOR_S1U_S12_S4_UP ERROR, ADDRESS IS $SGW_IPV4_ADDRESS_FOR_S1U_S12_S4_UP"; fi;
    ping -q -c 1 $ENB_IPV4_ADDRESS_FOR_S1_MME > /dev/null 2>&1
    if [ $? -ne 0 ]; then echo_fatal "PING INTERFACE $ENB_INTERFACE_NAME_FOR_S1_MME ADDR ERROR, ADDRESS IS $ENB_IPV4_ADDRESS_FOR_S1_MME"; fi;
    ping -q -c 1 $ENB_IPV4_ADDRESS_FOR_S1U > /dev/null 2>&1
    if [ $? -ne 0 ]; then echo_fatal "PING INTERFACE $ENB_INTERFACE_NAME_FOR_S1U ERROR, ADDRESS IS $ENB_IPV4_ADDRESS_FOR_S1U"; fi;
    ping -q -c 1 $MME_IPV4_ADDRESS_FOR_S11_MME > /dev/null 2>&1
    if [ $? -ne 0 ]; then echo_fatal "PING INTERFACE $MME_INTERFACE_NAME_FOR_S11_MME ERROR, ADDRESS IS $MME_IPV4_ADDRESS_FOR_S11_MME"; fi;
    ping -q -c 1 $SGW_IPV4_ADDRESS_FOR_S11 > /dev/null 2>&1
    if [ $? -ne 0 ]; then echo_fatal "PING INTERFACE $SGW_INTERFACE_NAME_FOR_S11 ERROR, ADDRESS IS $SGW_IPV4_ADDRESS_FOR_S11"; fi;
    
    
    ## TEST NETWORK BETWEEN ENB-MME-SP-GW
    iperf  --bind $MME_IPV4_ADDRESS_FOR_S1_MME -u -s 2>&1  > /dev/null &
    iperf  --bind $ENB_IPV4_ADDRESS_FOR_S1_MME -u --num 1K -c $MME_IPV4_ADDRESS_FOR_S1_MME 2>&1 | grep -i WARNING > /dev/null
    if [ $? -eq 0 ]; then
        pkill iperf 2>&1 > /dev/null
        echo_fatal 'NETWORK ERROR CONFIGURATION (openvswitch) between ENB and MME S1'
    else
        echo_success 'NETWORK TEST SUCCESS (openvswitch) between ENB and MME S1'

    fi
    pkill iperf 2>&1 > /dev/null

    iperf  --bind $SGW_IPV4_ADDRESS_FOR_S1U_S12_S4_UP -u -s 2>&1  > /dev/null &
    iperf  --bind $ENB_IPV4_ADDRESS_FOR_S1U -u --num 1K -c $SGW_IPV4_ADDRESS_FOR_S1U_S12_S4_UP 2>&1 | grep -i WARNING > /dev/null
    if [ $? -eq 0 ]; then
        pkill iperf 2>&1 > /dev/null
        echo_fatal 'NETWORK ERROR CONFIGURATION (openvswitch) between ENB and S-GW S1-U'
    else
        echo_success 'NETWORK TEST SUCCESS (openvswitch) between ENB and S-GW S1-U'
    fi
    pkill iperf 2>&1 > /dev/null

    iperf  --bind $SGW_IPV4_ADDRESS_FOR_S11 -u -s 2>&1  > /dev/null &
    iperf  --bind $MME_IPV4_ADDRESS_FOR_S11_MME -u --num 1K -c $SGW_IPV4_ADDRESS_FOR_S11 2>&1 | grep -i WARNING > /dev/null
    if [ $? -eq 0 ]; then
        pkill iperf 2>&1 > /dev/null
        echo_fatal 'NETWORK ERROR CONFIGURATION (openvswitch) between MME and S-GW S11'
    else
        echo_success 'NETWORK TEST SUCCESS (openvswitch) between MME and S-GW S11'
    fi
    pkill iperf 2>&1 > /dev/null
    return 0
}

clean_openvswitch_network() {
    ##################################################
    # del bridge between eNB and MME/SPGW
    ##################################################
    delete_openvswitch_interface $ENB_INTERFACE_NAME_FOR_S1_MME
    delete_openvswitch_interface $ENB_INTERFACE_NAME_FOR_S1U
    delete_openvswitch_interface $MME_INTERFACE_NAME_FOR_S1_MME
    delete_openvswitch_interface $SGW_INTERFACE_NAME_FOR_S1U_S12_S4_UP
    delete_openvswitch_interface $MME_INTERFACE_NAME_FOR_S11_MME
    delete_openvswitch_interface $SGW_INTERFACE_NAME_FOR_S11
    if  is_process_started ovs-vswitchd  ; then
        ovs-vsctl del-br $BRIDGE > /dev/null 2>&1
    fi
    stop_openswitch_daemon
}

build_epc_ovs_network() {

    cat $OPENAIRCN_DIR/$OBJ_DIR/Makefile | grep CFLAGS\ \=\  | grep DENABLE_USE_NETFILTER_FOR_SGI
    if [ $? -ne 0 ]
    then
        export ENABLE_USE_NETFILTER_FOR_SGI=0
    else
        export ENABLE_USE_NETFILTER_FOR_SGI=1
    fi

    cat $OPENAIRCN_DIR/$OBJ_DIR/Makefile | grep CFLAGS\ \=\  | grep DENABLE_USE_RAW_FOR_SGI
    if [ $? -ne 0 ]
    then
        export ENABLE_USE_RAW_FOR_SGI=0
    else
        export ENABLE_USE_RAW_FOR_SGI=1
    fi

    build_openvswitch_network

    ping -c 1 router.eur > /dev/null || { echo_fatal "router.eur does not respond to ping" >&2 ; }
    IP_ROUTER=`python -c 'import socket; print socket.gethostbyname("router.eur")'`
    export MAC_ROUTER=`ip neigh show | grep $IP_ROUTER | cut -d ' '  -f5 | tr -d ':'`
    echo_success "ROUTER MAC ADDRESS= $MAC_ROUTER"

    if [ $ENABLE_USE_NETFILTER_FOR_SGI -eq 1 ]; then

        bash_exec "modprobe nf_conntrack"
        bash_exec "modprobe nf_conntrack_ftp"

        ######################################################
        # PREROUTING
        ######################################################
        # We restore the mark following the CONNMARK mark. In fact, it does a simple MARK=CONNMARK
        # where MARK is the standard mark (usable by tc)
        # In French: Cette option de cible restaure le paquet marqué dans la marque de connexion
        # comme défini par CONNMARK. Un masque peut aussi être défini par l'option --mask.
        # Si une option mask est placée, seules les options masquées seront placées.
        # Notez que cette option de cible n'est valide que dans la table mangle.
        bash_exec "$IPTABLES -t mangle -A PREROUTING -j CONNMARK --restore-mark"

        # TEST bash_exec "$IPTABLES -t mangle -A PREROUTING -m mark --mark 0 -i $PGW_INTERFACE_NAME_FOR_SGI -j MARK --set-mark 15"
        # We set the mark of the initial packet as value of the conntrack mark for all the packets of the connection.
        # This mark will be restore for the other packets by the first rule of POSTROUTING --restore-mark).
        bash_exec "$IPTABLES -t mangle -A PREROUTING -j CONNMARK --save-mark"


        ######################################################
        # POSTROUTING
        ######################################################

        # MARK=CONNMARK
        bash_exec "iptables -A POSTROUTING -t mangle -o tap0 -j CONNMARK --restore-mark"
        # If we’ve got a mark no need to get further[
        bash_exec "iptables -A POSTROUTING -t mangle -o tap0 -m mark ! --mark 0 -j ACCEPT"

        #bash_exec "iptables -A POSTROUTING -p tcp --dport 21 -t mangle -j MARK --set-mark 1"
        #bash_exec "iptables -A POSTROUTING -p tcp --dport 80 -t mangle -j MARK --set-mark 2"

        # We set the mark of the initial packet as value of the conntrack mark for all the packets
        # of the connection. This mark will be restore for the other packets by the first rule
        # of POSTROUTING (–restore-mark).
        bash_exec "iptables -A POSTROUTING -t mangle -j CONNMARK --save-mark"

        bash_exec "iptables -A PREROUTING  -t mangle -j CONNMARK --restore-mark"

        # We restore the mark following the CONNMARK mark.
        # In fact, it does a simple MARK=CONNMARK where MARK is the standard mark (usable by tc)
        #bash_exec "$IPTABLES -A OUTPUT -t mangle -m mark ! --mark 0 -j CONNMARK --restore-mark"

        # If we’ve got a mark no need to get further[1]
        #TEST bash_exec "$IPTABLES -A OUTPUT -t mangle -p icmp -j MARK --set-mark 14"
        #bash_exec "$IPTABLES -A OUTPUT -t mangle -m mark ! --mark 0 -j ACCEPT"


        # We set the mark of the initial packet as value of the conntrack mark for all the packets of the connection.
        # This mark will be restore for the other packets by the first rule of OUTPUT (–restore-mark).
        #bash_exec "$IPTABLES -A OUTPUT -t mangle -j CONNMARK --save-mark"




        ######################################################
        # NETFILTER QUEUE
        ######################################################
        bash_exec "$IPTABLES -t mangle -A PREROUTING -i $PGW_INTERFACE_NAME_FOR_SGI -m connmark  --mark 5 -j NFQUEUE --queue-num 1"
        bash_exec "$IPTABLES -t mangle -A PREROUTING -i $PGW_INTERFACE_NAME_FOR_SGI -m connmark  --mark 6 -j NFQUEUE --queue-num 1"
        bash_exec "$IPTABLES -t mangle -A PREROUTING -i $PGW_INTERFACE_NAME_FOR_SGI -m connmark  --mark 7 -j NFQUEUE --queue-num 1"
        bash_exec "$IPTABLES -t mangle -A PREROUTING -i $PGW_INTERFACE_NAME_FOR_SGI -m connmark  --mark 8 -j NFQUEUE --queue-num 1"
        bash_exec "$IPTABLES -t mangle -A PREROUTING -i $PGW_INTERFACE_NAME_FOR_SGI -m connmark  --mark 9 -j NFQUEUE --queue-num 1"
        bash_exec "$IPTABLES -t mangle -A PREROUTING -i $PGW_INTERFACE_NAME_FOR_SGI -m connmark  --mark 10 -j NFQUEUE --queue-num 1"
        bash_exec "$IPTABLES -t mangle -A PREROUTING -i $PGW_INTERFACE_NAME_FOR_SGI -m connmark  --mark 11 -j NFQUEUE --queue-num 1"
        bash_exec "$IPTABLES -t mangle -A PREROUTING -i $PGW_INTERFACE_NAME_FOR_SGI -m connmark  --mark 12 -j NFQUEUE --queue-num 1"
        bash_exec "$IPTABLES -t mangle -A PREROUTING -i $PGW_INTERFACE_NAME_FOR_SGI -m connmark  --mark 13 -j NFQUEUE --queue-num 1"
        bash_exec "$IPTABLES -t mangle -A PREROUTING -i $PGW_INTERFACE_NAME_FOR_SGI -m connmark  --mark 14 -j NFQUEUE --queue-num 1"
        bash_exec "$IPTABLES -t mangle -A PREROUTING -i $PGW_INTERFACE_NAME_FOR_SGI -m connmark  --mark 15 -j NFQUEUE --queue-num 1"

        #echo 0 > /proc/sys/net/bridge/bridge-nf-call-iptables #To disable Iptables in the bridge.
        #Raw table: Some years ago appeared a new tables in Iptables.
        #This table can be used to avoid packets (connection really) to enter the NAT table:
        # iptables -t raw -I PREROUTING -i BRIDGE -s x.x.x.x -j NOTRACK.

        #bash_exec "$IPTABLES -t nat -A POSTROUTING -o $PGW_INTERFACE_NAME_FOR_SGI -j SNAT --to-source $PGW_IP_ADDR_FOR_SGI"
    else
        # # get ipv4 address from PGW_INTERFACE_NAME_FOR_SGI
        #IP_ADDR=`ifconfig $PGW_INTERFACE_NAME_FOR_SGI | awk '/inet addr/ {split ($2,A,":"); print A[2]}' | tr '\n' ' ' | sed -n '1h;1!H;${;g;s/^[ \t]*//g;s/[ \t]*$//g;p;}'`

        #NETWORK=`echo $IP_ADDR | cut -d . -f 1,2,3`

        bash_exec "modprobe 8021q"

        for i in 5 6 7 8 9 10 11 12 13 14 15
        do
            # create vlan interface
            ifconfig    $PGW_INTERFACE_NAME_FOR_SGI.$i down > /dev/null 2>&1
            vconfig rem $PGW_INTERFACE_NAME_FOR_SGI.$i      > /dev/null 2>&1
            sync
            bash_exec "vconfig add $PGW_INTERFACE_NAME_FOR_SGI $i"
            sync
            # configure vlan interface
            #CIDR=$NETWORK'.'$i'/24'
            base=200
            NET=$(( $i + $base ))
            CIDR='10.0.'$NET'.2/8'
            bash_exec "ip -4 addr add  $CIDR dev $PGW_INTERFACE_NAME_FOR_SGI.$i"
        done
    fi


    bash_exec "ip link set $PGW_INTERFACE_NAME_FOR_SGI promisc on"

    ##################################################
    # build bridge between SPGW and Internet
    ##################################################

    # # get ipv4 address from PGW_INTERFACE_NAME_FOR_SGI
    # IP_ADDR=`ifconfig $PGW_INTERFACE_NAME_FOR_SGI | awk '/inet addr/ {split ($2,A,":"); print A[2]}' | tr '\n' ' ' | sed -n '1h;1!H;${;g;s/^[ \t]*//g;s/[ \t]*$//g;p;}'`
    # if [ $IP_ADDR ]; then
    #   bash_exec "ip -4 addr del $IP_ADDR dev $PGW_INTERFACE_NAME_FOR_SGI"
    # fi
    #
    # # remove all ipv6 address from PGW_INTERFACE_NAME_FOR_SGI
    # IP_ADDR="not empty"
    # until [ "$IP_ADDR"x == "x" ]; do
    #   IP_ADDR=`ifconfig $PGW_INTERFACE_NAME_FOR_SGI | grep 'inet6' | head -1 | tr '\n' ' ' | sed -n '1h;1!H;${;g;s/^[ \t]*//g;s/[ \t]*$//g;p;}' | cut -d ' ' -f3`
    #   if [ $IP_ADDR ]; then
    #     bash_exec "ip -6 addr del $IP_ADDR dev $PGW_INTERFACE_NAME_FOR_SGI"
    #   fi
    # done
}

test_epc_ovs_network() {
    # Get MAC address of router.eur
    ping -c 1 hss.eur > /dev/null || { echo_fatal "hss.eur does not respond to ping" >&2 ; }
    ping -c 1 router.eur > /dev/null || { echo_fatal "router.eur does not respond to ping" >&2 ; }
    test_openvswitch_network
}

clean_epc_ovs_network() {
    bash_exec "modprobe tun"
    bash_exec "modprobe ip_tables"
    bash_exec "modprobe iptable_nat"
    bash_exec "modprobe x_tables"

    bash_exec "$IPTABLES -P INPUT ACCEPT"
    bash_exec "$IPTABLES -F INPUT"
    bash_exec "$IPTABLES -P OUTPUT ACCEPT"
    bash_exec "$IPTABLES -F OUTPUT"
    bash_exec "$IPTABLES -P FORWARD ACCEPT"
    bash_exec "$IPTABLES -F FORWARD"
    bash_exec "$IPTABLES -t raw    -F"
    bash_exec "$IPTABLES -t nat    -F"
    bash_exec "$IPTABLES -t mangle -F"
    bash_exec "$IPTABLES -t filter -F"

    bash_exec "ip route flush cache"

    echo "   Disabling forwarding"
    bash_exec "sysctl -w net.ipv4.ip_forward=0"
    assert "  `sysctl -n net.ipv4.ip_forward` -eq 0" $LINENO

    echo "   Enabling DynamicAddr.."
    bash_exec "sysctl -w net.ipv4.ip_dynaddr=1"
    assert "  `sysctl -n net.ipv4.ip_dynaddr` -eq 1" $LINENO

    bash_exec "sysctl -w net.ipv4.conf.all.log_martians=1"
    assert "  `sysctl -n net.ipv4.conf.all.log_martians` -eq 1" $LINENO


    echo "   Disabling reverse path filtering"
    bash_exec "sysctl -w net.ipv4.conf.all.rp_filter=0"
    assert "  `sysctl -n net.ipv4.conf.all.rp_filter` -eq 0" $LINENO
    
    
    for i in 5 6 7 8 9 10 11 12 13 14 15
    do
        ifconfig $PGW_INTERFACE_NAME_FOR_SGI.$i down > /dev/null 2>&1
        vconfig rem $PGW_INTERFACE_NAME_FOR_SGI.$i   > /dev/null 2>&1
    done
    
    clean_openvswitch_network
}


###########################################################
IPTABLES=/sbin/iptables
THIS_SCRIPT_PATH=$(dirname $(readlink -f $0))
declare -x OPENAIR_DIR=""
declare -x OPENAIR1_DIR=""
declare -x OPENAIR2_DIR=""
declare -x OPENAIR3_DIR=""
declare -x OPENAIRCN_DIR=""
declare -x OPENAIR_TARGETS=""
###########################################################

set_openair
cecho "OPENAIR_DIR     = $OPENAIR_DIR" $green
cecho "OPENAIR1_DIR    = $OPENAIR1_DIR" $green
cecho "OPENAIR2_DIR    = $OPENAIR2_DIR" $green
cecho "OPENAIR3_DIR    = $OPENAIR3_DIR" $green
cecho "OPENAIRCN_DIR   = $OPENAIRCN_DIR" $green
cecho "OPENAIR_TARGETS = $OPENAIR_TARGETS" $green
