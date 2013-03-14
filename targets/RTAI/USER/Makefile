GCCVERSION = $(shell gcc --version | grep ^gcc | sed 's/^.* //g')
ifeq "$(GCCVERSION)"  "4.4.3"
    CFLAGS += -Wno-packed-bitfield-compat
endif
ifeq "$(GCCVERSION)" "4.5.2"
    CFLAGS += -Wno-packed-bitfield-compat
endif
ifeq "$(GCCVERSION)" "4.6.1"
    CFLAGS += -Wno-packed-bitfield-compat
endif

CFLAGS += -O1

SRC = synctest.c condtest.c

RTAI_OBJ = sched_dlsch.o sched_ulsch.o sched_rx_pdsch.o
OBJ = $(OPENAIR1_DIR)/SIMULATION/TOOLS/taus.o $(OPENAIR_TARGETS)/SIMU/USER/init_lte.o 
ifdef DRIVER2013
RTAI_OBJ += lte-softmodem.o
OBJ += $(OPENAIR_TARGETS)/ARCH/EXMIMO/USERSPACE/LIB/openair0_lib.o
CFLAGS += -DDRIVER2013 -I$(OPENAIR_TARGETS)/ARCH/EXMIMO/USERSPACE/LIB/ -I$(OPENAIR_TARGETS)/ARCH/EXMIMO/DEFS
else
RTAI_OBJ += synctest.o 
OBJ += $(OPENAIR1_DIR)/SIMULATION/LTE_PHY/openair_hw.o $(OPENAIR1_DIR)/ARCH/CBMIMO1/DEVICE_DRIVER/cbmimo1_rf_cntl.o 
endif

TOP_DIR = $(OPENAIR1_DIR)
include $(OPENAIR1_DIR)/PHY/Makefile.inc
include $(OPENAIR1_DIR)/SCHED/Makefile.inc

OBJ += $(PHY_OBJS) 
RTAI_OBJ += $(SCHED_OBJS)

OPENAIR2_TOP = $(OPENAIR2_DIR)
include $(OPENAIR2_DIR)/LAYER2/Makefile.inc
include $(OPENAIR2_DIR)/UTIL/Makefile.inc
include $(OPENAIR2_DIR)/RRC/LITE/MESSAGES/Makefile.inc
include $(OPENAIR2_DIR)/RRC/NAS/Makefile.inc

OBJ += $(LOG_OBJS) 

ifndef OPENAIR2
OPENAIR2=1
endif
ifeq ($(OPENAIR2),1)
ASN1_MSG_OBJS1=$(addprefix $(OPENAIR2_DIR)/RRC/LITE/MESSAGES/, $(ASN1_MSG_OBJS))
OBJ += $(L2_OBJS) $(LIST_OBJ) $(TIMER_OBJ) $(MEM_OBJ) $(OTG_OBJS) $(MATH_OBJS) 
OBJ += $(OPENAIR1_DIR)/SIMULATION/ETH_TRANSPORT/netlink_init.o
CFLAGS += -DOPENAIR2 -DNO_RRM -DPUCCH -DMAC_CONTEXT=1
endif

CFLAGS += $(L2_incl) $(UTIL_incl)

CFLAGS += -I$(OPENAIR1_DIR) -I$(OPENAIR2_DIR)/RRC/LITE/MESSAGES	#-I$(OPENAIR3_DIR)/MESH -I$(OPENAIR3_DIR)/MESH/RRM

CFLGAS += -DNEW_FFT

ifeq ($(XFORMS),1)
CFLAGS += -DXFORMS -I/usr/include/X11
LDFLAGS += -lforms
OBJ += $(OPENAIR1_DIR)/PHY/TOOLS/lte_phy_scope.o
OBJ += stats.o
endif

CPUFLAGS = -mmmx -msse -msse2 -m32 -mssse3 -msse4.1
#CFLAGS += -fno-common -mpreferred-stack-boundary=4  
CFLAGS += -fno-strict-aliasing -DPHYSIM -DNODE_RG -DUSER_MODE -DPC_TARGET -DPC_DSP -DNB_ANTENNAS_RX=2 -DNB_ANTENNAS_TXRX=2 -DNB_ANTENNAS_TX=2 -DPHY_CONTEXT=1 -g -ggdb -rdynamic $(CPUFLAGS) # -Wno-packed-bitfield-compat
CFLAGS += -DOPENAIR_LTE -DENABLE_FXP -DOPENAIR1 -DDLSCH_THREAD #-DULSCH_THREAD

#only for CBMIMO1
ifdef CBMIMO1
CFLAGS += -DFW2011 -DBIT8_TX -DCBMIMO1
else
#only for EXPRESS MIMO
#CFLAGS += -DEXMIMO -DTIMING_ADVANCE_HW=138 #this is for ExpressMIMO 1
CFLAGS += -DEXMIMO -DTIMING_ADVANCE_HW=45 #this is for ExpressMIMO 2
endif

ifeq ($(HARD_RT),1)
CFLAGS += -DHARD_RT
endif

ifeq ($(EMOS),1)
CFLAGS += -DEMOS
LDFLAGS += -lgps
endif

ifeq ($(NAS),1)
CFLAGS += -DNAS_NETLINK -DLINUX
OBJ += $(NAS_OBJS)
endif

RTAI_CFLAGS += $(shell rtai-config --lxrt-cflags)
LDFLAGS += $(shell rtai-config --lxrt-ldflags) -lpthread -llxrt -lm

all: condtest synctest

$(RTAI_OBJ): %.o : %.c
	@echo Compiling $< ...
	@$(CC) -c $(CFLAGS) $(RTAI_CFLAGS) -o $@ $<

$(OBJ) $(ASN1_MSG_OBJS1): %.o : %.c
	@echo Compiling $< ...
	@$(CC) -c $(CFLAGS) -o $@ $<

condtest: condtest.c 
	$(CC) $(CFLAGS) $(LDFLAGS) condtest.c -o condtest

synctest: $(OBJ) $(ASN1_MSG_OBJS1) $(RTAI_OBJ)
	$(CC) $(CFLAGS) $(LDFLAGS) $(OBJ) $(RTAI_OBJ) $(ASN1_MSG_OBJS1) -o synctest

lte-softmodem: $(OBJ) $(ASN1_MSG_OBJS1) $(RTAI_OBJ)
	$(CC) $(CFLAGS) $(LDFLAGS) $(OBJ) $(RTAI_OBJ) $(ASN1_MSG_OBJS1) -o lte-softmodem

synctest_eNB: synctest
	cp synctest synctest_eNB

synctest_UE: synctest
	cp synctest synctest_UE

drivers:
	cd $(OPENAIR1_DIR) && make clean && make oai_user_exmimo.ko
	cd $(OPENAIR2_DIR) && make clean && make nasmesh_netlink.ko

run: condtest
	rtai-load condtest --verbose

run_eNB_test: 
	rtai-load eNB_test --verbose

run_eNB: 
	rtai-load eNB --verbose

run_UE: 
	rtai-load UE --verbose

run_UE0:
	rtai-load UE0 --verbose

run_UE0prach:
	rtai-load UE0prach --verbose

run_UE1prach:
	rtai-load UE1prach --verbose

run_UE2prach:
	rtai-load UE2prach --verbose

run_UE0noL2:
	rtai-load UE0noL2 -- verbose

run_UE1noL2: 
	rtai-load UE1noL2 --verbose

run_UE2noL2: 
	rtai-load UE2noL2 --verbose

run_UE0calib:
	rtai-load UE0calib --verbose

run_UE0calibmed:
	rtai-load UE0calibmed --verbose

run_UE0calibbyp:
	rtai-load UE0calibbyp --verbose

run_UE1: 
	rtai-load UE1 --verbose

run_UE2: 
	rtai-load UE2 --verbose

run_eNB0:
	rtai-load eNB0 --verbose

run_eNB1:
	rtai-load eNB1 --verbose

run_eNB2:
	rtai-load eNB2 --verbose

clean:
	rm -rf $(OBJ) $(RTAI_OBJ)

cleanall: clean
	rm -rf condtest synctest lte-softmodem
	rm -rf synctest_eNB synctest_UE
	rm -rf $(ASN1_MSG_OBJS1)

show:
	@echo $(CFLAGS)
	@echo $(OBJ) $(RTAI_OBJ)

beautiful:
	astyle --style=gnu -s2 $(SRC)
