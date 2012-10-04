CFLAGS = $(shell rtai-config --lxrt-cflags)
LDFLAGS = $(shell rtai-config --lxrt-ldflags) -lpthread -llxrt -lm

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

SRC = synctest.c condtest.c

OBJ = synctest.o $(OPENAIR1_DIR)/SIMULATION/LTE_PHY/openair_hw.o $(OPENAIR1_DIR)/SIMULATION/TOOLS/taus.o $(OPENAIR_TARGETS)/SIMU/USER/init_lte.o $(OPENAIR1_DIR)/ARCH/CBMIMO1/DEVICE_DRIVER/cbmimo1_rf_cntl.o sched_dlsch.o sched_ulsch.o

TOP_DIR = $(OPENAIR1_DIR)
include $(OPENAIR1_DIR)/PHY/Makefile.inc
include $(OPENAIR1_DIR)/SCHED/Makefile.inc

OBJ += $(PHY_OBJS) $(SCHED_OBJS)

OPENAIR2_TOP = $(OPENAIR2_DIR)
include $(OPENAIR2_DIR)/LAYER2/Makefile.inc
include $(OPENAIR2_DIR)/UTIL/Makefile.inc
include $(OPENAIR2_DIR)/RRC/LITE/MESSAGES/Makefile.inc

OBJ += $(LOG_OBJS) #$(OTG_OBJS) $(MATH_OBJS)
#OBJ += $(LOG_DIR)/log.o

ifeq ($(OPENAIR2),1)
ASN1_MSG_OBJS1=$(addprefix $(OPENAIR2_DIR)/RRC/LITE/MESSAGES/, $(ASN1_MSG_OBJS))
OBJ += $(L2_OBJS) $(LIST_OBJ) $(TIMER_OBJ) $(MEM_OBJ) 
CFLAGS += -DOPENAIR2 -DNO_RRM -DMAC_CONTEXT=1
endif

CFLAGS += $(L2_incl) $(UTIL_incl)

CFLAGS += -I$(OPENAIR1_DIR) -I$(OPENAIR2_DIR)/RRC/LITE/MESSAGES	#-I$(OPENAIR3_DIR)/MESH -I$(OPENAIR3_DIR)/MESH/RRM

ifeq ($(XFORMS),1)
CFLAGS += -DXFORMS -I/usr/include/X11
LDFLAGS += -lforms
OBJ += lte_scope.o
OBJ += stats.o
endif

CPUFLAGS = -mmmx -msse -msse2 -m32 -mssse3
#CFLAGS += -fno-common -mpreferred-stack-boundary=4  
CFLAGS += -fno-strict-aliasing -DPHYSIM -DNODE_RG -DUSER_MODE -DPC_TARGET -DPC_DSP -DNB_ANTENNAS_RX=2 -DNB_ANTENNAS_TXRX=2 -DNB_ANTENNAS_TX=2 -DPHY_CONTEXT=1 -g -ggdb -rdynamic $(CPUFLAGS) # -Wno-packed-bitfield-compat
CFLAGS += -DOPENAIR_LTE -DENABLE_FXP -DOPENAIR1 -DPUCCH #-DDLSCH_THREAD #-DULSCH_THREAD

#only for CBMIMO1
ifdef CBMIMO1
CFLAGS += -DFW2011 -DBIT8_TX -DCBMIMO1
else
#only for EXPRESS MIMO
CFLAGS += -DEXMIMO -DTIMING_ADVANCE_HW=320 #560 with PXB
endif

ifeq ($(HARD_RT),1)
CFLAGS += -DHARD_RT
endif

ifeq ($(EMOS),1)
CFLAGS += -DEMOS
endif

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

all: condtest synctest

$(OBJ) $(ASN1_MSG_OBJS1): %.o : %.c
	@echo Compiling $< ...
	@$(CC) -c $(CFLAGS) -o $@ $<

condtest: condtest.c 
	$(CC) $(CFLAGS) $(LDFLAGS) condtest.c -o condtest

synctest: $(OBJ) $(ASN1_MSG_OBJS1)
	$(CC) $(CFLAGS) $(LDFLAGS) $(OBJ) $(ASN1_MSG_OBJS1) -o synctest

synctest_eNB: synctest
	mv synctest synctest_eNB

synctest_UE: synctest
	mv synctest synctest_UE

run: condtest
	rtai-load condtest --verbose

run_eNB_test: synctest 
	rtai-load eNB_test --verbose

run_eNB: synctest
	rtai-load eNB --verbose

run_UE: synctest
	rtai-load UE --verbose

run_UE0: synctest
	rtai-load UE0 --verbose

run_UE0prach: synctest
	rtai-load UE0prach --verbose

run_UE0noL2: synctest
	rtai-load UE0noL2 -- verbose

run_UE0calib: synctest
	rtai-load UE0calib --verbose

run_UE0calibmed: synctest
	rtai-load UE0calibmed --verbose

run_UE0calibbyp: synctest
	rtai-load UE0calibbyp --verbose

run_UE1: 
	rtai-load UE1 --verbose

run_UE2: 
	rtai-load UE2 --verbose

run_eNB0:
	rtai-load eNB0 --verbose

clean:
	rm -rf condtest synctest
	rm -rf $(OBJ)

cleanall: clean
	rm -rf $(ASN1_MSG_OBJS1)

show:
	@echo $(CFLAGS)
	@echo $(OBJ)

beautiful:
	astyle --style=gnu -s2 $(SRC)
