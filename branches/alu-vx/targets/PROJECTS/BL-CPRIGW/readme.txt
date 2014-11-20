# ---------------------------------------------------------------------------
# Alcatel-Lucent Bell Labs CPRIGW interface in OAI
# (c) 2014
# ---------------------------------------------------------------------------

#
# To compile lte-softmodem with CPRIGW interface
#
make lte-softmodem CPRIGW=1 [other options except EXMIMO, USRP and ETHERNET]

#
# Locate the cprigw config file in LIBCPGW_CFGFILE environment variable
#
export LIBCPGW_CFGFILE=./enb_cprigw10.cfg

# 
# Adapt config file to your system settings
#
# Some important parameters are:
#
#    localip:"191.168.20.10" => set to your local 10GbE NIC address
#    rflags:"USERAW" => raw protocol with local NIC
#    numiqsperchan:87 => number of iqs (i.e. 348)
#

#
# Locate the path to trx_cprigw.so in LD_LIBRARY_PATH
#

# Run lte-softmodem as usual
# You can print CPRIGW statistics in lte-softmodem stdout by sending
# SIGUSR1 signal to lte-softmodem

# CPRIGW debug
# save trx_cprigw.so to trx_cprigw.no_dbg.so
# overwrite trx_cprigw.so with trx_cprigw_dbg.so
# restart lte-softmodem

 
