CC = gcc

TOP_DIR = ../..
OPENAIR2_DIR = ../../../../openair2
OPENAIR2_TOP = $(OPENAIR2_DIR)

CFLAGS = -DNODE_RG -DUSER_MODE -DPC_TARGET -DPC_DSP -DMAX_MODULES=1 -DPHY_CONTEXT=1 -DDEBUG_PHY -DCBMIMO1 -DNB_ANTENNAS_RX=2 -DNB_ANTENNAS_TX=2 -DNB_ANTENNAS_TXRX=2 -DOPEN_OUTPUT_FILE  -DIFFT_FLAG -I$(OPENAIR1_DIR) -I$(OPENAIR2_DIR)/RRC/LITE/MESSAGES

include $(OPENAIR2_DIR)/LAYER2/Makefile.inc
CFLAGS += $(L2_incl)

#ifdef OPENAIR_LTE
CFLAGS += -DOPENAIR_LTE
#endif

XTRA_CFLAGS = -msse -msse2 -mssse3 

OPENAIROBJS +=$(OPENAIR1_DIR)/PHY/INIT/init_top.o 
OPENAIROBJS +=$(OPENAIR1_DIR)/PHY/TOOLS/fft.o 

#OPENAIROBJS = $(OPENAIR1_DIR)/PHY/CONFIG/openair_readconfigfile.o $(OPENAIR1_DIR)/PHY/CONFIG/openair_configure.o 

OCTAVEOBJS =  oarf_set_tcxo_dac.oct oarf_config.oct oarf_set_calibrated_rx_gain.oct oarf_set_tx_gain.oct oarf_get_frame.oct oarf_set_rx_gain.oct  oarf_set_rx_rfmode.oct oarf_stop.oct oarf_send_frame.oct

all: $(OCTAVEOBJS) gpib_send.oct 

oarf: $(OCTAVEOBJS)

$(OPENAIROBJS) : %.o : %.c
	gcc -c $(CFLAGS) $(XTRA_CFLAGS) -o $@ $<

test:
	echo $(OPENAIROBJS)

$(OCTAVEOBJS) : %.oct : %.cc $(OPENAIROBJS) 
	mkoctfile -o $@ $(CFLAGS) -lm $(OPENAIROBJS) $<

gpib_send.oct:	gpib_send.cc
	mkoctfile -I/usr/include/gpib -I/usr/local/include/gpib -lgpib gpib_send.cc

clean:
	rm -f gpib_send.oct $(OCTAVEOBJS) $(OPENAIROBJS)
