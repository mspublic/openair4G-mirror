CFLAGS = -DNODE_RG -DUSER_MODE -DPC_TARGET -DPC_DSP -DMAX_MODULES=1 -DPHY_CONTEXT=1 -DDEBUG_PHY -DCBMIMO1 -DNB_ANTENNAS_RX=2 -DNB_ANTENNAS_TX=2 -DNB_ANTENNAS_TXRX=2 -DOPEN_OUTPUT_FILE  -DBIT8_TXMUX -I$(OPENAIR1_DIR)

ifdef OPENAIR_LTE
CFLAGS += -DOPENAIR_LTE
endif

XTRA_CFLAGS = -msse -msse2 -mssse3 

OPENAIROBJS = $(OPENAIR1_DIR)/PHY/CONFIG/openair_readconfigfile.o $(OPENAIR1_DIR)/PHY/CONFIG/openair_configure.o $(OPENAIR1_DIR)/PHY/TOOLS/8bit_txmux.o

OCTAVEOBJS = oarf_send_frame.oct oarf_set_tcxo_dac.oct oarf_config.oct oarf_set_calibrated_rx_gain.oct oarf_set_tx_gain.oct oarf_get_frame.oct oarf_set_rx_gain.oct  oarf_set_rx_rfmode.oct oarf_stop.oct
# oarf_get_rtframe.oct

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
