CC = gcc

TOP_DIR = ../..

CFLAGS = -Wall -fno-common -mpreferred-stack-boundary=4  -DDEBUG_PHY -DNODE_RG -DUSER_MODE -DPC_TARGET -DPC_DSP -DNB_ANTENNAS_RX=2 -DNB_ANTENNAS_TXRX=2 -DNB_ANTENNAS_TX=2 -DMAX_MODULES=1 -DPHY_CONTEXT=1 -g -ggdb -msse2 -mmmx -msse -DFILE_OUTPUT -mssse3 -DOPENAIR_LTE

PHY_OBJS = $(TOP_DIR)/PHY/LTE_TRANSPORT/pss.o
PHY_OBJS += $(TOP_DIR)/PHY/MODULATION/ofdm_mod.o
PHY_OBJS += $(TOP_DIR)/PHY/LTE_ESTIMATION/lte_sync_time.o
PHY_OBJS += $(TOP_DIR)/PHY/INIT/lte_init.o

TOOLS_OBJS = $(TOP_DIR)/PHY/TOOLS/file_output.o
TOOLS_OBJS += $(TOP_DIR)/PHY/TOOLS/fft.o

OBJ = $(PHY_OBJS) $(SIMULATION_OBJS) $(TOOLS_OBJS)

all: dlsim

$(OBJ) : %.o : %.c
	$(CC) -c $(CFLAGS) -Wall -I$(TOP_DIR) -o $@ $<

dlsim : $(OBJ) dlsim.c
	$(CC) dlsim.c -I$(TOP_DIR) -o dlsim $(CFLAGS) $(OBJ) -lm

clean:
	rm -f dlsim
	rm -f $(PHY_OBJS)
	rm -f *.o
	rm -f *.exe*

