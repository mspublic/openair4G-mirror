
#/*________________________mac_init.c________________________

# Authors : Hicham Anouar, Raymond Knopp
# Company : EURECOM
# Emails  : anouar@eurecom.fr,  knopp@eurecom.fr
#________________________________________________________________*/

CC = gcc

TOPDIR = ../../..
OPENAIR3=../../../../openair3

COMMON_DIR = $(TOPDIR)/COMMON
L2_INTERFACE_DIR = $(TOPDIR)/RRC/L2_INTERFACE
RLC_UM_DIR = $(TOPDIR)/LAYER2/RLC/UM_v6.1.0_LITE
RLC_AM_DIR = $(TOPDIR)/LAYER2/RLC/AM
RLC_TM_DIR = $(TOPDIR)/LAYER2/RLC/TM
RLC_DIR = $(TOPDIR)/LAYER2/RLC
PDCP_DIR = $(TOPDIR)/LAYER2/PDCP
MEM_DIR = $(TOPDIR)/UTIL/MEM
LIST_DIR = $(TOPDIR)/UTIL/LISTS
MATH_DIR = $(TOPDIR)/UTIL/MATH
TIMER_DIR = $(TOPDIR)/UTIL/TIMER
PHY_INTERFACE_DIR = $(TOPDIR)/PHY_INTERFACE
RRC_DIR = $(TOPDIR)/RRC/MESH
L2_INTERFACE_DIR = $(TOPDIR)/RRC/L2_INTERFACE
SIMULATION_DIR = $(TOPDIR)/SIMULATION
CONTROL_DIR = $(TOPDIR)/SIMULATION/PHY_EMULATION/CONTROL
ABSTRACTION_DIR = $(TOPDIR)/SIMULATION/PHY_EMULATION/ABSTRACTION
TRANSPORT_DIR = $(TOPDIR)/SIMULATION/PHY_EMULATION/TRANSPORT
MAC_DIR = $(TOPDIR)/LAYER2/MAC
DEVICE_DIR = $(TOPDIR)/SIMULATION/PHY_EMULATION/DEVICE_DRIVER
RRC_CELL_DIR = $(TOPDIR)/RRC/CELLULAR




export RLC_UM_DIR
export RLC_AM_DIR
export RLC_TM_DIR
export RLC_DIR
export PDCP_DIR
export MEM_DIR
export LIST_DIR
export MATH_DIR
export TIMER_DIR
export PHY_INTERFACE_DIR
export MAC_DIR
export RRC_DIR
export L2_INTERFACE_DIR
export SIMULATION_DIR
export CONTROL_DIR
export ABSTARCTION_DIR
export TRANSPORT_DIR
export DEVICE_DIR

SOURCES_L2 +=  $(LIST_DIR)/list.c
SOURCES_L2 +=  $(LIST_DIR)/list2.c

SOURCES_L2 +=  $(TIMER_DIR)/umts_timer.c

SOURCES_L2 +=  $(PDCP_DIR)/pdcp.c
SOURCES_L2 +=  $(PDCP_DIR)/pdcp_fifo.c


SOURCES_L2 +=  $(RLC_AM_DIR)/rlc_am.c
SOURCES_L2 +=  $(RLC_AM_DIR)/rlc_am_segment.c
SOURCES_L2 +=  $(RLC_AM_DIR)/rlc_am_fsm.c
SOURCES_L2 +=  $(RLC_AM_DIR)/rlc_am_mux.c
SOURCES_L2 +=  $(RLC_AM_DIR)/rlc_am_retrans.c
SOURCES_L2 +=  $(RLC_AM_DIR)/rlc_am_util.c
SOURCES_L2 +=  $(RLC_AM_DIR)/rlc_am_mac_status.c
SOURCES_L2 +=  $(RLC_AM_DIR)/rlc_am_demux.c
SOURCES_L2 +=  $(RLC_AM_DIR)/rlc_am_receiver.c
SOURCES_L2 +=  $(RLC_AM_DIR)/rlc_am_reassembly.c
SOURCES_L2 +=  $(RLC_AM_DIR)/rlc_am_discard_rx.c
SOURCES_L2 +=  $(RLC_AM_DIR)/rlc_am_control_primitives.c
SOURCES_L2 +=  $(RLC_AM_DIR)/rlc_am_reset.c
SOURCES_L2 +=  $(RLC_AM_DIR)/rlc_am_status.c
SOURCES_L2 +=  $(RLC_AM_DIR)/rlc_am_timers.c
SOURCES_L2 +=  $(RLC_AM_DIR)/rlc_am_discard_notif.c
SOURCES_L2 +=  $(RLC_AM_DIR)/rlc_am_discard_tx.c

SOURCES_L2 +=  $(RLC_TM_DIR)/rlc_tm.c
SOURCES_L2 +=  $(RLC_TM_DIR)/rlc_tm_control_primitives.c
SOURCES_L2 +=  $(RLC_TM_DIR)/rlc_tm_fsm.c
SOURCES_L2 +=  $(RLC_TM_DIR)/rlc_tm_segment.c
SOURCES_L2 +=  $(RLC_TM_DIR)/rlc_tm_reassembly.c

SOURCES_L2 +=  $(RLC_UM_DIR)/rlc_um.c
SOURCES_L2 +=  $(RLC_UM_DIR)/rlc_um_fsm.c
SOURCES_L2 +=  $(RLC_UM_DIR)/rlc_um_control_primitives.c
SOURCES_L2 +=  $(RLC_UM_DIR)/rlc_um_segment.c
SOURCES_L2 +=  $(RLC_UM_DIR)/rlc_um_reassembly.c
SOURCES_L2 +=  $(RLC_UM_DIR)/rlc_um_receiver.c

SOURCES_L2 +=  $(RLC_DIR)/rlc_mac.c
SOURCES_L2 +=  $(RLC_DIR)/rlc.c
SOURCES_L2 +=  $(RLC_DIR)/rlc_rrc.c
SOURCES_L2 +=  $(RLC_DIR)/rlc_mpls.c


SOURCES_L2 +=  $(MATH_DIR)/random.c
SOURCES_L2 +=  $(MATH_DIR)/crc_byte.c

SOURCES_L2 +=  $(MEM_DIR)/mem_block.c

L2_OBJS=$(addsuffix .o,$(basename $(SOURCES_L2)))

####################################################
#      EXTRA COMPILER FLAGS
####################################################


CFLAGS = -g -ggdb -Wall -DUSER_MODE  -DPC_TARGET -DNB_ANTENNAS=1 -DPHY_EMUL -DPHY_CONTEXT -DMAC_CONTEXT  -DOPENAIR2 -DMESH #-DNO_RRM #-DRRC_RRM_XFACE  

ifdef ONE_MACHINE
CFLAGS+=-DPHY_EMUL_ONE_MACHINE
endif

ifdef FOR_ECOS
include $(ECOS_INSTALL_DIR)/include/pkgconf/ecos.mak
CFLAGS+=-I$(ECOS_INSTALL_DIR)/include -DECOS -static
LDFLAGS =  -nostartfiles -L$(ECOS_INSTALL_DIR)/lib -Ttarget.ld 
else
LDFLAGS= -lm -lrt -pthread
endif


##-MW- CFLAGS = -Wall -DUSER_MODE  -DPC_TARGET -DDEBUG_PHY -DNB_ANTENNAS=1 -DPHY_EMUL -DPHY_CONTEXT -DMAC_CONTEXT  -mmmx -msse -msse2 -g -ggdb
#-DNAS_NETLINK

ifdef NO_RRM
CFLAGS += -DNO_RRM
endif

#TARGET=ue_mac_sim
ifdef MASTER
CFLAGS += -DNODE_RG
#TARGET=ch_mac_sim
else
CFLAGS += -DNODE_MT
endif

ifdef NETLINK
CFLAGS += -DNAS_NETLINK
CFLAGS += -DLINUX
endif 

ifdef CELLULAR
CFLAGS += -DCELLULAR -DNO_RRM -I$(RRC_CELL_DIR)
endif



CFLAGS += -I$(TOPDIR)  -I$(COMMON_DIR) -I$(RLC_DIR) -I$(RLC_AM_DIR) -I$(RLC_UM_DIR)  -I$(RLC_TM_DIR) -I$(PDCP_DIR) -I$(MEM_DIR) -I$(LIST_DIR) -I$(MATH_DIR) -I$(TIMER_DIR)  -I$(OPENAIR3)/MESH -I$(OPENAIR3)/MESH/RRM 



SIMULATION_OBJS = $(SIMULATION_DIR)/openair_readconfigfile.o $(SIMULATION_DIR)/openair_configure.o

DEVICE_DRIVER_OBJS= $(DEVICE_DIR)/emul_init.o

CONTROL_OBJS = $(CONTROL_DIR)/phy_emulation.o


TRANSPORT_OBJS = $(TRANSPORT_DIR)/bypass_session_layer.o $(TRANSPORT_DIR)/multicast_link.o $(TRANSPORT_DIR)/socket.o

ABSTRACTION_OBJS = $(ABSTRACTION_DIR)/radio_emulation.o $(ABSTRACTION_DIR)/gauss.o $(ABSTRACTION_DIR)/taus.o $(ABSTRACTION_DIR)/dB_routines.o

PHY_INTERFACE_OBJS = $(PHY_INTERFACE_DIR)/mac_phy_primitives.o

MAC_OBJS = $(MAC_DIR)/openair2_proc.o $(MAC_DIR)/layer2_scheduler.o $(MAC_DIR)/nodeb_scheduler.o $(MAC_DIR)/ue_scheduler.o $(MAC_DIR)/main.o $(MAC_DIR)/ue_control_plane_procedures.o $(MAC_DIR)/nodeb_control_plane_procedures.o $(MAC_DIR)/lchan_interface.o $(MAC_DIR)/utils.o

L2_INTERFACE_OBJS = $(L2_INTERFACE_DIR)/openair_rrc_L2_interface.o

ifndef CELLULAR
RRC_OBJS = $(RRC_DIR)/main.o $(RRC_DIR)/L2_interface.o $(RRC_DIR)/utils.o 
else
ifdef MASTER
RRC_OBJS = $(RRC_CELL_DIR)/rrc_L2_interface.o $(RRC_CELL_DIR)/rrc_rg_init.o  $(RRC_CELL_DIR)/rrc_rg_main.o
else
RRC_OBJS = $(RRC_CELL_DIR)/rrc_L2_interface.o $(RRC_CELL_DIR)/rrc_ue_init.o  $(RRC_CELL_DIR)/rrc_ue_main.o
#RRC_OBJS = $(RRC_CELL_DIR)/ $(RRC_CELL_DIR)/
endif
endif

ifndef NO_RRM
RRC_OBJS +=$(RRC_DIR)/rrc_config.o $(RRC_DIR)/rrc_2_rrm_msg.o  $(RRC_DIR)/rrm_2_rrc_msg.o $(RRC_DIR)/rrc_rrm_interface.o 
endif

OBJ +=$(SIMULATION_OBJS) $(DEVICE_DRIVER_OBJS) $(CONTROL_OBJS) $(TRANSPORT_OBJS) $(ABSTRACTION_OBJS) $(PHY_INTERFACE_OBJS) $(MAC_OBJS) $(L2_INTERFACE_OBJS) $(RRC_OBJS) $(L2_OBJS) 

ifdef NETLINK
OBJ+=netlink_init.o
endif


all: mac_sim
$(OBJ) : %.o : %.c 
	$(CC) -c $(CFLAGS) $(ECOS_GLOBAL_CFLAGS) -o $@ $< 

mac_sim : $(OBJ) mac_sim.c 
	$(CC) mac_sim.c -o mac_sim $(CFLAGS) $(OBJ) $(LDFLAGS) $(ECOS_GLOBAL_LDFLAGS)

mac_sim_rrm : $(OBJ) mac_sim.c 
	$(CC) mac_sim.c -o mac_sim_rrm $(CFLAGS) $(OBJ) $(LDFLAGS) $(ECOS_GLOBAL_LDFLAGS)

mac_sim_mt : $(OBJ) mac_sim.c netlink_init.c
	$(CC)  mac_sim.c netlink_init.c  -o mac_sim_mt $(CFLAGS) $(OBJ) -lm -lrt -pthread

mac_sim_rg : $(OBJ) mac_sim.c netlink_init.c
	$(CC)  mac_sim.c netlink_init.c  -o mac_sim_rg $(CFLAGS) $(OBJ) -lm -lrt -pthread




clean:
	rm -f $(OBJ)
