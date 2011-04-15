#ifdef cygwin
#cyg=cygwin
#endif

help:
	@echo make one_eNB_two_UE_nas compiles the code and performs a simulation with IP traffic between two virtual nodes (Linux only)
	@echo make one_eNB_one_UE compiles the code and performs a small simulation (30 frames) with 1 eNB and 1 UE and checks that RLC data is transported between the two nodes after the connection setup 
	@echo make one_eNB_four_UE comples the code and performs a small simulation (60 frames) with 1 eNB and 4 UE and checks that all 4 UEs have established a complete connection (RRC output is traced)
	@echo make clean Removes all .o files

all: help

oaisim: 
	(cd $(OPENAIR_TARGETS)/SIMU/USER && make ) 
nasmesh:
	(cd $(OPENAIR2_DIR)/NAS/DRIVER/MESH/ && make)
	(cd $(OPENAIR2_DIR)/NAS/DRIVER/MESH/RB_TOOL/rb_tool && make)

one_eNB_two_UE_nas: oaisim nasmesh

one_eNB_one_UE : oaisim
	../../USER/oaisim -n30 | egrep SEND_SDU

one_eNB_four_UE : oaisim
	../../USER/oaisim -n60 -u4 | egrep "\[RRC\]"	

clean:
	(cd $(OPENAIR_TARGETS)/SIMU/USER && make clean)
