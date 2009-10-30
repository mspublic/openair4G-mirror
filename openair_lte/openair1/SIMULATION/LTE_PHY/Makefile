CC = gcc

TOP_DIR = ../..

CFLAGS = -Wall -fno-common -mpreferred-stack-boundary=4  -DDEBUG_PHY -DNODE_RG -DUSER_MODE -DPC_TARGET -DPC_DSP -DNB_ANTENNAS_RX=2 -DNB_ANTENNAS_TXRX=2 -DNB_ANTENNAS_TX=2 -DMAX_MODULES=1 -DPHY_CONTEXT=1 -g -ggdb -msse2 -mmmx -msse -DFILE_OUTPUT -DOPENAIR_LTE

PHY_OBJS = $(TOP_DIR)/PHY/LTE_TRANSPORT/pss.o
PHY_OBJS += $(TOP_DIR)/PHY/LTE_TRANSPORT/pilots.o
PHY_OBJS += $(TOP_DIR)/PHY/MODULATION/ofdm_mod.o
PHY_OBJS += $(TOP_DIR)/PHY/MODULATION/slot_fep.o
PHY_OBJS += $(TOP_DIR)/PHY/LTE_ESTIMATION/lte_sync_time.o
PHY_OBJS += $(TOP_DIR)/PHY/LTE_ESTIMATION/lte_dl_channel_estimation.o
PHY_OBJS += $(TOP_DIR)/PHY/LTE_REFSIG/lte_dl_cell_spec.o
PHY_OBJS += $(TOP_DIR)/PHY/LTE_REFSIG/lte_gold.o
PHY_OBJS += $(TOP_DIR)/PHY/TOOLS/cmult_sv.o
PHY_OBJS += $(TOP_DIR)/PHY/INIT/lte_init.o
PHY_OBJS += $(TOP_DIR)/PHY/INIT/init_top.o
PHY_OBJS += $(TOP_DIR)/MAC_INTERFACE/init.o
PHY_OBJS += $(TOP_DIR)/PHY/CODING/ccoding_byte.o $(TOP_DIR)/PHY/CODING/crc_byte.o $(TOP_DIR)/PHY/CODING/viterbi.o 
PHY_OBJS += $(TOP_DIR)/SIMULATION/TOOLS/gauss.o  $(TOP_DIR)/SIMULATION/TOOLS/random_channel.o  $(TOP_DIR)/SIMULATION/TOOLS/rangen_double.o  $(TOP_DIR)/SIMULATION/TOOLS/taus.o



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

