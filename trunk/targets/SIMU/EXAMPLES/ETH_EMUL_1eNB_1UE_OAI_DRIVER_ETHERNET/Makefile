all: oaisim nasmesh_netlink_ether

oaisim:
	(cd $(OPENAIR_TARGETS)/SIMU/USER && make oaisim NAS_NETLINK=1)
nasmesh_netlink_ether:
	(cd $(OPENAIR2_DIR) && make nasmesh_netlink_ether.ko)
	(cd $(OPENAIR2_DIR)/NAS/DRIVER/MESH/RB_TOOL/ && make)

one_eNB_one_UE: oaisim nasmesh_netlink_ether

userclean: clean oaisim nasmesh_netlink_ether

clean:
	(cd $(OPENAIR_TARGETS)/SIMU/USER && make clean)
	(cd $(OPENAIR2_DIR)/NAS/DRIVER/MESH && make clean)
