CCC = gcc

TOP_DIR = ../..

CFLAGS = -DNODE_RG -DUSER_MODE -DPC_TARGET -DPC_DSP -DMAX_MODULES=1 -DPHY_CONTEXT=1  -g -mtune=pentium4 -mmmx -msse2 -mssse3 -msse4 -O -funroll-loops -I$(TOP_DIR) -I/usr/realtime/include/ 
#-DICC 

ifdef DEBUGPHY
CFLAGS += -DDEBUG_PHY
endif

ifdef PLATON
CFLAGS += -DPLATON -DNB_ANTENNAS=2 -DNB_ANTENNAS_TX=4 
FNAME = platon
endif

ifdef PHY_EMUL 
CFLAGS += -DPHY_EMUL_IOCTL
endif

ifdef CBMIMO1
CFLAGS += -DHW_PREFIX_REMOVAL -DCBMIMO1 -DNB_ANTENNAS_RX=2 -DOPEN_OUTPUT_FILE 
endif

ifdef OPENAIR_LTE
FNAME = cbmimo1_lte
else
FNAME = cbmimo1
endif

ifdef OPENAIR_LTE
CFLAGS += -DOPENAIR_LTE -DIFFT_FPGA
endif

ifdef IDROMEL_RF
CFLAGS += -DNB_ANTENNAS_RX=2 -DNB_ANTENNAS_TX=1 -DNB_ANTENNAS_TXRX=2
else
CFLAGS += -DNB_ANTENNAS_RX=2 -DNB_ANTENNAS_TX=2 -DNB_ANTENNAS_TXRX=2 -DBIT8_TXMUX=1 -DBIT8_TX=1	
endif

ifdef EMOS
CFLAGS += -DEMOS
endif

# PHY Initialization routines
ifndef OPENAIR_LTE
PHY_OBJS        +=$(TOP_DIR)/PHY/INIT/init.o
else
PHY_OBJS	+=$(TOP_DIR)/PHY/INIT/init_top.o 
PHY_OBJS	+=$(TOP_DIR)/PHY/INIT/lte_init.o 
endif

# PHY TOOLS routines
PHY_OBJS +=$(TOP_DIR)/PHY/TOOLS/fft.o $(TOP_DIR)/PHY/TOOLS/cmult_vv.o $(TOP_DIR)/PHY/TOOLS/cmult_sv.o $(TOP_DIR)/PHY/TOOLS/cmult_mm.o $(TOP_DIR)/PHY/TOOLS/cmult_vvh.o $(TOP_DIR)/PHY/TOOLS/cadd_vv.o 
#ifdef CBMIMO1
#PHY_OBJS +=$(TOP_DIR)/PHY/TOOLS/8bit_txmux.o
#endif

# PHY ESTIMATION routines 
ifndef OPENAIR_LTE
PHY_OBJS += $(TOP_DIR)/PHY/ESTIMATION/channel_estimation.o 
else

endif

# PHY MODULATION routines
PHY_OBJS +=$(TOP_DIR)/PHY/MODULATION/ofdm_mod.o

# PHY Transport routines
ifndef OPENAIR_LTE
PHY_OBJS += $(TOP_DIR)/PHY/TRANSPORT/chsch.o $(TOP_DIR)/PHY/TRANSPORT/chbch.o $(TOP_DIR)/PHY/TRANSPORT/sch.o $(TOP_DIR)/PHY/TRANSPORT/sach.o $(TOP_DIR)/PHY/TRANSPORT/mrbch.o
else
PHY_OBJS += $(TOP_DIR)/PHY/LTE_TRANSPORT/pss.o
PHY_OBJS += $(TOP_DIR)/PHY/LTE_TRANSPORT/pilots.o
PHY_OBJS += $(TOP_DIR)/PHY/LTE_TRANSPORT/pbch.o
PHY_OBJS += $(TOP_DIR)/PHY/LTE_TRANSPORT/dlsch_modulation.o
PHY_OBJS += $(TOP_DIR)/PHY/LTE_TRANSPORT/srs_modulation.o
PHY_OBJS += $(TOP_DIR)/PHY/LTE_REFSIG/lte_dl_cell_spec.o
PHY_OBJS += $(TOP_DIR)/PHY/LTE_REFSIG/lte_gold.o
PHY_OBJS += $(TOP_DIR)/PHY/LTE_REFSIG/lte_ul_ref.o
PHY_OBJS += $(TOP_DIR)/PHY/LTE_TRANSPORT/lte_mcs.o
endif

# PHY Coding routines
PHY_OBJS+=$(TOP_DIR)/PHY/CODING/ccoding_byte.o $(TOP_DIR)/PHY/CODING/crc_byte.o $(TOP_DIR)/PHY/CODING/viterbi.o $(TOP_DIR)/PHY/CODING/rate_matching.o $(TOP_DIR)/PHY/CODING/3gpplte.o $(TOP_DIR)/PHY/CODING/3gpplte_turbo_decoder_sse.o
PHY_OBJS += $(TOP_DIR)/PHY/CODING/ccoding_byte_lte.o
PHY_OBJS += $(TOP_DIR)/PHY/CODING/lte_rate_matching.o
PHY_OBJS += $(TOP_DIR)/PHY/CODING/viterbi_lte.o

# PHY Synch routines
ifndef OPENAIR_LTE
PHY_OBJS +=$(TOP_DIR)/PHY/ESTIMATION/synch_time.o 
else
PHY_OBJS +=$(TOP_DIR)/PHY/LTE_ESTIMATION/lte_sync_time.o 
endif

# Utility routines
PHY_OBJS +=$(TOP_DIR)/PHY/TOOLS/dB_routines.o $(TOP_DIR)/PHY/TOOLS/file_output.o $(TOP_DIR)/PHY/TOOLS/memory_routines.o $(TOP_DIR)/PHY/TOOLS/signal_energy.o $(TOP_DIR)/PHY/TOOLS/sqrt.o $(TOP_DIR)/PHY/TOOLS/log2_approx.o $(TOP_DIR)/PHY/TOOLS/cdot_prod.o

PHY_OBJS +=$(TOP_DIR)/SIMULATION/TOOLS/rangen_double.o $(TOP_DIR)/SIMULATION/TOOLS/random_channel.o 

# CONFIG routines
PHY_OBJS +=$(TOP_DIR)/PHY/CONFIG/openair_configure.o $(TOP_DIR)/PHY/CONFIG/openair_readconfigfile.o

#PHY_OBJS +=$(TOP_DIR)/ARCH/CBMIMO1/DEVICE_DRIVER/cbmimo1_generate_ofdm.o


USERRFOBJS += $(PHY_OBJS) openair_rf.o openair_generate_ofdm.o
#load.o fix_fft2.o load_calibration_data.o 


all:	openair_rf

$(USERRFOBJS) : %.o : %.c
	$(CCC) -c $(CFLAGS) -o $@ $<

openair_rf: $(USERRFOBJS)
	$(CCC) -o openair_rf_$(FNAME) $(CFLAGS) $(USERRFOBJS) -lm  


clean:
	rm -f $(USERRFOBJS)


