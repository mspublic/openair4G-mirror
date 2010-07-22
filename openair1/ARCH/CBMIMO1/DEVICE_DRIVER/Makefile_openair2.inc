TOP_DIR = ../..
OPENAIR2_DIR = ../../../openair2
OPENAIR3     = ../../../openair3

CFLAGS += -DOPENAIR2

COMMON_DIR = $(OPENAIR2_DIR)/COMMON
L2_INTERFACE_DIR = $(OPENAIR2_DIR)/RRC/L2_INTERFACE
RLC_UM_DIR = $(OPENAIR2_DIR)/LAYER2/RLC/UM_v6.1.0_LITE
RLC_AM_DIR = $(OPENAIR2_DIR)/LAYER2/RLC/AM
RLC_TM_DIR = $(OPENAIR2_DIR)/LAYER2/RLC/TM
RLC_DIR = $(OPENAIR2_DIR)/LAYER2/RLC
PDCP_DIR = $(OPENAIR2_DIR)/LAYER2/PDCP
MEM_DIR = $(OPENAIR2_DIR)/UTIL/MEM
LIST_DIR = $(OPENAIR2_DIR)/UTIL/LISTS
MATH_DIR = $(OPENAIR2_DIR)/UTIL/MATH
TIMER_DIR = $(OPENAIR2_DIR)/UTIL/TIMER
PHY_INTERFACE_DIR = $(OPENAIR2_DIR)/PHY_INTERFACE
RRC_DIR = $(OPENAIR2_DIR)/RRC/MESH
L2_INTERFACE_DIR = $(OPENAIR2_DIR)/RRC/L2_INTERFACE
SIMULATION_DIR = $(OPENAIR2_DIR)/SIMULATION
CONTROL_DIR = $(OPENAIR2_DIR)/SIMULATION/PHY_EMULATION/CONTROL
ABSTRACTION_DIR = $(OPENAIR2_DIR)/SIMULATION/PHY_EMULATION/ABSTRACTION
TRANSPORT_DIR = $(OPENAIR2_DIR)/SIMULATION/PHY_EMULATION/TRANSPORT
MAC_DIR = $(OPENAIR2_DIR)/LAYER2/MAC
DEVICE_DIR = $(OPENAIR2_DIR)/SIMULATION/PHY_EMULATION/DEVICE_DRIVER
RRC_CELL_DIR = $(OPENAIR2_DIR)/RRC/CELLULAR

openair_rf-objs +=  $(LIST_DIR)/list.c
openair_rf-objs +=  $(LIST_DIR)/list2.c

openair_rf-objs +=  $(TIMER_DIR)/umts_timer.c

openair_rf-objs +=  $(PDCP_DIR)/pdcp.c
openair_rf-objs +=  $(PDCP_DIR)/pdcp_fifo.c


openair_rf-objs +=  $(RLC_AM_DIR)/rlc_am.c
openair_rf-objs +=  $(RLC_AM_DIR)/rlc_am_segment.c
openair_rf-objs +=  $(RLC_AM_DIR)/rlc_am_fsm.c
openair_rf-objs +=  $(RLC_AM_DIR)/rlc_am_mux.c
openair_rf-objs +=  $(RLC_AM_DIR)/rlc_am_retrans.c
openair_rf-objs +=  $(RLC_AM_DIR)/rlc_am_util.c
openair_rf-objs +=  $(RLC_AM_DIR)/rlc_am_mac_status.c
openair_rf-objs +=  $(RLC_AM_DIR)/rlc_am_demux.c
openair_rf-objs +=  $(RLC_AM_DIR)/rlc_am_receiver.c
openair_rf-objs +=  $(RLC_AM_DIR)/rlc_am_reassembly.c
openair_rf-objs +=  $(RLC_AM_DIR)/rlc_am_discard_rx.c
openair_rf-objs +=  $(RLC_AM_DIR)/rlc_am_control_primitives.c
openair_rf-objs +=  $(RLC_AM_DIR)/rlc_am_reset.c
openair_rf-objs +=  $(RLC_AM_DIR)/rlc_am_status.c
openair_rf-objs +=  $(RLC_AM_DIR)/rlc_am_timers.c
openair_rf-objs +=  $(RLC_AM_DIR)/rlc_am_discard_notif.c
openair_rf-objs +=  $(RLC_AM_DIR)/rlc_am_discard_tx.c

openair_rf-objs +=  $(RLC_TM_DIR)/rlc_tm.c
openair_rf-objs +=  $(RLC_TM_DIR)/rlc_tm_control_primitives.c
openair_rf-objs +=  $(RLC_TM_DIR)/rlc_tm_fsm.c
openair_rf-objs +=  $(RLC_TM_DIR)/rlc_tm_segment.c
openair_rf-objs +=  $(RLC_TM_DIR)/rlc_tm_reassembly.c

openair_rf-objs +=  $(RLC_UM_DIR)/rlc_um.c
openair_rf-objs +=  $(RLC_UM_DIR)/rlc_um_fsm.c
openair_rf-objs +=  $(RLC_UM_DIR)/rlc_um_control_primitives.c
openair_rf-objs +=  $(RLC_UM_DIR)/rlc_um_segment.c
openair_rf-objs +=  $(RLC_UM_DIR)/rlc_um_reassembly.c
openair_rf-objs +=  $(RLC_UM_DIR)/rlc_um_receiver.c

openair_rf-objs +=  $(RLC_DIR)/rlc_mac.c
openair_rf-objs +=  $(RLC_DIR)/rlc.c
openair_rf-objs +=  $(RLC_DIR)/rlc_rrc.c
openair_rf-objs +=  $(RLC_DIR)/rlc_mpls.c

openair_rf-objs +=  $(MEM_DIR)/mem_block.c

RRC_OBJS =  $(RRC_DIR)/main.o $(RRC_DIR)/L2_interface.o $(RRC_DIR)/utils.o 

PHY_INTERFACE_OBJS = $(PHY_INTERFACE_DIR)/mac_phy_primitives.o

MAC_OBJS = $(MAC_DIR)/openair2_proc.o $(MAC_DIR)/layer2_scheduler.o $(MAC_DIR)/nodeb_scheduler.o $(MAC_DIR)/ue_scheduler.o $(MAC_DIR)/main.o $(MAC_DIR)/ue_control_plane_procedures.o $(MAC_DIR)/nodeb_control_plane_procedures.o $(MAC_DIR)/lchan_interface.o $(MAC_DIR)/rar_tools.o

L2_INTERFACE_OBJS = $(L2_INTERFACE_DIR)/openair_rrc_L2_interface.o


L2_incl = -I$(OPENAIR2_DIR)  -I$(COMMON_DIR) -I$(RLC_DIR) -I$(RLC_AM_DIR) -I$(RLC_UM_DIR)  -I$(RLC_TM_DIR) -I$(PDCP_DIR) -I$(MEM_DIR) -I$(LIST_DIR) -I$(MATH_DIR) -I$(TIMER_DIR) -I$(OPENAIR3)/MESH -I$(OPENAIR3)/MESH/RRM  -DNO_RRM


$(L2_OBJS) : %.o : %.c
	$(CC) -c $(CFLAGS) -DMAC_CONTEXT=1 -DPHY_CONTEXT=1 -I$(TOP_DIR) $(L2_incl) -o $@ $<

physim : $(OBJ) $(L2_OBJS) phy_procedures_sim.c 
	$(CC)  phy_procedures_sim.c -I$(TOP_DIR) $(L2_incl) -o physim $(CFLAGS) $(OBJ) $(L2_OBJS) -lm

clean:
	rm -f dlsim dlsim2
	rm -f $(OBJ)
	rm -f $(L2_OBJS)
	rm -f *.o
	rm -f *.exe*

cleanl2:
	rm -f $(L2_OBJS)
