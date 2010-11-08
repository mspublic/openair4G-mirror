CC = $(shell if [ $$OSTYPE = "cygwin" ]; then echo "gcc-4";  else echo "gcc"; fi)
#CC = gcc-4

CFLAGS += -DCELLULAR

SSE3PROC = $(shell echo `grep ssse3 /proc/cpuinfo`) 
SSE4PROC = $(shell echo `grep sse4 /proc/cpuinfo`) 
CPUFLAGS = -mmmx -msse -msse2 -m32 #-mssse3 -msse4
CPUFLAGS += $(shell if [ -z $(SSE3PROC) ]; then echo "" ; else echo "-mssse3"; fi)
CPUFLAGS += $(shell if [ -z $(SSE4PROC) ]; then echo "" ; else echo "-msse4"; fi)

TOP_DIR = ../..

CFLAGS = -Wall -fno-common -mpreferred-stack-boundary=4 -fno-strict-aliasing -DPHYSIM -DNODE_RG -DUSER_MODE -DPC_TARGET -DPC_DSP -DNB_ANTENNAS_RX=2 -DNB_ANTENNAS_TXRX=2 -DNB_ANTENNAS_TX=2 -DMAX_MODULES=1 -DPHY_CONTEXT=1 -g -ggdb $(CPUFLAGS) 

#CFLAGS = -Wall -Wno-packed-bitfield-compat -fno-common -mpreferred-stack-boundary=4 -fno-strict-aliasing -DNODE_RG -DUSER_MODE -DPC_TARGET -DPC_DSP -DNB_ANTENNAS_RX=2 -DNB_ANTENNAS_TXRX=2 -DNB_ANTENNAS_TX=2 -DMAX_MODULES=1 -DPHY_CONTEXT=1 -g -ggdb $(CPUFLAGS)

CFLAGS += -DOPENAIR_LTE -DOFDMA_ULSCH #-DIFFT_FPGA -DIFFT_FPGA_UE
#CFLAGS += -DTBS_FIX

ifdef EMOS
CFLAGS += -DEMOS
endif

ifdef DEBUG_PHY
CFLAGS += -DDEBUG_PHY
endif

ifdef MeNBMUE
CFLAGS += -DMeNBMUE
endif

ifdef MU_RECEIVER
CFLAGS += -DMU_RECEIVER
endif

ifdef ZBF_ENABLED
CFLAGS += -DNULL_SHAPE_BF_ENABLED
endif

ifdef RANDOM_BF
CFLAGS += -DRANDOM_BF
endif

ifdef PBS_SIM
CFLAGS += -DPBS_SIM
endif

ifdef OPENAIR2
CFLAGS += -DOPENAIR2 
endif

CFLAGS += -I$(OPENAIR2_DIR) -I$(OPENAIR3_DIR)/MESH -I$(OPENAIR2_DIR)/COMMON -I$(OPENAIR2_DIR)/LAYER2/RLC -I$(OPENAIR2_DIR)/UTIL/MEM -I$(OPENAIR2_DIR)/UTIL/MATH -I$(OPENAIR2_DIR)/UTIL/LISTS -I$(OPENAIR2_DIR)/LAYER2/RLC/AM -I$(OPENAIR2_DIR)/LAYER2/RLC/UM_v6.1.0_LITE -I$(OPENAIR2_DIR)/LAYER2/RLC/TM -I$(OPENAIR2_DIR)/UTIL/TIMER -I/usr/include/X11 -I/usr/X11R6/include


include $(TOP_DIR)/PHY/Makefile.inc
include $(TOP_DIR)/SCHED/Makefile.inc

SIMULATION_OBJS  = $(TOP_DIR)/SIMULATION/TOOLS/gauss.o  
SIMULATION_OBJS += $(TOP_DIR)/SIMULATION/TOOLS/random_channel.o  
SIMULATION_OBJS += $(TOP_DIR)/SIMULATION/TOOLS/rangen_double.o  
SIMULATION_OBJS += $(TOP_DIR)/SIMULATION/TOOLS/taus.o  
SIMULATION_OBJS += $(TOP_DIR)/SIMULATION/TOOLS/multipath_channel.o
SIMULATION_OBJS += $(TOP_DIR)/SIMULATION/RF/rf.o
SIMULATION_OBJS += $(TOP_DIR)/SIMULATION/RF/adc.o
SIMULATION_OBJS += $(TOP_DIR)/SIMULATION/RF/dac.o
SIMULATION_OBJS += $(TOP_DIR)/SIMULATION/TOOLS/llr_quantization.o  # erhan added this!

#STATS_OBJS += $(TOP_DIR)/ARCH/CBMIMO1/DEVICE_DRIVER/cbmimo1_proc.o

LAYER2_OBJ += $(OPENAIR2_DIR)/LAYER2/MAC/rar_tools.o

OBJ = $(PHY_OBJS) $(SIMULATION_OBJS) $(TOOLS_OBJS) $(SCHED_OBJS) $(STATS_OBJS) $(LAYER2_OBJ)	
OBJ2 = $(PHY_OBJS) $(SIMULATION_OBJS) $(TOOLS_OBJS) 


all: relay_sim relay_DF_sim relay_QF_sim relay_test_sim

$(OBJ) : %.o : %.c
	$(CC) -c $(CFLAGS) -I$(TOP_DIR) -o $@ $<

relay_sim : $(OBJ2) relay_sim.c
	$(CC) relay_sim.c -I$(TOP_DIR) -o relay_sim $(CFLAGS) $(OBJ2) -lm -lblas

relay_DF_sim : $(OBJ2) relay_DF_sim.c
	$(CC) relay_DF_sim.c -I$(TOP_DIR) -o relay_DF_sim $(CFLAGS) $(OBJ2) -lm -lblas

relay_QF_sim : $(OBJ2) relay_QF_sim.c
	$(CC) relay_QF_sim.c -I$(TOP_DIR) -o relay_QF_sim $(CFLAGS) $(OBJ2) -lm -lblas

relay_test_sim : $(OBJ2) relay_test_sim.c
	$(CC) relay_test_sim.c -I$(TOP_DIR) -o relay_test_sim $(CFLAGS) $(OBJ2) -lm -lblas

clean:
	rm -f relay_sim relay_DF_sim relay_QF_sim relay_test_sim 
	rm -f $(OBJ)
	rm -f *.o
	rm -f *.exe*

showcflags:
	@echo $(CFLAGS)

