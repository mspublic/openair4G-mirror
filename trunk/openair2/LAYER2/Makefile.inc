ifeq ($(OPENAIR2),1)
EXTRA_CFLAGS += -DOPENAIR2
endif
EXTRA_CFLAGS += -DASSERT
EXTRA_CFLAGS += -DNO_RRM

COMMON_DIR = $(OPENAIR2_TOP)/COMMON
L2_INTERFACE_DIR = $(OPENAIR2_TOP)/RRC/L2_INTERFACE
#RLC_UM_DIR = $(OPENAIR2_TOP)/LAYER2/RLC/UM_v6.1.0_LITE
RLC_UM_DIR = $(OPENAIR2_TOP)/LAYER2/RLC/UM_v9.3.0
RLC_AM_DIR = $(OPENAIR2_TOP)/LAYER2/RLC/AM_v9.3.0
RLC_TM_DIR = $(OPENAIR2_TOP)/LAYER2/RLC/TM
RLC_DIR = $(OPENAIR2_TOP)/LAYER2/RLC
PDCP_DIR = $(OPENAIR2_TOP)/LAYER2/PDCP_v10.1.0
MEM_DIR = $(OPENAIR2_TOP)/UTIL/MEM
LIST_DIR = $(OPENAIR2_TOP)/UTIL/LISTS
MATH_DIR = $(OPENAIR2_TOP)/UTIL/MATH
TIMER_DIR = $(OPENAIR2_TOP)/UTIL/TIMER
PHY_INTERFACE_DIR = $(OPENAIR2_TOP)/PHY_INTERFACE
RRC_DIR = $(OPENAIR2_TOP)/RRC/LITE
L2_INTERFACE_DIR = $(OPENAIR2_TOP)/RRC/L2_INTERFACE
#CONTROL_DIR = $(OPENAIR2_TOP)/SIMULATION/PHY_EMULATION/CONTROL
MAC_DIR = $(OPENAIR2_TOP)/LAYER2/MAC
RRC_CELL_DIR = $(OPENAIR2_TOP)/RRC/CELLULAR
LOG_DIR=$(OPENAIR2_TOP)/UTIL/LOG
OCG_DIR=$(OPENAIR2_TOP)/UTIL/OCG
OPT_DIR=$(OPENAIR2_TOP)/UTIL/OPT
OMG_DIR=$(OPENAIR2_TOP)/UTIL/OMG


SOURCES_L2 =  $(LIST_DIR)/list.c
SOURCES_L2 +=  $(LIST_DIR)/list2.c

SOURCES_L2 +=  $(TIMER_DIR)/umts_timer.c

SOURCES_L2 +=  $(PDCP_DIR)/pdcp.c
SOURCES_L2 +=  $(PDCP_DIR)/pdcp_fifo.c
SOURCES_L2 +=  $(PDCP_DIR)/pdcp_sequence_manager.c
SOURCES_L2 +=  $(PDCP_DIR)/pdcp_primitives.c

SOURCES_L2 +=  $(RLC_AM_DIR)/rlc_am.c
SOURCES_L2 +=  $(RLC_AM_DIR)/rlc_am_init.c
SOURCES_L2 +=  $(RLC_AM_DIR)/rlc_am_timer_poll_retransmit.c
SOURCES_L2 +=  $(RLC_AM_DIR)/rlc_am_timer_reordering.c
SOURCES_L2 +=  $(RLC_AM_DIR)/rlc_am_timer_status_prohibit.c
SOURCES_L2 +=  $(RLC_AM_DIR)/rlc_am_segment.c
SOURCES_L2 +=  $(RLC_AM_DIR)/rlc_am_segments_holes.c
SOURCES_L2 +=  $(RLC_AM_DIR)/rlc_am_in_sdu.c
SOURCES_L2 +=  $(RLC_AM_DIR)/rlc_am_receiver.c
SOURCES_L2 +=  $(RLC_AM_DIR)/rlc_am_retransmit.c
SOURCES_L2 +=  $(RLC_AM_DIR)/rlc_am_windows.c
SOURCES_L2 +=  $(RLC_AM_DIR)/rlc_am_rx_list.c
SOURCES_L2 +=  $(RLC_AM_DIR)/rlc_am_reassembly.c
SOURCES_L2 +=  $(RLC_AM_DIR)/rlc_am_status_report.c


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
SOURCES_L2 +=  $(RLC_UM_DIR)/rlc_um_dar.c

ifeq ($(OPENAIR_EMU),1)
SOURCES_L2 +=  $(RLC_UM_DIR)/rlc_um_very_simple_test.c
endif

SOURCES_L2 +=  $(RLC_DIR)/rlc_mac.c
SOURCES_L2 +=  $(RLC_DIR)/rlc.c
SOURCES_L2 +=  $(RLC_DIR)/rlc_rrc.c
SOURCES_L2 +=  $(RLC_DIR)/rlc_mpls.c

SOURCES_L2 +=  $(MEM_DIR)/mem_block.c

RRC_OBJS =  $(RRC_DIR)/rrc_UE.o $(RRC_DIR)/rrc_eNB.o $(RRC_DIR)/rrc_common.o $(RRC_DIR)/L2_interface.o 


MAC_OBJS =  $(MAC_DIR)/lte_transport_init.o
MAC_OBJS += $(MAC_DIR)/main.o
MAC_OBJS += $(MAC_DIR)/ue_procedures.o
MAC_OBJS += $(MAC_DIR)/rar_tools.o
MAC_OBJS += $(MAC_DIR)/eNB_scheduler.o
MAC_OBJS += $(MAC_DIR)/config.o

LOG_OBJS =  $(LOG_DIR)/log.o

ifeq ($(OPENAIR_EMU),1)
OCG_OBJS  =  $(OCG_DIR)/OCG.o
OCG_OBJS +=  $(OCG_DIR)/OCG_create_dir.o
OCG_OBJS +=  $(OCG_DIR)/OCG_detect_file.o
OCG_OBJS +=  $(OCG_DIR)/OCG_generate_report.o
OCG_OBJS +=  $(OCG_DIR)/OCG_parse_filename.o
OCG_OBJS +=  $(OCG_DIR)/OCG_parse_XML.o
OCG_OBJS +=  $(OCG_DIR)/OCG_save_XML.o

OPT_OBJS =  $(OPT_DIR)/probe.o

OMG_OBJS =  $(OMG_DIR)/omg.o
OMG_OBJS +=  $(OMG_DIR)/common.o
OMG_OBJS +=  $(OMG_DIR)/job.o
OMG_OBJS +=  $(OMG_DIR)/static.o
OMG_OBJS +=  $(OMG_DIR)/rwp.o
OMG_OBJS +=  $(OMG_DIR)/rwalk.o
OMG_OBJS +=  $(OMG_DIR)/hashtable.o
OMG_OBJS +=  $(OMG_DIR)/mobility_parser.o
OMG_OBJS +=  $(OMG_DIR)/trace.o
endif

L2_INTERFACE_OBJS = $(L2_INTERFACE_DIR)/openair_rrc_L2_interface.o

L2_OBJS=$(addsuffix .o,$(basename $(SOURCES_L2))) $(PHY_INTERFACE_OBJS) $(MAC_OBJS) $(L2_INTERFACE_OBJS) $(LOG_OBJS) $(OCG_OBJS) $(OPT_OBJS) $(OMG_OBJS)

L2_incl = -I$(OPENAIR2_TOP)  -I$(COMMON_DIR) -I$(RLC_DIR) -I$(RLC_AM_DIR) -I$(RLC_UM_DIR)  -I$(RLC_TM_DIR) -I$(PDCP_DIR) -I$(MEM_DIR) -I$(LIST_DIR) -I$(OCG_DIR) -I$(LOG_DIR) -I$(MATH_DIR) -I$(TIMER_DIR) -I$(OPENAIR3)/MESH  -I$(OPENAIR2_TOP)/RRC/LITE/MESSAGES -I$(OPENAIR3)/MESH/RRM -I$(OPT_DIR) -I$(OMG_DIR)

ifeq ($(rrc_cellular),1)
  include $(RRC_CELL_DIR)/Makefile.inc
  L2_OBJS += $(RRC_CELLULAR_OBJS)
  L2_incl += -I$(OPENAIR2_TOP)/RRC/CELLULAR
  EXTRA_CFLAGS += -DRRC_CELLULAR -DCELLULAR
else
  L2_OBJS += $(RRC_OBJS)
  L2_incl += -I$(OPENAIR2_TOP)/RRC/LITE
endif


#EXTRA_CFLAGS += $(L2_incl)

printl2objs:
	@echo $(L2_OBJS)
	@echo rrc_cellular variable is $(rrc_cellular)
	@echo eNB_flag is $(eNB_flag)
	@echo UE_flag is $(UE_flag)

printl2incl:
	@echo $(L2_incl)
