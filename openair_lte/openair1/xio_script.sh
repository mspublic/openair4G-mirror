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
