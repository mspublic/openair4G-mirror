ifdef cygwin
cyg=1
endif

oaisim: 
	(cd $(OPENAIR_TARGETS)/SIMU/USER && make cygwin=$(cyg))
nasmesh:
	(cd $(OPENAIR2_DIR)/NAS/DRIVER/MESH/ && make)
	(cd $(OPENAIR2_DIR)/NAS/DRIVER/MESH/RB_TOOL/rb_tool && make)

one_eNB_two_UE_nas: oaisim nasmesh

one_eNB_one_UE : oaisim
	../../USER/oaisim -n1000 | egrep SEND_SDU

one_eNB_four_UE : oaisim
	../../USER/oaisim -n60 -u4 | egrep RRC	

clean:
	(cd $(OPENAIR_TARGETS)/SIMU/USER && make clean)
