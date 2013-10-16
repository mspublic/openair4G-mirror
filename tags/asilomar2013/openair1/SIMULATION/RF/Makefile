OBJS = adc.o dac.o rf.o ../../PHY/TOOLS/file_output.o  ../../PHY/TOOLS/signal_energy.o ../TOOLS/rangen_double.o ../TOOLS/random_channel.o ../TOOLS/multipath_channel.o ../TOOLS/taus.o ../../../openair2/UTIL/LOG/log.o ../../../openair2/UTIL/LOG/vcd_signal_dumper.o
CFLAGS = -DRF_MAIN -DUSER_MODE -DDEBUG_PHY -DNB_ANTENNAS_RX=2 -DNB_ANTENNAS_TX=2
CFLAGS += -O2 -Wall -g -mmmx -msse -msse2 -mssse3 
CFLAGS += -I$(OPENAIR1_DIR) -I$(OPENAIR2_DIR)

all: $(OBJS)
	gcc -o rf $(OBJS) -lm -lblas

$(OBJ) : %.o : %.c 
	$(CC) -c $(CFLAGS) -Wall -I$(TOP_DIR) -o $@ $<

clean:
	rm -f $(OBJS)
