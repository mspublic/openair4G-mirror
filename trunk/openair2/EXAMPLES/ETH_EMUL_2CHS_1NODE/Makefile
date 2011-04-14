
userclean:

	(cd $(OPENAIR2_DIR) && make clean nasmesh_netlink.ko mac_sim rb_tool)
kernclean:
	(cd $(OPENAIR2_DIR) && make clean nasmesh.ko openair_rrc.ko openair_emul.ko openair_layer2_emul.ko emul_rt bypass_phy rb_tool)

user:
	(cd $(OPENAIR2_DIR) && make nasmesh_netlink.ko mac_sim rb_tool)
kern:
	(cd $(OPENAIR2_DIR) && make nasmesh.ko openair_rrc.ko openair_emul.ko openair_layer2_emul.ko emul_rt bypass_phy rb_tool)



