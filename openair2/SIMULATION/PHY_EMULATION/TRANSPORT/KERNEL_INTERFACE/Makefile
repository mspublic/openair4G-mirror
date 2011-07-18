ifndef FOR_ECOS
CCC = gcc

#TOP_DIR=/homes/knopp/Devel/openair/trunk/openair2
TOP_DIR=../../../..
CFLAGS = -DUSER_MODE  -g -mtune=pentium4 -mmmx -msse2 -O -funroll-loops -I$(TOP_DIR) -I$(TOP_DIR)/COMMON -DNB_ANTENNAS=2 -DNODE_RG 
#-DICC 



# CONFIG routines
SIMULATION_OBJS = $(TOP_DIR)/SIMULATION/PHY_EMULATION/TRANSPORT/multicast_link.o $(TOP_DIR)/SIMULATION/PHY_EMULATION/TRANSPORT/socket.o


OBJ = $(SIMULATION_OBJS)

all:	bypass_phy

$(OBJ) : %.o : %.c 
	$(CC) -c $(CFLAGS) -o $@ $< -pthread

bypass_phy : $(OBJ) bypass_phy.c 
	$(CC)  bypass_phy.c -o bypass_phy $(CFLAGS) $(OBJ) -lm -lrt -pthread


else  # for ECOS

CCC = gcc
TOP_DIR = $(OPENAIR2_DIR)
include $(ECOS_INSTALL_DIR)/include/pkgconf/ecos.mak

LDFLAGS = -nostartfiles -L$(ECOS_INSTALL_DIR)/lib -Ttarget.ld
CFLAGS = -I$(ECOS_INSTALL_DIR)/include -DECOS -I$(TOP_DIR) -I$(TOP_DIR)/COMMON
CFLAGS += -DUSER_MODE -I$(TOP_DIR) -I$(TOP_DIR)/COMMON -DNB_ANTENNAS=2 -DNODE_RG 



# CONFIG routines
SIMULATION_OBJS = $(TOP_DIR)/SIMULATION/PHY_EMULATION/TRANSPORT/multicast_link.o $(TOP_DIR)/SIMULATION/PHY_EMULATION/TRANSPORT/socket.o

OBJ = $(SIMULATION_OBJS)

all:	bypass_phy

$(OBJ) : %.o: %.c  
	$(CCC) -c $(CFLAGS)$(ECOS_GLOBAL_CFLAGS) -o $@ $< 

bypass_phy: $(OBJ) bypass_phy.c
	$(CCC) bypass_phy.c -o bypass_phy $(CFLAGS) $(OBJ) $(LDFLAGS)  $(ECOS_GLOBAL_LDFLAGS) 


endif


clean : 
	rm -f $(OBJ)	
	rm -f bypass_phy.o bypass_phy

