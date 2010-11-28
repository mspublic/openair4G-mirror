CC = $(shell if [ $(OSTYPE) = "cygwin" ]; then echo "gcc-4";  else echo "gcc"; fi)
#CC = gcc-4

SSE3PROC = $(shell echo `grep ssse3 /proc/cpuinfo`) 
SSE4PROC = $(shell echo `grep sse4 /proc/cpuinfo`) 
CPUFLAGS = -mmmx -msse -msse2 -m32 #-mssse3 -msse4
CPUFLAGS += $(shell if [ -z $(SSE3PROC) ]; then echo "" ; else echo "-mssse3"; fi)
CPUFLAGS += $(shell if [ -z $(SSE4PROC) ]; then echo "" ; else echo "-msse4"; fi)

TOP_DIR = ../..

CFLAGS = -O2 -Wall -fno-common -mpreferred-stack-boundary=4 -fno-strict-aliasing -DPHYSIM -DNODE_RG -DUSER_MODE -DPC_TARGET -DPC_DSP -DNB_ANTENNAS_RX=2 -DNB_ANTENNAS_TXRX=2 -DNB_ANTENNAS_TX=2 -DMAX_MODULES=1 -DPHY_CONTEXT=1 -g -ggdb $(CPUFLAGS) 

#CFLAGS = -Wall -Wno-packed-bitfield-compat -fno-common -mpreferred-stack-boundary=4 -fno-strict-aliasing -DNODE_RG -DUSER_MODE -DPC_TARGET -DPC_DSP -DNB_ANTENNAS_RX=2 -DNB_ANTENNAS_TXRX=2 -DNB_ANTENNAS_TX=2 -DMAX_MODULES=1 -DPHY_CONTEXT=1 -g -ggdb $(CPUFLAGS)

CFLAGS += -DOPENAIR_LTE -DOFDMA_ULSCH #-DIFFT_FPGA -DIFFT_FPGA_UE
#CFLAGS += -DTBS_FIX
CFLAGS += -DCELLULAR

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
include $(TOP_DIR)/SIMULATION/Makefile.inc

STATS_OBJS += $(TOP_DIR)/ARCH/CBMIMO1/DEVICE_DRIVER/cbmimo1_proc.o

LAYER2_OBJ += $(OPENAIR2_DIR)/LAYER2/MAC/rar_tools.o

OBJ = $(PHY_OBJS) $(SIMULATION_OBJS) $(TOOLS_OBJS) $(SCHED_OBJS) $(STATS_OBJS) $(LAYER2_OBJ)	
OBJ2 = $(PHY_OBJS) $(SIMULATION_OBJS) $(TOOLS_OBJS)  

all: dlsim pbchsim pdcchsim ulsim ulsim2 

test: $(SIMULATION_OBJS) $(TOOLS_OBJS) $(TOP_DIR)/PHY/INIT/lte_init.o test.c
	$(CC)  test.c -I$(TOP_DIR) -o test $(CFLAGS) $(SIMULATION_OBJS) $(TOOLS_OBJS) -lm 

$(OBJ) : %.o : %.c
	$(CC) -c $(CFLAGS) -I$(TOP_DIR) -o $@ $<

dlsim : $(OBJ2) dlsim.c
	$(CC) dlsim.c -I$(TOP_DIR) -o dlsim $(CFLAGS) $(OBJ2) /usr/lib/libblas.a -lm 

pbchsim : $(OBJ2) pbchsim.c
	$(CC) pbchsim.c -I$(TOP_DIR) -o pbchsim $(CFLAGS) $(OBJ2) -lm -lblas

pdcchsim : $(OBJ2) pdcchsim.c
	$(CC) pdcchsim.c -I$(TOP_DIR) -o pdcchsim $(CFLAGS) $(OBJ2) -lm -lblas

dlsim_secsys : $(OBJ2) dlsim_secsys.c
	$(CC) dlsim_secsys.c -I$(TOP_DIR) -o dlsim_secsys $(CFLAGS) $(OBJ2) -lm -lblas

ulsim : $(OBJ2) ulsim.c #ulsim_form.c
	$(CC) ulsim.c -I$(TOP_DIR) -o ulsim $(CFLAGS) $(OBJ2) -lm -lforms -L/usr/local/lib -lforms -lX11 -L/usr/X11R6/lib -lXpm -lblas

ulsim2 : $(OBJ2) ulsim2.c
	$(CC) ulsim2.c -I$(TOP_DIR) -o ulsim2 $(CFLAGS) $(OBJ2) -lm

physim : $(OBJ) phy_procedures_sim.c 
	$(CC)  phy_procedures_sim.c -I$(TOP_DIR) -o physim $(CFLAGS) $(OBJ) -lm
physim_secsys : $(OBJ) phy_procedures_sim_secsys.c 
	$(CC)  phy_procedures_sim_secsys.c -I$(TOP_DIR) -o physim_secsys $(CFLAGS) $(OBJ) -lm

clean:
	rm -f dlsim dlsim2 dlsim_secsys
	rm -f $(OBJ)
	rm -f *.o
	rm -f *.exe*

showcflags:
	@echo $(CFLAGS)

