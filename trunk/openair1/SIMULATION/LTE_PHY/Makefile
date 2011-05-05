CC = $(shell if [ $(OSTYPE) = "cygwin" ]; then echo "gcc-4";  else echo "gcc"; fi)
#CC = gcc-4.3

SSE3PROC = $(shell echo `grep ssse3 /proc/cpuinfo`) 
SSE4PROC = $(shell echo `grep sse4 /proc/cpuinfo`) 
CPUFLAGS = -mmmx -msse -msse2 -m32 #-mssse3 -msse4
CPUFLAGS += $(shell if [ -z $(SSE3PROC) ]; then echo "" ; else echo "-mssse3"; fi)
CPUFLAGS += $(shell if [ -z $(SSE4PROC) ]; then echo "" ; else echo "-msse4"; fi)

TOP_DIR = $(OPENAIR1_DIR)
OPENAIR1_TOP = $(OPENAIR1_DIR)
OPENAIR2_TOP = $(OPENAIR2_DIR)
OPENAIR3 = $(OPENAIR3_DIR)

CFLAGS = -O2 -Wall -fno-common -mpreferred-stack-boundary=4 -fno-strict-aliasing -DPHYSIM -DNODE_RG -DUSER_MODE -DPC_TARGET -DPC_DSP -DNB_ANTENNAS_RX=2 -DNB_ANTENNAS_TXRX=2 -DNB_ANTENNAS_TX=2 -DMAX_MODULES=1 -DPHY_CONTEXT=1 -g -ggdb $(CPUFLAGS) 

GCCVERSION = $(shell gcc --version | grep ^gcc | sed 's/^.* //g')
ifeq "$(GCCVERSION)" "4.4.3"
    CFLAGS += -Wno-packed-bitfield-compat
endif

CFLAGS += -DOPENAIR_LTE #-DOFDMA_ULSCH #-DIFFT_FPGA -DIFFT_FPGA_UE
#CFLAGS += -DTBS_FIX
CFLAGS += -DCELLULAR

ASN1_MSG_INC = $(OPENAIR2_DIR)/RRC/LITE/MESSAGES

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

CFLAGS += -DOPENAIR2 -DNO_RRM -DPHY_ABSTRACTION

CFLAGS += -I/usr/include/X11 -I/usr/X11R6/include


include $(TOP_DIR)/PHY/Makefile.inc
include $(TOP_DIR)/SCHED/Makefile.inc
include $(TOP_DIR)/SIMULATION/Makefile.inc
include $(TOP_DIR)/../openair2/LAYER2/Makefile.inc
include $(OPENAIR2_DIR)/RRC/LITE/MESSAGES/Makefile.inc

CFLAGS += -I$(TOP_DIR) $(L2_incl) -I$(ASN1_MSG_INC) -I$(OPENAIR3)

#STATS_OBJS += $(TOP_DIR)/ARCH/CBMIMO1/DEVICE_DRIVER/cbmimo1_proc.o

#LAYER2_OBJ += $(OPENAIR2_DIR)/LAYER2/MAC/rar_tools.o

OBJ = $(PHY_OBJS) $(SIMULATION_OBJS) $(TOOLS_OBJS) $(SCHED_OBJS) #$(ASN1_MSG_OBJS) 
OBJ2 = $(PHY_OBJS) $(SIMULATION_OBJS) $(TOOLS_OBJS)  

all: dlsim pbchsim pdcchsim ulsim pucchsim

test: $(SIMULATION_OBJS) $(TOOLS_OBJS) $(TOP_DIR)/PHY/INIT/lte_init.o test.c
	$(CC)  test.c -o test $(CFLAGS) $(SIMULATION_OBJS) $(TOOLS_OBJS) -lm 

$(OBJ) : %.o : %.c
	$(CC) -c $(CFLAGS) -o $@ $<

dlsim : $(OBJ) dlsim.c
	$(CC) dlsim.c -o dlsim $(CFLAGS) $(OBJ) -lm -lblas 

framegen: $(OBJ) framegen.c
	$(CC) framegen.c -o framegen $(CFLAGS) $(OBJ) -lm -lblas

pbchsim : $(OBJ) pbchsim.c
	$(CC) pbchsim.c -o pbchsim $(CFLAGS) $(OBJ) -lm -lblas

pdcchsim : $(OBJ) pdcchsim.c
	$(CC) pdcchsim.c -o pdcchsim $(CFLAGS) $(OBJ) -lm -lblas

pucchsim : $(OBJ)  pucchsim.c
	$(CC) pucchsim.c -o pucchsim $(CFLAGS) $(OBJ) -lm -lblas

dlsim_secsys : $(OBJ2) dlsim_secsys.c
	$(CC) dlsim_secsys.c -o dlsim_secsys $(CFLAGS) $(OBJ2) -lm -lblas

ulsim : $(OBJ) ulsim.c #ulsim_form.c
	$(CC) ulsim.c -o ulsim $(CFLAGS) $(OBJ) -lm -lforms -L/usr/local/lib -lforms -lX11 -L/usr/X11R6/lib -lXpm -lblas

ulsim2 : $(OBJ2) ulsim2.c
	$(CC) ulsim2.c -o ulsim2 $(CFLAGS) $(OBJ2) -lm

physim : $(OBJ) phy_procedures_sim.c 
	$(CC)  phy_procedures_sim.c -o physim $(CFLAGS) $(OBJ) -lm

physim_secsys : $(OBJ) phy_procedures_sim_secsys.c 
	$(CC)  phy_procedures_sim_secsys.c -o physim_secsys $(CFLAGS) $(OBJ) -lm

clean:
	rm -f dlsim dlsim2 dlsim_secsys
	rm -f $(OBJ)
	rm -f *.o
	rm -f *.exe*

showcflags:
	@echo $(CFLAGS)

