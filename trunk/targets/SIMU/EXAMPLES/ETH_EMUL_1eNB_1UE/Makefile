clean: 
	(cd $(OPENAIR_TARGETS)/SIMU/USER && make clean)

oaisim: 
	(cd $(OPENAIR_TARGETS)/SIMU/USER && make NAS_NETLINK=1)
nasmesh:
	(cd $(OPENAIR2_DIR) && make nasmesh_netlink.ko)
	(cd $(OPENAIR2_DIR)/NAS/DRIVER/MESH/RB_TOOL/ && make)

one_eNB_two_UE: oaisim nasmesh

userclean: clean oaisim nasmesh	

