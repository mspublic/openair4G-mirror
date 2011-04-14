CCC = gcc

TOP_DIR = ../../..

CFLAGS = -DUSER_MODE  -g  -mmmx -msse2 -O -funroll-loops -I$(TOP_DIR) -DNB_ANTENNAS=2 -DNODE_RG -DPHY_EMUL
#-DICC 



# CONFIG routines
SIMULATION_OBJS = $(TOP_DIR)/SIMULATION/openair_configure.o $(TOP_DIR)/SIMULATION/openair_readconfigfile.o 



OBJ = $(SIMULATION_OBJS)

#all:	rt_emul

$(OBJ) : %.o : %.c 
	$(CC) -c $(CFLAGS)  -o $@ $< -pthread

rt_emul : $(OBJ) rt_emul.c 
	$(CC)  rt_emul.c -o rt_emul $(CFLAGS) $(OBJ) -lm -lrt -pthread


clean : 
	rm rt_emul
	rm $(SIMULATION_OBJS)
