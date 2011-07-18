CC = gcc

SSE3PROC = $(shell echo `grep ssse3 /proc/cpuinfo`) 
SSE4PROC = $(shell echo `grep sse4 /proc/cpuinfo`) 
CPUFLAGS = -mmmx -msse -msse2 -m32  -mssse3 #-msse4
CPUFLAGS += $(shell if [ -z $$SSE3PROC]; then echo "" ; else echo "-mssse3"; fi)
CPUFLAGS += $(shell if [ -z $$SSE4PROC]; then echo "" ; else echo "-msse4"; fi)
cygwin = $(shell if [ $$OSTYPE = "cygwin" ] ; then echo "1" ; else echo "0" ; fi)

TOP_DIR      = $(OPENAIR1_DIR)
OPENAIR1_TOP = $(OPENAIR1_DIR)
OPENAIR2_TOP = $(OPENAIR2_DIR)
OPENAIR3_TOP = $(OPENAIR3_DIR)
OPENAIR3     = $(OPENAIR3_DIR)

CFLAGS = -Wall -fno-strict-aliasing -DPHYSIM -DNODE_RG -DUSER_MODE -DPC_TARGET -DPC_DSP -DNB_ANTENNAS_RX=2 -DNB_ANTENNAS_TXRX=2 -DNB_ANTENNAS_TX=2 -DMAX_MODULES=1 -g -ggdb $(CPUFLAGS) -I/usr/include/X11 #-Wno-packed-bitfield-compat

ASN1_MSG_INC = $(OPENAIR2_DIR)/RRC/LITE/MESSAGES

CFLAGS += -DOPENAIR_LTE -DOPENAIR2 #-DOFDMA_ULSCH -DIFFT_FPGA -DIFFT_FPGA_UE 
CFLAGS += -I/usr/include/libxml2 -lxml2

#CFLAGS += -DXFORMS

ifdef DEBUG_PHY
CFLAGS += -DDEBUG_PHY
endif

ifdef NAS_NETLINK
CFLAGS += -DNAS_NETLINK -DLINUX -DDEBUG_CONTROL
endif

CFLAGS += -DPHY_ABSTRACTION  #-DEMIT_ASN_DEBUG=1
CFLAGS += $(shell if [ $$OSTYPE = "cygwin" ] ; then echo "-DCYGWIN" ; fi)

include $(OPENAIR1_DIR)/PHY/Makefile.inc
include $(OPENAIR1_DIR)/SCHED/Makefile.inc
include $(OPENAIR2_DIR)/LAYER2/Makefile.inc
include $(OPENAIR1_DIR)/SIMULATION/ETH_TRANSPORT/Makefile.inc
include $(OPENAIR2_DIR)/RRC/LITE/MESSAGES/Makefile.inc

SIMULATION_OBJS  = $(TOP_DIR)/SIMULATION/TOOLS/gauss.o  
SIMULATION_OBJS += $(TOP_DIR)/SIMULATION/TOOLS/random_channel.o  
SIMULATION_OBJS += $(TOP_DIR)/SIMULATION/TOOLS/rangen_double.o  
SIMULATION_OBJS += $(TOP_DIR)/SIMULATION/TOOLS/taus.o  
SIMULATION_OBJS += $(TOP_DIR)/SIMULATION/TOOLS/multipath_channel.o
SIMULATION_OBJS += $(TOP_DIR)/SIMULATION/RF/rf.o
SIMULATION_OBJS += $(TOP_DIR)/SIMULATION/RF/adc.o
SIMULATION_OBJS += $(TOP_DIR)/SIMULATION/RF/dac.o

ASN1_MSG_OBJS2 := $(foreach file, $(ASN1_MSG_OBJS), $(OPENAIR2_DIR)/RRC/LITE/MESSAGES/$(file))

OBJ = $(PHY_OBJS) $(SIMULATION_OBJS) $(ETHERNET_TRANSPORT_OBJS) $(TOOLS_OBJS) $(SCHED_OBJS) $(STATS_OBJS) $(ASN1_MSG_OBJS2)

all: oaisim


$(OBJ) : %.o : %.c
	$(CC) -c $(CFLAGS) $(EXTRA_CFLAGS) -DPHY_CONTEXT=1 -I$(TOP_DIR) $(L2_incl) -I$(ASN1_MSG_INC) -o $@ $<

$(L2_OBJS) : %.o : %.c
	$(CC) -c $(CFLAGS) $(EXTRA_CFLAGS) -DMAC_CONTEXT=1 -DPHY_CONTEXT=1 -I$(TOP_DIR) $(L2_incl) -I$(ASN1_MSG_INC) -o $@ $<

oaisim : $(OBJ) $(L2_OBJS) oaisim.c  
	echo CYGWIN variable is $(cygwin)
	$(CC)  oaisim.c -I$(TOP_DIR) $(L2_incl) -I$(ASN1_MSG_INC) -o oaisim $(CFLAGS) $(EXTRA_CFLAGS) $(OBJ) $(L2_OBJS) -lm -lforms -L/usr/local/lib -lforms -lX11 -L/usr/X11R6/lib -lXpm -lblas -lpthread


clean:
	rm -f physim
	rm -f $(OBJ)
	rm -f $(L2_OBJS)
	rm -f *.o
	rm -f *.exe*

cleanl1:
	rm -f physim
	rm -f $(OBJ)
	rm -f *.o
	rm -f *.exe
cleanl2:
	rm -f $(L2_OBJS)
