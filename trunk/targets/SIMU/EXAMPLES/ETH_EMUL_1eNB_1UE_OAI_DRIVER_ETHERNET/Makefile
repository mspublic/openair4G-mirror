all: oaisim naslite_netlink_ether

oaisim:
	(cd $(OPENAIR_TARGETS)/SIMU/USER && make oaisim NAS_DRIVER_TYPE_ETHERNET=1 NAS_NETLINK=1)
naslite_netlink_ether:
	(cd $(OPENAIR2_DIR) && make naslite_netlink_ether.ko)
	(cd $(OPENAIR2_DIR)/NAS/DRIVER/LITE/RB_TOOL/ && make)

one_eNB_one_UE: oaisim naslite_netlink_ether

userclean: clean oaisim naslite_netlink_ether

clean:
	(cd $(OPENAIR_TARGETS)/SIMU/USER && make clean)
	(cd $(OPENAIR2_DIR)/NAS/DRIVER/LITE && make clean)
