CFLAGS = $(shell rtai-config --lxrt-cflags)
LDFLAGS = $(shell rtai-config --lxrt-ldflags) -lpthread -llxrt -lm

SRC = synctest.c condtest.c

OBJ = synctest.o $(OPENAIR1_DIR)/SIMULATION/LTE_PHY/openair_hw.o $(OPENAIR1_DIR)/SIMULATION/TOOLS/taus.o $(OPENAIR_TARGETS)/SIMU/USER/init_lte.o $(OPENAIR1_DIR)/ARCH/CBMIMO1/DEVICE_DRIVER/cbmimo1_rf_cntl.o

TOP_DIR = $(OPENAIR1_DIR)
include $(OPENAIR1_DIR)/PHY/Makefile.inc
include $(OPENAIR1_DIR)/SCHED/Makefile.inc

OBJ += $(PHY_OBJS) $(SCHED_OBJS)

OPENAIR2_TOP = $(OPENAIR2_DIR)
include $(OPENAIR2_DIR)/LAYER2/Makefile.inc
include $(OPENAIR2_DIR)/UTIL/Makefile.inc
include $(OPENAIR2_DIR)/RRC/LITE/MESSAGES/Makefile.inc

OBJ += $(LOG_OBJS)

ifeq ($(OPENAIR2),1)
ASN1_MSG_OBJS1=$(addprefix $(OPENAIR2_DIR)/RRC/LITE/MESSAGES/, $(ASN1_MSG_OBJS))
OBJ += $(L2_OBJS) $(LIST_OBJ) $(TIMER_OBJ) $(MEM_OBJ) 
CFLAGS += -DOPENAIR2 -DNO_RRM -DMAC_CONTEXT=1
endif

CFLAGS += $(L2_incl) $(UTIL_incl)

CFLAGS += -I$(OPENAIR1_DIR) -I$(OPENAIR2_DIR)/RRC/LITE/MESSAGES	#-I$(OPENAIR3_DIR)/MESH -I$(OPENAIR3_DIR)/MESH/RRM

#ifdef XFORMS
CFLAGS += -DXFORMS -I/usr/include/X11
LDFLAGS += -lforms
OBJ += $(OPENAIR1_DIR)/USERSPACE_TOOLS/SCOPE/lte_scope.o
#endif

CPUFLAGS = -mmmx -msse -msse2 -m32 -mssse3
#CFLAGS += -fno-common -mpreferred-stack-boundary=4  
CFLAGS += -fno-strict-aliasing -DPHYSIM -DNODE_RG -DUSER_MODE -DPC_TARGET -DPC_DSP -DNB_ANTENNAS_RX=2 -DNB_ANTENNAS_TXRX=2 -DNB_ANTENNAS_TX=2 -DPHY_CONTEXT=1 -g -ggdb -rdynamic $(CPUFLAGS) # -Wno-packed-bitfield-compat
CFLAGS += -DOPENAIR_LTE -DENABLE_FXP -DOPENAIR1 

#only for CBMIMO1
CFLAGS += -DBIT8_TX -DFW2011 -DCBMIMO1

ifdef HARD_RT
CFLAGS += -DHARD_RT
endif

all: condtest synctest

$(OBJ) $(ASN1_MSG_OBJS1): %.o : %.c
	@echo Compiling $< ...
	@$(CC) -c $(CFLAGS) -o $@ $<

condtest: condtest.c 
	$(CC) $(CFLAGS) $(LDFLAGS) condtest.c -o condtest

synctest: $(OBJ) $(ASN1_MSG_OBJS1)
	$(CC) $(CFLAGS) $(LDFLAGS) $(OBJ) $(ASN1_MSG_OBJS1) -o synctest

run: condtest
	rtai-load condtest --verbose

run_eNB_test: synctest 
	rtai-load eNB_test --verbose

run_eNB: synctest
	rtai-load eNB --verbose

run_UE: synctest
	rtai-load UE --verbose

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