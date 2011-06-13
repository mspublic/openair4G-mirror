RRC_CELLULAR_OBJS = $(RRC_CELL_DIR)/rrc_L2_interface.o
ifdef eNB_flag
RRC_CELLULAR_OBJS += $(RRC_CELL_DIR)/rrc_rg_init.o
RRC_CELLULAR_OBJS += $(RRC_CELL_DIR)/rrc_rg_main.o
EXTRA_CFLAGS += -DNODE_RG -DCELLULAR
endif

ifdef UE_flag
RRC_CELLULAR_OBJS += $(RRC_CELL_DIR)/rrc_ue_init.o
RRC_CELLULAR_OBJS += $(RRC_CELL_DIR)/rrc_ue_main.o
EXTRA_CFLAGS += -DNODE_MT -DCELLULAR
endif

rrc_test:
	@echo RRC makefile is in
	@echo RRC cellular variable $(rrc_cellular)
	@echo $(L2_incl)