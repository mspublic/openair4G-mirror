userclean:
	(cd $(OPENAIR2_DIR) && make clean nasmesh_netlink_address_fix.ko mac_sim)
userclean_rrm:
	(cd $(OPENAIR2_DIR) && make clean nasmesh_netlink_address_fix.ko mac_sim_rrm)
kernclean:
	(cd $(OPENAIR2_DIR) && make clean nasmesh_address_fix.ko openair_rrc_emul.ko openair_emul.ko openair_layer2_emul.ko rt_emul rb_tool)


user:
	(cd $(OPENAIR2_DIR) && make nasmesh_netlink_address_fix.ko mac_sim rb_tool)
user_rrm:
	(cd $(OPENAIR2_DIR) && make nasmesh_netlink_address_fix.ko mac_sim_rrm rb_tool)
kern:
	(cd $(OPENAIR2_DIR) && make nasmesh_address_fix.ko openair_rrc_emul.ko openair_emul.ko openair_layer2_emul.ko rt_emul rb_tool)

kern_rrm:	
	(cd $(OPENAIR2_DIR) && make nasmesh_address_fix.ko openair_rrc_emul_rrm.ko openair_emul_rrm.ko openair_layer2_emul_rrm.ko rt_emul rb_tool)