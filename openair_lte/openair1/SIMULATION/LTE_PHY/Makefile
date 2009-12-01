CC = gcc

TOP_DIR = ../..

CFLAGS = -Wall -fno-common -mpreferred-stack-boundary=4 -fno-strict-aliasing -DNODE_RG -DUSER_MODE -DPC_TARGET -DPC_DSP -DNB_ANTENNAS_RX=2 -DNB_ANTENNAS_TXRX=2 -DNB_ANTENNAS_TX=2 -DMAX_MODULES=1 -DPHY_CONTEXT=1 -g -ggdb -msse2 -mssse3 -msse4 -mmmx -msse

CFLAGS += -DDEBUG_PHY -DOPENAIR_LTE -DIFFT_FPGA

ifdef OPENAIR2
CFLAGS += -DOPENAIR2 -I$(OPENAIR2_DIR) -I$(OPENAIR3_DIR)/MESH -I$(OPENAIR2_DIR)/COMMON -I$(OPENAIR2_DIR)/LAYER2/RLC -I$(OPENAIR2_DIR)/UTIL/MEM -I$(OPENAIR2_DIR)/UTIL/MATH -I$(OPENAIR2_DIR)/UTIL/LISTS -I$(OPENAIR2_DIR)/LAYER2/RLC/AM -I$(OPENAIR2_DIR)/LAYER2/RLC/UM_v6.1.0_LITE -I$(OPENAIR2_DIR)/LAYER2/RLC/TM -I$(OPENAIR2_DIR)/UTIL/TIMER
endif


PHY_OBJS = $(TOP_DIR)/PHY/LTE_TRANSPORT/pss.o
PHY_OBJS += $(TOP_DIR)/PHY/LTE_TRANSPORT/pilots.o
PHY_OBJS += $(TOP_DIR)/PHY/LTE_TRANSPORT/dlsch_coding.o
PHY_OBJS += $(TOP_DIR)/PHY/LTE_TRANSPORT/dlsch_modulation.o
PHY_OBJS += $(TOP_DIR)/PHY/LTE_TRANSPORT/dlsch_demodulation.o
PHY_OBJS += $(TOP_DIR)/PHY/LTE_TRANSPORT/dlsch_decoding.o
PHY_OBJS += $(TOP_DIR)/PHY/LTE_TRANSPORT/pbch.o
PHY_OBJS += $(TOP_DIR)/PHY/MODULATION/ofdm_mod.o
PHY_OBJS += $(TOP_DIR)/PHY/MODULATION/slot_fep.o
PHY_OBJS += $(TOP_DIR)/PHY/LTE_ESTIMATION/lte_sync_time.o
PHY_OBJS += $(TOP_DIR)/PHY/LTE_ESTIMATION/lte_adjust_sync.o
PHY_OBJS += $(TOP_DIR)/PHY/LTE_ESTIMATION/lte_dl_channel_estimation.o
PHY_OBJS += $(TOP_DIR)/PHY/LTE_ESTIMATION/lte_est_freq_offset.o
PHY_OBJS += $(TOP_DIR)/PHY/LTE_ESTIMATION/lte_ue_measurements.o
PHY_OBJS += $(TOP_DIR)/PHY/LTE_REFSIG/lte_dl_cell_spec.o
PHY_OBJS += $(TOP_DIR)/PHY/LTE_REFSIG/lte_gold.o
PHY_OBJS += $(TOP_DIR)/PHY/CODING/lte_segmentation.o
PHY_OBJS += $(TOP_DIR)/PHY/CODING/ccoding_byte.o
PHY_OBJS += $(TOP_DIR)/PHY/CODING/3gpplte.o
PHY_OBJS += $(TOP_DIR)/PHY/CODING/crc_byte.o
PHY_OBJS += $(TOP_DIR)/PHY/CODING/3gpplte_turbo_decoder_sse.o
PHY_OBJS += $(TOP_DIR)/PHY/CODING/lte_rate_matching.o
PHY_OBJS += $(TOP_DIR)/PHY/CODING/rate_matching.o
PHY_OBJS += $(TOP_DIR)/PHY/CODING/viterbi.o
PHY_OBJS += $(TOP_DIR)/PHY/INIT/lte_init.o
PHY_OBJS += $(TOP_DIR)/PHY/INIT/init_top.o
PHY_OBJS += $(TOP_DIR)/MAC_INTERFACE/init.o
ifdef OPENAIR2
PHY_OBJS += $(OPENAIR2_DIR)/PHY_INTERFACE/mac_phy_primitives.o
endif

SIMULATION_OBJS  = $(TOP_DIR)/SIMULATION/TOOLS/gauss.o  
SIMULATION_OBJS += $(TOP_DIR)/SIMULATION/TOOLS/random_channel.o  
SIMULATION_OBJS += $(TOP_DIR)/SIMULATION/TOOLS/rangen_double.o  
SIMULATION_OBJS += $(TOP_DIR)/SIMULATION/TOOLS/taus.o  
SIMULATION_OBJS += $(TOP_DIR)/SIMULATION/TOOLS/multipath_channel.o

TOOLS_OBJS = $(TOP_DIR)/PHY/TOOLS/file_output.o
TOOLS_OBJS += $(TOP_DIR)/PHY/TOOLS/fft.o
TOOLS_OBJS += $(TOP_DIR)/PHY/TOOLS/log2_approx.o
TOOLS_OBJS += $(TOP_DIR)/PHY/TOOLS/cmult_sv.o
TOOLS_OBJS += $(TOP_DIR)/PHY/TOOLS/cdot_prod.o
TOOLS_OBJS += $(TOP_DIR)/PHY/TOOLS/signal_energy.o
TOOLS_OBJS += $(TOP_DIR)/PHY/TOOLS/dB_routines.o

OBJ = $(PHY_OBJS) $(SIMULATION_OBJS) $(TOOLS_OBJS)

all: dlsim dlsim2

$(OBJ) : %.o : %.c
	$(CC) -c $(CFLAGS) -I$(TOP_DIR) -o $@ $<

dlsim : $(OBJ) dlsim.c
	$(CC) dlsim.c -I$(TOP_DIR) -o dlsim $(CFLAGS) $(OBJ) -lm

dlsim2 : $(OBJ) dlsim2.c
	$(CC) dlsim2.c -I$(TOP_DIR) -o dlsim2 $(CFLAGS) $(OBJ) -lm

clean:
	rm -f dlsim dlsim2
	rm -f $(OBJ)
	rm -f *.o
	rm -f *.exe*

