CC = gcc

TOP_DIR = ../..

CFLAGS = -Wall -g -ggdb -DNODE_RG -DUSER_MODE -DNB_ANTENNAS_RX=2 -DNB_ANTENNAS_TX=2 -DNB_ANTENNAS_TXRX=2 -DWIDENS_DLC -DPHY_CONTEXT -DPC_DSP -DPC_TARGET -DMAX_MODULES=1 -DCBMIMO1 -mtune=pentium4 -mmmx -msse2 -O -funroll-loops -I$(TOP_DIR) -I/usr/include/X11 -I/usr/X11R6/include

COMMON_OBJS = $(TOP_DIR)/PHY/CONFIG/openair_configure.o $(TOP_DIR)/PHY/CONFIG/openair_readconfigfile.o 
CHBCH_OBJS = chbch_scope.o chbch.o 
SACH_OBJS = sach_scope.o sach.o 
#CHBCH2_OBJS = chbch_2streams_scope.o chbch_2streams.o 
CHBCH2_OBJS = chbch_scope.o chbch_2streams_ml.o 
LTE_OBJS = lte_scope.o lte.o $(TOP_DIR)/PHY/TOOLS/file_output.o

ifdef EMOS
CFLAGS += -DEMOS
endif

all: chbch_scope

chbch: $(CHBCH_OBJS) $(COMMON_OBJS) 
	$(CC)  -o chbch_scope $(CFLAGS) $(CHBCH_OBJS) $(COMMON_OBJS) -lm -L/usr/local/lib -lforms -lX11 -L/usr/X11R6/lib -lXpm -lpthread  -fno-stack-check 

chbch2: $(CHBCH2_OBJS) $(COMMON_OBJS) 
	$(CC)  -o chbch2_scope $(CFLAGS) $(CHBCH2_OBJS) $(COMMON_OBJS) -lm -L/usr/local/lib -lforms -lX11 -L/usr/X11R6/lib -lXpm -lpthread  -fno-stack-check 

sach: $(SACH_OBJS) $(COMMON_OBJS) 
	$(CC)  -o sach_scope $(CFLAGS) $(SACH_OBJS) $(COMMON_OBJS) -lm -L/usr/local/lib -lforms -lX11 -L/usr/X11R6/lib -lXpm -lpthread  -fno-stack-check 

lte: $(LTE_OBJS) 
	$(CC)  -o lte_scope $(CFLAGS) $(LTE_OBJS)  -lm -L/usr/local/lib -lforms -lX11 -L/usr/X11R6/lib -lXpm -lpthread  -fno-stack-check -DOPENAIR_LTE

$(CHBCH_OBJS) $(CHBCH2_OBJS) $(SACH_OBJS) $(COMMON_OBJS) : %.o: %.c 
	$(CC) -c $(DEBUG_FLAGS) $(CFLAGS) $(INCLUDE) -o $@ $<

$(LTE_OBJS) : %.o: %.c
	$(CC) -c $(DEBUG_FLAGS) $(CFLAGS) -DOPENAIR_LTE $(INCLUDE) -o $@ $<

clean:
	rm -f $(CHBCH_OBJS) $(CHBCH2_OBJS) $(SACH_OBJS) $(COMMON_OBJS) $(LTE_OBJS)


