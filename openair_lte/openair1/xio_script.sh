# This script configures the ExpressCard to Cardbus Adaptor from STLab to work properly with the CBMIMO1 cards. The numeric ID of the card should be 
XIO_ADAPTER_ID = 104c:8231

# We also need to configure the PCI bridge the card is plugged into. To find the ID of the root port, type
# lspci -tnnv | grep 16e3:0210
# You should see a line which looks like this 
# +-1c.3-[0000:0e-0f]----00.0-[0000:0f]----00.0 European Space Agency Device [16e3:0210]
# (16e3:0210 is the ID of the CBMIMO1 card)
# From the output we can find the slot number of the PCI bridge is 1c.3. To find out the ID of that bridge type
# lspci -s 1c.3 -nn
# the output should be something like
# 00:1c.3 PCI bridge [0604]: Intel Corporation 82801I (ICH9 Family) PCI Express Port 4 [8086:2946] (rev 02)
# from that we can read that the ID of the root port is
ROOT_PORT_ID = 8086:2946


# Disable ASPM on root port
setpci -d 8086:2946 50.W=0040

# XIO2000 Prefetch 4x, threshold disabled
setpci -d 104c:8231 c0.B=00

# XIO2000 L1 Latency 4us
setpci -d 104c:8231 C4.L=00108108

# XIO2000 common reference clock, ASPM disabled
setpci -d 104c:8231 A0.W=0040

setpci -d 104c:8231 d4.L=83042000

setpci -d 104c:8231 CACHE_LINE_SIZE=20

setpci -d 104c:8231 SEC_LATENCY_TIMER=10

#setpci -d 16e3:0210 CACHE_LINE_SIZE=80
#setpci -d 16e3:0210 LATENCY_TIMER=10
