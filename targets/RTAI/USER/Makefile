CFLAGS = $(shell rtai-config --lxrt-cflags)
LDFLAGS = $(shell rtai-config --lxrt-ldflags) -lpthread -llxrt

CFLAGS += -D__IN_RTAI__ -Wall

OBJ = synctest.o $(OPENAIR1_DIR)/SIMULATION/LTE_PHY/openair_hw.o $(OPENAIR1_DIR)/PHY/INIT/lte_parms.o 

OPENAIR2_TOP = $(OPENAIR2_DIR)
include $(OPENAIR2_DIR)/LAYER2/Makefile.inc
include $(OPENAIR2_DIR)/UTIL/Makefile.inc
CFLAGS += $(L2_incl) $(UTIL_incl)

CFLAGS += -I$(OPENAIR1_DIR) -I$(OPENAIR2_DIR)/RRC/LITE/MESSAGES	

CPUFLAGS = -mmmx -msse -msse2 -m32 -mssse3
CFLAGS += -O2 -Wall -fno-common -mpreferred-stack-boundary=4 -fno-strict-aliasing -DPHYSIM -DNODE_RG -DUSER_MODE -DPC_TARGET -DPC_DSP -DNB_ANTENNAS_RX=2 -DNB_ANTENNAS_TXRX=2 -DNB_ANTENNAS_TX=2 -DPHY_CONTEXT=1 -g -ggdb -rdynamic $(CPUFLAGS) -DMALLOC_CHECK_=1 # -Wno-packed-bitfield-compat
CFLAGS += -DOPENAIR_LTE

all: condtest synctest

condtest: condtest.c 
	$(CC) $(CFLAGS) $(LDFLAGS) condtest.c -o condtest

synctest: $(OBJ)
	$(CC) $(CFLAGS) $(LDFLAGS) $(OBJ) -o synctest

run: all
	rtai-load --verbose

clean:
	rm -rf condtest synctest
	rm -rf $(OBJ)
