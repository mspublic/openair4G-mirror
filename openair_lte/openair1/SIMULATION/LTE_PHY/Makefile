CC = gcc

TOP_DIR = ../..

CFLAGS = -Wall -fno-common -mpreferred-stack-boundary=4 -fno-strict-aliasing -DNODE_RG -DUSER_MODE -DPC_TARGET -DPC_DSP -DNB_ANTENNAS_RX=2 -DNB_ANTENNAS_TXRX=2 -DNB_ANTENNAS_TX=2 -DMAX_MODULES=1 -DPHY_CONTEXT=1 -g -ggdb -msse2 -mssse3 -msse4 -mmmx -msse -DFILE_OUTPUT -DOPENAIR_LTE

CFLAGS += -DDEBUG_PHY -DIFFT_FPGA

PHY_OBJS = $(TOP_DIR)/PHY/LTE_TRANSPORT/pss.o
PHY_OBJS += $(TOP_DIR)/PHY/LTE_TRANSPORT/pilots.o
PHY_OBJS += $(TOP_DIR)/PHY/LTE_TRANSPORT/dlsch.o
PHY_OBJS += $(TOP_DIR)/PHY/LTE_TRANSPORT/dlsch_rx.o
PHY_OBJS += $(TOP_DIR)/PHY/LTE_TRANSPORT/dlsch_decoding.o
PHY_OBJS += $(TOP_DIR)/PHY/LTE_TRANSPORT/pbch.o
PHY_OBJS += $(TOP_DIR)/PHY/MODULATION/ofdm_mod.o
PHY_OBJS += $(TOP_DIR)/PHY/MODULATION/slot_fep.o
PHY_OBJS += $(TOP_DIR)/PHY/LTE_ESTIMATION/lte_sync_time.o
PHY_OBJS += $(TOP_DIR)/PHY/LTE_ESTIMATION/lte_dl_channel_estimation.o
PHY_OBJS += $(TOP_DIR)/PHY/LTE_REFSIG/lte_dl_cell_spec.o
PHY_OBJS += $(TOP_DIR)/PHY/LTE_REFSIG/lte_gold.o
PHY_OBJS += $(TOP_DIR)/PHY/CODING/ccoding_byte.o
PHY_OBJS += $(TOP_DIR)/PHY/CODING/3gpplte.o
PHY_OBJS += $(TOP_DIR)/PHY/CODING/crc_byte.o
PHY_OBJS += $(TOP_DIR)/PHY/CODING/logmap8_sse2.o
PHY_OBJS += $(TOP_DIR)/PHY/CODING/lte_rate_matching.o
PHY_OBJS += $(TOP_DIR)/PHY/CODING/rate_matching.o
PHY_OBJS += $(TOP_DIR)/PHY/CODING/viterbi.o
PHY_OBJS += $(TOP_DIR)/PHY/TOOLS/cmult_sv.o
PHY_OBJS += $(TOP_DIR)/PHY/INIT/lte_init.o
PHY_OBJS += $(TOP_DIR)/PHY/INIT/init_top.o
PHY_OBJS += $(TOP_DIR)/MAC_INTERFACE/init.o
PHY_OBJS += $(TOP_DIR)/SIMULATION/TOOLS/gauss.o  $(TOP_DIR)/SIMULATION/TOOLS/random_channel.o  $(TOP_DIR)/SIMULATION/TOOLS/rangen_double.o  $(TOP_DIR)/SIMULATION/TOOLS/taus.o  $(TOP_DIR)/SIMULATION/TOOLS/multipath_channel.o



TOOLS_OBJS = $(TOP_DIR)/PHY/TOOLS/file_output.o
TOOLS_OBJS += $(TOP_DIR)/PHY/TOOLS/fft.o
TOOLS_OBJS += $(TOP_DIR)/PHY/TOOLS/log2_approx.o

OBJ = $(PHY_OBJS) $(SIMULATION_OBJS) $(TOOLS_OBJS)

all: dlsim dlsim2

$(OBJ) : %.o : %.c
	$(CC) -c $(CFLAGS) -Wall -I$(TOP_DIR) -o $@ $<

dlsim : $(OBJ) dlsim.c
	$(CC) dlsim.c -I$(TOP_DIR) -o dlsim $(CFLAGS) $(OBJ) -lm

dlsim2 : $(OBJ) dlsim2.c
	$(CC) dlsim2.c -I$(TOP_DIR) -o dlsim2 $(CFLAGS) $(OBJ) -lm

clean:
	rm -f dlsim dlsim2
	rm -f $(PHY_OBJS)
	rm -f *.o
	rm -f *.exe*

