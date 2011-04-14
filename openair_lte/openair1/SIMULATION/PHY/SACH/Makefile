CC = gcc

TOP_DIR                    = ../../..

CFLAGS = -Wall -fno-common -mpreferred-stack-boundary=4 -DNODE_RG -DUSER_MODE -DPC_TARGET -DPC_DSP -DNB_ANTENNAS_RX=2 -DNB_ANTENNAS_TX=2 -DNB_ANTENNAS_TXRX=2 -DMAX_MODULES=1 -DPHY_CONTEXT=1 -g -msse2 -mmmx -msse
#-O -funroll-loops -mtune=pentium4 
#-DICC 

ifdef DEBUGPHY
CFLAGS += -DDEBUG_PHY
endif


# PHY Initialization routines
PHY_OBJS	=$(TOP_DIR)/PHY/INIT/init.o 

# PHY TOOLS routines
PHY_OBJS +=$(TOP_DIR)/PHY/TOOLS/fft.o $(TOP_DIR)/PHY/TOOLS/cmult_vv.o $(TOP_DIR)/PHY/TOOLS/cmult_sv.o $(TOP_DIR)/PHY/TOOLS/cmult_vvh.o $(TOP_DIR)/PHY/TOOLS/cmult_mm.o $(TOP_DIR)/PHY/TOOLS/cadd_sv.o $(TOP_DIR)/PHY/TOOLS/cadd_vv.o $(TOP_DIR)/PHY/TOOLS/8bit_txmux.o


# PHY ESTIMATION routines 
PHY_OBJS += $(TOP_DIR)/PHY/ESTIMATION/channel_estimation.o 

# PHY MODULATION routines
PHY_OBJS +=$(TOP_DIR)/PHY/MODULATION/ofdm_mod.o

# PHY Transport routines
PHY_OBJS += $(TOP_DIR)/PHY/TRANSPORT/chsch.o $(TOP_DIR)/PHY/TRANSPORT/chbch.o $(TOP_DIR)/PHY/TRANSPORT/sch.o $(TOP_DIR)/PHY/TRANSPORT/sach.o $(TOP_DIR)/PHY/TRANSPORT/mrbch.o $(TOP_DIR)/PHY/TRANSPORT/tools.o


# PHY Coding routines
PHY_OBJS+=$(TOP_DIR)/PHY/CODING/ccoding_byte.o $(TOP_DIR)/PHY/CODING/crc_byte.o $(TOP_DIR)/PHY/CODING/viterbi.o $(TOP_DIR)/PHY/CODING/rate_matching.o

# PHY Synch routines
PHY_OBJS +=$(TOP_DIR)/PHY/ESTIMATION/synch_time.o 

# Utility routines
PHY_OBJS +=$(TOP_DIR)/PHY/TOOLS/dB_routines.o $(TOP_DIR)/PHY/TOOLS/file_output.o $(TOP_DIR)/PHY/TOOLS/memory_routines.o $(TOP_DIR)/PHY/TOOLS/signal_energy.o $(TOP_DIR)/PHY/TOOLS/sqrt.o $(TOP_DIR)/PHY/TOOLS/log2_approx.o $(TOP_DIR)/SIMULATION/TOOLS/rangen_double.o $(TOP_DIR)/SIMULATION/TOOLS/random_channel.o 

# CONFIG routines
PHY_OBJS +=$(TOP_DIR)/PHY/CONFIG/openair_configure.o $(TOP_DIR)/PHY/CONFIG/openair_readconfigfile.o

OBJ = $(PHY_OBJS) 

all: sach_sim 

$(OBJ) : %.o : %.c 
	$(CC) -c $(CFLAGS) -Wall -I$(TOP_DIR) -o $@ $<

sach_sim : $(OBJ) sach_sim.c
	$(CC)  sach_sim.c -I$(TOP_DIR) -o sach_sim $(CFLAGS) $(PHY_OBJS) -lm 

clean:
	rm -f sach_sim
	rm -f *.o
	rm -f *.exe*
	rm -f $(TOP_DIR)/PHY/INIT/*.o
	rm -f $(TOP_DIR)/PHY/TOOLS/*.o
	rm -f $(TOP_DIR)/PHY/MODULATION/*.o
	rm -f $(TOP_DIR)/PHY/CODING/*.o
	rm -f $(TOP_DIR)/PHY/TRANSPORT/*.o
	rm -f $(TOP_DIR)/PHY/ESTIMATION/*.o
	rm -f $(TOP_DIR)/PHY/CONFIG/*.o
	rm -f $(TOP_DIR)/SIMULATION/TOOLS/*.o
	rm -f *.m


