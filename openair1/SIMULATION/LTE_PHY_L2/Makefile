CC = $(shell if [ $$OSTYPE = "cygwin" ] ; then echo "gcc-4";  else echo "gcc"; fi)
#CC = gcc

SSE3PROC = $(shell echo `grep ssse3 /proc/cpuinfo`) 
SSE4PROC = $(shell echo `grep sse4 /proc/cpuinfo`) 
CPUFLAGS = -mmmx -msse -msse2 -m32  #-mssse3 -msse4
CPUFLAGS += $(shell if [ -z $(SSE3PROC)]; then echo "" ; else echo "-mssse3"; fi)
CPUFLAGS += $(shell if [ -z $(SSE4PROC)]; then echo "" ; else echo "-msse4"; fi)

TOP_DIR = ../..
OPENAIR2_DIR = ../../../openair2
OPENAIR3     = $(OPENAIR3_DIR)

CFLAGS = -O2 -Wall -fno-common -mpreferred-stack-boundary=4 -fno-strict-aliasing -DPHYSIM -DNODE_RG -DUSER_MODE -DPC_TARGET -DPC_DSP -DNB_ANTENNAS_RX=2 -DNB_ANTENNAS_TXRX=2 -DNB_ANTENNAS_TX=2 -DMAX_MODULES=1 -g -ggdb $(CPUFLAGS)

#CFLAGS = -Wall -Wno-packed-bitfield-compat -fno-common -mpreferred-stack-boundary=4 -fno-strict-aliasing -DNODE_RG -DUSER_MODE -DPC_TARGET -DPC_DSP -DNB_ANTENNAS_RX=2 -DNB_ANTENNAS_TXRX=2 -DNB_ANTENNAS_TX=2 -DMAX_MODULES=1 -DPHY_CONTEXT=1 -g -ggdb $(CPUFLAGS)

CFLAGS += -DOPENAIR_LTE -DIFFT_FPGA -DIFFT_FPGA_UE -DOFDMA_ULSCH


CFLAGS += -DOPENAIR2 -DTBS_FIX
ifdef EMOS
CFLAGS += -DEMOS
endif

ifdef DEBUG_PHY
CFLAGS += -DDEBUG_PHY
endif



PHY_OBJS = $(TOP_DIR)/PHY/LTE_TRANSPORT/pss.o
PHY_OBJS += $(TOP_DIR)/PHY/LTE_TRANSPORT/pilots.o
PHY_OBJS += $(TOP_DIR)/PHY/LTE_TRANSPORT/dlsch_coding.o
PHY_OBJS += $(TOP_DIR)/PHY/LTE_TRANSPORT/dlsch_modulation.o
PHY_OBJS += $(TOP_DIR)/PHY/LTE_TRANSPORT/dlsch_demodulation.o
PHY_OBJS += $(TOP_DIR)/PHY/LTE_TRANSPORT/dlsch_decoding.o
PHY_OBJS += $(TOP_DIR)/PHY/LTE_TRANSPORT/dci_tools.o
PHY_OBJS += $(TOP_DIR)/PHY/LTE_TRANSPORT/uci_tools.o
PHY_OBJS += $(TOP_DIR)/PHY/LTE_TRANSPORT/lte_mcs.o
PHY_OBJS += $(TOP_DIR)/PHY/LTE_TRANSPORT/pbch.o
PHY_OBJS += $(TOP_DIR)/PHY/LTE_TRANSPORT/dci.o
PHY_OBJS += $(TOP_DIR)/PHY/LTE_TRANSPORT/phich.o
PHY_OBJS += $(TOP_DIR)/PHY/LTE_TRANSPORT/pcfich.o
PHY_OBJS += $(TOP_DIR)/PHY/LTE_TRANSPORT/srs_modulation.o
PHY_OBJS += $(TOP_DIR)/PHY/LTE_TRANSPORT/drs_modulation.o
PHY_OBJS += $(TOP_DIR)/PHY/LTE_TRANSPORT/ulsch_modulation.o
PHY_OBJS += $(TOP_DIR)/PHY/LTE_TRANSPORT/ulsch_demodulation.o
PHY_OBJS += $(TOP_DIR)/PHY/LTE_TRANSPORT/ulsch_coding.o
PHY_OBJS += $(TOP_DIR)/PHY/LTE_TRANSPORT/ulsch_decoding.o
PHY_OBJS += $(TOP_DIR)/PHY/LTE_TRANSPORT/rar_tools.o
PHY_OBJS += $(TOP_DIR)/PHY/MODULATION/ofdm_mod.o
PHY_OBJS += $(TOP_DIR)/PHY/MODULATION/slot_fep.o
PHY_OBJS += $(TOP_DIR)/PHY/MODULATION/slot_fep_ul.o
PHY_OBJS += $(TOP_DIR)/PHY/LTE_ESTIMATION/lte_sync_time.o
PHY_OBJS += $(TOP_DIR)/PHY/LTE_ESTIMATION/lte_adjust_sync.o
PHY_OBJS += $(TOP_DIR)/PHY/LTE_ESTIMATION/lte_dl_channel_estimation.o
PHY_OBJS += $(TOP_DIR)/PHY/LTE_ESTIMATION/lte_ul_channel_estimation.o
PHY_OBJS += $(TOP_DIR)/PHY/LTE_ESTIMATION/lte_est_freq_offset.o
PHY_OBJS += $(TOP_DIR)/PHY/LTE_ESTIMATION/lte_ue_measurements.o
PHY_OBJS += $(TOP_DIR)/PHY/LTE_ESTIMATION/lte_eNB_measurements.o
PHY_OBJS += $(TOP_DIR)/PHY/LTE_ESTIMATION/adjust_gain.o
PHY_OBJS += $(TOP_DIR)/PHY/LTE_REFSIG/lte_dl_cell_spec.o
PHY_OBJS += $(TOP_DIR)/PHY/LTE_REFSIG/lte_gold.o
PHY_OBJS += $(TOP_DIR)/PHY/LTE_REFSIG/lte_ul_ref.o
PHY_OBJS += $(TOP_DIR)/PHY/CODING/lte_segmentation.o
PHY_OBJS += $(TOP_DIR)/PHY/CODING/ccoding_byte.o
PHY_OBJS += $(TOP_DIR)/PHY/CODING/ccoding_byte_lte.o
PHY_OBJS += $(TOP_DIR)/PHY/CODING/3gpplte.o
PHY_OBJS += $(TOP_DIR)/PHY/CODING/crc_byte.o
PHY_OBJS += $(TOP_DIR)/PHY/CODING/3gpplte_turbo_decoder_sse.o
PHY_OBJS += $(TOP_DIR)/PHY/CODING/lte_rate_matching.o
PHY_OBJS += $(TOP_DIR)/PHY/CODING/rate_matching.o
PHY_OBJS += $(TOP_DIR)/PHY/CODING/viterbi.o
PHY_OBJS += $(TOP_DIR)/PHY/CODING/viterbi_lte.o
PHY_OBJS += $(TOP_DIR)/PHY/INIT/lte_init.o
PHY_OBJS += $(TOP_DIR)/PHY/INIT/init_top.o
PHY_OBJS += $(TOP_DIR)/MAC_INTERFACE/init.o
ifdef EMOS
PHY_OBJS += $(TOP_DIR)/PHY/LTE_ESTIMATION/lte_dl_channel_estimation_emos.o
endif



SIMULATION_OBJS  = $(TOP_DIR)/SIMULATION/TOOLS/gauss.o  
SIMULATION_OBJS += $(TOP_DIR)/SIMULATION/TOOLS/random_channel.o  
SIMULATION_OBJS += $(TOP_DIR)/SIMULATION/TOOLS/rangen_double.o  
SIMULATION_OBJS += $(TOP_DIR)/SIMULATION/TOOLS/taus.o  
SIMULATION_OBJS += $(TOP_DIR)/SIMULATION/TOOLS/multipath_channel.o
SIMULATION_OBJS += $(TOP_DIR)/SIMULATION/RF/rf.o
SIMULATION_OBJS += $(TOP_DIR)/SIMULATION/RF/adc.o
SIMULATION_OBJS += $(TOP_DIR)/SIMULATION/RF/dac.o

TOOLS_OBJS = $(TOP_DIR)/PHY/TOOLS/file_output.o
TOOLS_OBJS += $(TOP_DIR)/PHY/TOOLS/fft.o
TOOLS_OBJS += $(TOP_DIR)/PHY/TOOLS/lte_dfts.o
TOOLS_OBJS += $(TOP_DIR)/PHY/TOOLS/log2_approx.o
TOOLS_OBJS += $(TOP_DIR)/PHY/TOOLS/cmult_sv.o
TOOLS_OBJS += $(TOP_DIR)/PHY/TOOLS/cmult_vv.o
TOOLS_OBJS += $(TOP_DIR)/PHY/TOOLS/cadd_vv.o
TOOLS_OBJS += $(TOP_DIR)/PHY/TOOLS/cdot_prod.o
TOOLS_OBJS += $(TOP_DIR)/PHY/TOOLS/signal_energy.o
TOOLS_OBJS += $(TOP_DIR)/PHY/TOOLS/dB_routines.o

SCHED_OBJS += $(TOP_DIR)/SCHED/phy_procedures_lte_common.o
SCHED_OBJS += $(TOP_DIR)/SCHED/phy_procedures_lte_ue.o
SCHED_OBJS += $(TOP_DIR)/SCHED/phy_procedures_lte_eNb.o

STATS_OBJS += $(TOP_DIR)/ARCH/CBMIMO1/DEVICE_DRIVER/cbmimo1_proc.o


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

SOURCES_L2 +=  $(MEM_DIR)/mem_block.c

RRC_OBJS =  $(RRC_DIR)/main.o $(RRC_DIR)/L2_interface.o $(RRC_DIR)/utils.o 

PHY_INTERFACE_OBJS = $(PHY_INTERFACE_DIR)/mac_phy_primitives.o

MAC_OBJS = $(MAC_DIR)/openair2_proc.o $(MAC_DIR)/layer2_scheduler.o $(MAC_DIR)/nodeb_scheduler.o $(MAC_DIR)/ue_scheduler.o $(MAC_DIR)/main.o $(MAC_DIR)/ue_control_plane_procedures.o $(MAC_DIR)/nodeb_control_plane_procedures.o $(MAC_DIR)/lchan_interface.o $(MAC_DIR)/rar_tools.o

L2_INTERFACE_OBJS = $(L2_INTERFACE_DIR)/openair_rrc_L2_interface.o


L2_OBJS=$(addsuffix .o,$(basename $(SOURCES_L2))) $(PHY_INTERFACE_OBJS) $(MAC_OBJS) $(L2_INTERFACE_OBJS) $(RRC_OBJS)

L2_incl = -I$(OPENAIR2_DIR)  -I$(COMMON_DIR) -I$(RLC_DIR) -I$(RLC_AM_DIR) -I$(RLC_UM_DIR)  -I$(RLC_TM_DIR) -I$(PDCP_DIR) -I$(MEM_DIR) -I$(LIST_DIR) -I$(MATH_DIR) -I$(TIMER_DIR) -I$(OPENAIR3)/MESH -I$(OPENAIR3)/MESH/RRM  -DNO_RRM


OBJ = $(PHY_OBJS) $(SIMULATION_OBJS) $(TOOLS_OBJS) $(SCHED_OBJS) $(STATS_OBJS)
OBJ2 = $(PHY_OBJS) $(SIMULATION_OBJS) $(TOOLS_OBJS)

all: dlsim dlsim2 ulsim ulsim2 physim

test:
	echo $(CFLAGS)

$(OBJ) : %.o : %.c
	$(CC) -c $(CFLAGS) -DPHY_CONTEXT=1 -I$(TOP_DIR) $(L2_incl) -o $@ $<

$(L2_OBJS) : %.o : %.c
	$(CC) -c $(CFLAGS) -DMAC_CONTEXT=1 -DPHY_CONTEXT=1 -I$(TOP_DIR) $(L2_incl) -o $@ $<

dlsim : $(OBJ2) dlsim.c
	$(CC) dlsim.c -I$(TOP_DIR) -o dlsim $(CFLAGS) $(OBJ2) -lm

dlsim2 : $(OBJ2) dlsim2.c
	$(CC) dlsim2.c -I$(TOP_DIR) -o dlsim2 $(CFLAGS) $(OBJ2) -lm

ulsim : $(OBJ2) ulsim.c
	$(CC) ulsim.c -I$(TOP_DIR) -o ulsim $(CFLAGS) $(OBJ2) -lm

ulsim2 : $(OBJ2) ulsim2.c
	$(CC) ulsim2.c -I$(TOP_DIR) -o ulsim2 $(CFLAGS) $(OBJ2) -lm

physim : $(OBJ) $(L2_OBJS) phy_procedures_sim.c 
	$(CC)  phy_procedures_sim.c -I$(TOP_DIR) $(L2_incl) -o physim $(CFLAGS) $(OBJ) $(L2_OBJS) -lm

clean:
	rm -f dlsim dlsim2
	rm -f $(OBJ)
	rm -f $(L2_OBJS)
	rm -f *.o
	rm -f *.exe*

cleanl1:
	rm -f physim
	rm -f $(OBJ)
	rm -f *.o
	rm -f *.exe
cleanl2:
	rm -f $(L2_OBJS)
