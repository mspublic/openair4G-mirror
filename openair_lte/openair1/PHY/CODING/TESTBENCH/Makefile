CC = $(shell if [ $$OSTYPE = "cygwin" ]; then echo "gcc-4";  else echo "gcc"; fi)

SSE3PROC = $(shell echo `grep ssse3 /proc/cpuinfo`) 
SSE4PROC = $(shell echo `grep sse4 /proc/cpuinfo`) 
CPUFLAGS = -mmmx -msse -msse2 -mssse3 -msse4 -m32 
CPUFLAGS += $(shell if [ -z $(SSE3PROC)]; then echo "" ; else echo "-mssse3"; fi)
CPUFLAGS += $(shell if [ -z $(SSE4PROC)]; then echo "" ; else echo "-msse4"; fi)


TOP_DIR                    = ../../..

CFLAGS = -Wall -O2 -fno-common -mpreferred-stack-boundary=4  -DNODE_RG -DUSER_MODE -DPC_TARGET -DPC_DSP -DNB_ANTENNAS_RX=2 -DNB_ANTENNAS_TXRX=2 -DNB_ANTENNAS_TX=2 -DMAX_MODULES=1 -DPHY_CONTEXT=1 -DBIT8_TXMUX -DBIT8_TX -DOPENAIR_LTE -g $(CPUFLAGS)

PHY_OBJS +=$(TOP_DIR)/PHY/TOOLS/fft.o $(TOP_DIR)/PHY/TOOLS/cdot_prod.o $(TOP_DIR)/SIMULATION/TOOLS/rangen_double.o $(TOP_DIR)/SIMULATION/TOOLS/taus.o $(TOP_DIR)/PHY/TOOLS/file_output.o $(TOP_DIR)/PHY/INIT/lte_init.o $(TOP_DIR)/PHY/INIT/init_top.o $(TOP_DIR)/PHY/LTE_REFSIG/lte_gold.o $(TOP_DIR)/PHY/LTE_TRANSPORT/dlsch_coding.o $(TOP_DIR)/PHY/LTE_TRANSPORT/dlsch_modulation.o $(TOP_DIR)/PHY/LTE_TRANSPORT/dlsch_decoding.o $(TOP_DIR)/PHY/CODING/lte_rate_matching.o $(TOP_DIR)/PHY/CODING/lte_segmentation.o $(TOP_DIR)/PHY/LTE_ESTIMATION/lte_sync_time.o
PHY_OBJS += $(TOP_DIR)/PHY/CODING/viterbi.o $(TOP_DIR)/PHY/CODING/3gpplte_turbo_decoder_sse.o $(TOP_DIR)/PHY/CODING/3gpplte.o $(TOP_DIR)/PHY/CODING/ccoding_byte.o $(TOP_DIR)/PHY/CODING/rate_matching.o $(TOP_DIR)/PHY/CODING/crc_byte.o 
PHY_OBJS += $(TOP_DIR)/PHY/TOOLS/log2_approx.o

#OPENAIR0_OBJS = $(OPENAIR0_DIR)/express-mimo/simulator/baseband/C/channel_decoder/decoder.o $(OPENAIR0_DIR)/express-mimo/simulator/baseband/C/channel_decoder/decoder_api.o $(OPENAIR0_DIR)/express-mimo/simulator/baseband/C/channel_decoder/3gpp_intlv.o $(OPENAIR0_DIR)/express-mimo/simulator/baseband/C/channel_decoder/3gpp_encoder.o $(OPENAIR0_DIR)/express-mimo/simulator/baseband/C/channel_decoder/lte_intlv.o

OBJ = $(PHY_OBJS) #$(OPENAIR0_OBJS)


all: ltetest

#$(OBJ) : %.o : %.c 
#	$(CC) -c -O2 $(CFLAGS) -Wall -I$(TOP_DIR) -I$(TOP_DIR)/PHY/CODING -I$(OPENAIR0_DIR)/express-mimo/simulator/baseband/C/channel_decoder -o $@ $<

$(OBJ) : %.o : %.c
	$(CC) -c -O2 $(CFLAGS) -Wall -I$(TOP_DIR) -I$(TOP_DIR)/PHY/CODING -I$(OPENAIR0_DIR)/express-mimo/simulator/baseband/C/channel_decoder -o $@ $<

ltetest: $(OBJ) ltetest.c
	$(CC) ltetest.c -O2 -I$(TOP_DIR) -I$(TOP_DIR)/PHY/CODING -I$(OPENAIR0_DIR)/express-mimo/simulator/baseband/C/channel_decoder -o ltetest $(CFLAGS) $(OBJ) -lm -DOPENAIR_LTE

viterbi_test: $(OBJ) viterbi_test.c
	$(CC) viterbi_test.c -I$(TOP_DIR) -o viterbi_test $(CFLAGS) $(PHY_OBJS) -lm

clean: 
	rm -f $(TOP_DIR)/PHY/CODING/*.o
	rm -f $(TOP_DIR)/SIMULATION/TOOLS/*.o
	rm -f ltetest
	rm -f *.m

