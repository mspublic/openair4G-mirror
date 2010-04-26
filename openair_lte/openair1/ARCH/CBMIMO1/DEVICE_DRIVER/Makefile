CCC = gcc
KERNEL_MAIN_TYPE=$(shell echo `uname -r | cut -d. -f-2  | tr "." "_"`)
export KERNEL_MAIN_TYPE

SUBVERSION=$(shell echo `grep '^SUBLEVEL =' /usr/src/linux/Makefile | sed -e 's, ,,g' | sed -e 's/SUBLEVEL=//'`)
IS_KERNEL_SUBVERSION_GREATER_THAN_20=$(shell if [ $(SUBVERSION) -ge 20 ] ; then echo true ; fi)

GRPCI_SOFTCONFIG_H=grpci_softconfig.h
GRPCI_SOFTREGS_H=grpci_softregs.h

OPENAIR1_TOP = ../../..
OPENAIR2_TOP = ../../../../openair2
OPENAIR3_TOP = ../../../../openair3

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

ifdef RTAI
# Get the RTAI variables
CCC = $(shell rtai-config --cc)
RTAI_SUBVERSION=$(shell rtai-config --version | sed -e 's/^..\(.\).*$$/\1/')
IS_RTAI_SUBVERSION_LESS_THAN_FIVE=$(shell if [ $(RTAI_SUBVERSION) -lt 5 ] ; then echo true ; fi)

EXTRA_CFLAGS = -D__IN_RTAI__ $(shell rtai-config --module-cflags) -g -march=pentium4 -DNODE_RG -DCBMIMO1 -DBIGPHYSAREA -DRTAI_ENABLED $(if $(IS_KERNEL_SUBVERSION_GREATER_THAN_20),-mregparm=3 -fno-stack-protector,) -DHW_PREFIX_REMOVAL -Wall -D__KERNEL__ -DMODULE -D_LOOSE_KERNEL_NAMES -DPHY_CONTEXT -DWIDENS_DLC -I/lib/modules/$(shell uname -r)/build/include -I/lib/modules/$(shell uname -r)/build/include/asm/mach-default -include /lib/modules/$(shell uname -r)/build/include/linux/autoconf.h $(if $(IS_RTAI_SUBVERSION_LESS_THAN_FIVE),-DRTAI_ISNT_POSIX,)
endif

EXTRA_CFLAGS +=   -DKERNEL$(KERNEL_MAIN_TYPE)
#-DOLD_REMAP

# Old for 2.6.20
#EXTRA_CFLAGS += -Wall -Wstrict-prototypes -fno-common -fno-strict-aliasing -pipe -fno-strength-reduce -mmmx -msse -msse2 -falign-loops=2 -falign-jumps=2 -falign-functions=2 -DCPU=686 -DMODULE  -D_LOOSE_KERNEL_NAMES -O2


#EXTRA_CFLAGS += -Wall -Wstrict-prototypes -fno-common -fno-strict-aliasing -fno-unit-at-a-time -march=i686 -fno-strength-reduce  -fno-exceptions -pipe -mmmx -msse -msse2 -falign-loops=2 -falign-jumps=2 -falign-functions=2 -fno-strict-aliasing -fno-common -ffreestanding -fomit-frame-pointer -nostdinc -DMODULE  -D_LOOSE_KERNEL_NAMES -O2

EXTRA_CFLAGS += -Wall -Wstrict-prototypes -fno-common -fno-strict-aliasing -pipe -mpreferred-stack-boundary=4 -freg-struct-return -ffreestanding -maccumulate-outgoing-args -funroll-loops -march=i686 -fomit-frame-pointer -nostdinc -DMODULE  -D_LOOSE_KERNEL_NAMES -O2

SSE3PROC = $(shell echo `grep ssse3 /proc/cpuinfo`) 
SSE4PROC = $(shell echo `grep sse4 /proc/cpuinfo`) 
CPUFLAGS = -mmmx -msse -msse2 -m32 
CPUFLAGS += $(shell if [ -z $(SSE3PROC)]; then echo "" ; else echo "-mssse3"; fi)
CPUFLAGS += $(shell if [ -z $(SSE4PROC)]; then echo "" ; else echo "-msse4"; fi)

EXTRA_CFLAGS += $(CPUFLAGS)

EXTRA_CFLAGS += -DMAX_MODULES=1

ifdef DEBUG_PHY
EXTRA_CFLAGS += -DDEBUG_PHY
endif

ifdef NO_CARD_TEST
EXTRA_CFLAGS += -DNOCARD_TEST
endif

ifdef EMOS
EXTRA_CFLAGS += -DEMOS 
endif

ifdef DUALSTREAM
EXTRA_CFLAGS += -DDUALSTREAM
endif

EXTRA_CFLAGS += -I$(OPENAIR1_DIR) -I$(obj)/../LEON3/hostpc  -I/usr/include/

ifndef NO_TXMUX
EXTRA_CFLAGS += -DBIT8_TXMUX -DBIT8_TX
endif

ifdef IDROMEL_RF 
EXTRA_CFLAGS += -DNB_ANTENNAS_RX=2 -DNB_ANTENNAS_TX=1 -DNB_ANTENNAS_TXRX=2
else
EXTRA_CFLAGS += -DNB_ANTENNAS_RX=2 -DNB_ANTENNAS_TX=2 -DNB_ANTENNAS_TXRX=2 
endif

#CFLAGS += -I$(OPENAIR1_DIR) -I$(obj)/../LEON3/hostpc 
EXTRA_CFLAGS += -I$(OPENAIR1_DIR) -I$(OPENAIR1_DIR)/ARCH/LEON3/hostpc

ifdef OPENAIR_LTE
EXTRA_CFLAGS += -DOPENAIR_LTE -DIFFT_FPGA -DTBS_FIX
endif

EXTRA_CFLAGS += -I$(OPENAIR2_DIR)  -I$(COMMON_DIR) -I$(RLC_DIR) -I$(RLC_AM_DIR) -I$(RLC_UM_DIR)  -I$(RLC_TM_DIR) -I$(PDCP_DIR) -I$(MEM_DIR) -I$(LIST_DIR) -I$(MATH_DIR) -I$(TIMER_DIR) -I$(OPENAIR3_DIR)/MESH -I$(OPENAIR3_DIR)/MESH/RRM  -DNO_RRM -DMAC_CONTEXT -DPHY_CONTEXT





obj-m += openair_rf.o



# files for Soft MODEM implementation

EXTRA_CFLAGS += -DPC_TARGET -DPC_DSP


openair_rf-objs += $(OPENAIR1_TOP)/PHY/TOOLS/memory_routines.o
openair_rf-objs += $(OPENAIR1_TOP)/PHY/TOOLS/dB_routines.o
openair_rf-objs += $(OPENAIR1_TOP)/PHY/TOOLS/signal_energy.o
openair_rf-objs += $(OPENAIR1_TOP)/PHY/TOOLS/sqrt.o
openair_rf-objs += $(OPENAIR1_TOP)/PHY/TOOLS/log2_approx.o
#openair_rf-objs += $(OPENAIR1_TOP)/PHY/TOOLS/8bit_txmux.o
openair_rf-objs += $(OPENAIR1_TOP)/PHY/TOOLS/fft.o
openair_rf-objs += $(OPENAIR1_TOP)/PHY/TOOLS/cmult_vv.o
openair_rf-objs += $(OPENAIR1_TOP)/PHY/TOOLS/cmult_sv.o
openair_rf-objs += $(OPENAIR1_TOP)/PHY/TOOLS/cmult_vvh.o
openair_rf-objs += $(OPENAIR1_TOP)/PHY/TOOLS/cmult_mm.o
openair_rf-objs += $(OPENAIR1_TOP)/PHY/TOOLS/cadd_sv.o
openair_rf-objs += $(OPENAIR1_TOP)/PHY/TOOLS/cadd_vv.o
openair_rf-objs += $(OPENAIR1_TOP)/PHY/TOOLS/angle.o
openair_rf-objs += $(OPENAIR1_TOP)/PHY/TOOLS/cdot_prod.o
openair_rf-objs += $(OPENAIR1_TOP)/SIMULATION/TOOLS/taus.o

openair_rf-objs += $(OPENAIR1_TOP)/PHY/MODULATION/ofdm_mod.o

ifndef OPENAIR_LTE
ifdef EMOS
openair_rf-objs += $(OPENAIR1_TOP)/PHY/ESTIMATION/channel_estimation_emos.o
openair_rf-objs += $(OPENAIR1_TOP)/PHY/TOOLS/phase_comp.o
endif
openair_rf-objs += $(OPENAIR1_TOP)/PHY/ESTIMATION/channel_estimation.o
openair_rf-objs += $(OPENAIR1_TOP)/PHY/ESTIMATION/synch_time.o
openair_rf-objs += $(OPENAIR1_TOP)/PHY/ESTIMATION/adjust_sync.o
openair_rf-objs += $(OPENAIR1_TOP)/PHY/ESTIMATION/adjust_sync2.o
openair_rf-objs += $(OPENAIR1_TOP)/PHY/ESTIMATION/adjust_gain.o
openair_rf-objs += $(OPENAIR1_TOP)/PHY/ESTIMATION/calc_timing_offset.o
openair_rf-objs += $(OPENAIR1_TOP)/PHY/ESTIMATION/channel_interpolation.o
openair_rf-objs += $(OPENAIR1_TOP)/PHY/ESTIMATION/calc_mmse_filter.o
openair_rf-objs += $(OPENAIR1_TOP)/PHY/ESTIMATION/model_based_sensing.o

openair_rf-objs += $(OPENAIR1_TOP)/PHY/TRANSPORT/chsch.o
openair_rf-objs += $(OPENAIR1_TOP)/PHY/TRANSPORT/sch.o
openair_rf-objs += $(OPENAIR1_TOP)/PHY/TRANSPORT/mrbch.o
openair_rf-objs += $(OPENAIR1_TOP)/PHY/TRANSPORT/chbch.o
openair_rf-objs += $(OPENAIR1_TOP)/PHY/TRANSPORT/mrbch.o
openair_rf-objs += $(OPENAIR1_TOP)/PHY/TRANSPORT/sach.o
openair_rf-objs += $(OPENAIR1_TOP)/PHY/TRANSPORT/tools.o
else 
ifdef EMOS
openair_rf-objs += $(OPENAIR1_TOP)/PHY/LTE_ESTIMATION/lte_dl_channel_estimation_emos.o
endif
openair_rf-objs += $(OPENAIR1_TOP)/PHY/LTE_REFSIG/lte_dl_cell_spec.o
openair_rf-objs += $(OPENAIR1_TOP)/PHY/LTE_REFSIG/lte_ul_ref.o
openair_rf-objs += $(OPENAIR1_TOP)/PHY/LTE_REFSIG/lte_gold.o
openair_rf-objs += $(OPENAIR1_TOP)/PHY/LTE_ESTIMATION/lte_sync_time.o
openair_rf-objs += $(OPENAIR1_TOP)/PHY/LTE_ESTIMATION/lte_adjust_sync.o
openair_rf-objs += $(OPENAIR1_TOP)/PHY/LTE_ESTIMATION/lte_dl_channel_estimation.o
openair_rf-objs += $(OPENAIR1_TOP)/PHY/LTE_ESTIMATION/lte_ul_channel_estimation.o
openair_rf-objs += $(OPENAIR1_TOP)/PHY/LTE_ESTIMATION/lte_ue_measurements.o
openair_rf-objs += $(OPENAIR1_TOP)/PHY/LTE_ESTIMATION/lte_eNB_measurements.o
openair_rf-objs += $(OPENAIR1_TOP)/PHY/LTE_ESTIMATION/adjust_gain.o
openair_rf-objs += $(OPENAIR1_TOP)/PHY/LTE_ESTIMATION/lte_est_freq_offset.o
openair_rf-objs += $(OPENAIR1_TOP)/PHY/LTE_TRANSPORT/dlsch_modulation.o  
openair_rf-objs += $(OPENAIR1_TOP)/PHY/LTE_TRANSPORT/dlsch_coding.o  
openair_rf-objs += $(OPENAIR1_TOP)/PHY/LTE_TRANSPORT/dlsch_demodulation.o  
openair_rf-objs += $(OPENAIR1_TOP)/PHY/LTE_TRANSPORT/dlsch_decoding.o  
openair_rf-objs += $(OPENAIR1_TOP)/PHY/LTE_TRANSPORT/ulsch_modulation.o  
openair_rf-objs += $(OPENAIR1_TOP)/PHY/LTE_TRANSPORT/ulsch_coding.o  
openair_rf-objs += $(OPENAIR1_TOP)/PHY/LTE_TRANSPORT/ulsch_demodulation.o  
openair_rf-objs += $(OPENAIR1_TOP)/PHY/LTE_TRANSPORT/ulsch_decoding.o  
openair_rf-objs += $(OPENAIR1_TOP)/PHY/LTE_TRANSPORT/srs_modulation.o  
openair_rf-objs += $(OPENAIR1_TOP)/PHY/LTE_TRANSPORT/drs_modulation.o 
openair_rf-objs += $(OPENAIR1_TOP)/PHY/LTE_TRANSPORT/dci.o  
openair_rf-objs += $(OPENAIR1_TOP)/PHY/LTE_TRANSPORT/phich.o
openair_rf-objs += $(OPENAIR1_TOP)/PHY/LTE_TRANSPORT/pcfich.o
openair_rf-objs += $(OPENAIR1_TOP)/PHY/LTE_TRANSPORT/dci_tools.o  
openair_rf-objs += $(OPENAIR1_TOP)/PHY/LTE_TRANSPORT/rar_tools.o  
openair_rf-objs += $(OPENAIR1_TOP)/PHY/LTE_TRANSPORT/uci_tools.o
openair_rf-objs += $(OPENAIR1_TOP)/PHY/LTE_TRANSPORT/lte_mcs.o
openair_rf-objs += $(OPENAIR1_TOP)/PHY/LTE_TRANSPORT/pbch.o  
openair_rf-objs += $(OPENAIR1_TOP)/PHY/LTE_TRANSPORT/pilots.o  
openair_rf-objs += $(OPENAIR1_TOP)/PHY/LTE_TRANSPORT/pss.o
openair_rf-objs += $(OPENAIR1_TOP)/PHY/LTE_TRANSPORT/phich.o
openair_rf-objs += $(OPENAIR1_TOP)/PHY/LTE_TRANSPORT/pcfich.o
openair_rf-objs += $(OPENAIR1_TOP)/PHY/LTE_TRANSPORT/lte_transport_init.o
openair_rf-objs += $(OPENAIR1_TOP)/PHY/MODULATION/slot_fep.o
openair_rf-objs += $(OPENAIR1_TOP)/PHY/MODULATION/slot_fep_ul.o
openair_rf-objs += $(OPENAIR1_TOP)/PHY/CODING/lte_segmentation.o
openair_rf-objs += $(OPENAIR1_TOP)/PHY/CODING/lte_rate_matching.o
openair_rf-objs += $(OPENAIR1_TOP)/PHY/CODING/rate_matching.o
openair_rf-objs += $(OPENAIR1_TOP)/PHY/CODING/3gpplte.o
openair_rf-objs += $(OPENAIR1_TOP)/PHY/CODING/3gpplte_turbo_decoder_sse.o
openair_rf-objs += $(OPENAIR1_TOP)/PHY/CODING/ccoding_byte.o
openair_rf-objs += $(OPENAIR1_TOP)/PHY/CODING/ccoding_byte_lte.o
openair_rf-objs += $(OPENAIR1_TOP)/PHY/CODING/viterbi.o
openair_rf-objs += $(OPENAIR1_TOP)/PHY/CODING/viterbi_lte.o


endif

openair_rf-objs += $(OPENAIR1_TOP)/PHY/CODING/ccoding_byte.o
openair_rf-objs += $(OPENAIR1_TOP)/PHY/CODING/rate_matching.o
openair_rf-objs += $(OPENAIR1_TOP)/PHY/CODING/viterbi.o
openair_rf-objs += $(OPENAIR1_TOP)/PHY/CODING/crc_byte.o

ifndef OPENAIR_LTE
openair_rf-objs += $(OPENAIR1_TOP)/PHY/INIT/init.o
else
openair_rf-objs += $(OPENAIR1_TOP)/PHY/INIT/init_top.o
openair_rf-objs += $(OPENAIR1_TOP)/PHY/INIT/lte_init.o
endif

openair_rf-objs += $(OPENAIR1_TOP)/ARCH/COMMON/bigphys.o
openair_rf-objs += $(OPENAIR1_TOP)/ARCH/COMMON/reserve_mem.o
#openair_rf-objs += $(OPENAIR1_TOP)/ARCH/COMMON/amp.o

ifndef OPENAIR_LTE
openair_rf-objs += $(OPENAIR1_TOP)/SCHED/sched.o
else
openair_rf-objs += $(OPENAIR1_TOP)/SCHED/sched_lte.o
openair_rf-objs += $(OPENAIR1_TOP)/SCHED/sched_dlsch.o
endif

# PHY Procedures
# note that ifdef OPENAIR_LTE and ifdef EMOS we use phy_procedures_lte.o
ifdef OPENAIR_LTE 
openair_rf-objs += $(OPENAIR1_TOP)/SCHED/phy_procedures_lte.o
else
ifdef EMOS
openair_rf-objs += $(OPENAIR1_TOP)/SCHED/phy_procedures_emos.o
else
openair_rf-objs += $(OPENAIR1_TOP)/SCHED/phy_procedures.o
endif
endif

openair_rf-objs += $(OPENAIR1_TOP)/MAC_INTERFACE/init.o
openair_rf-objs += $(OPENAIR1_TOP)/MAC_INTERFACE/register.o

openair_rf-objs += cbmimo1_device.o cbmimo1_fileops.o cbmimo1_rf_cntl.o cbmimo1_init.o cbmimo1_dma.o cbmimo1_get_frame.o cbmimo1_generate_fs4.o cbmimo1_generate_ofdm.o cbmimo1_proc.o fifo_printf.o 

#openair_rf-objs += openair_device.o openair_fileops.o openair_rf_cntl.o openair_init.o openair_dma.o openair_get_frame.o  openair_generate_fs4.o openair_generate_ofdm.o


ifdef OPENAIR2
EXTRA_CFLAGS += -DOPENAIR2

openair_rf-objs += l2_init.o
openair_rf-objs +=  $(OPENAIR2_TOP)/UTIL/LISTS/list.o
openair_rf-objs +=  $(OPENAIR2_TOP)/UTIL/LISTS/list2.o

openair_rf-objs +=  $(OPENAIR2_TOP)/UTIL/TIMER/umts_timer.o

openair_rf-objs +=  $(OPENAIR2_TOP)/LAYER2/PDCP/pdcp.o
openair_rf-objs +=  $(OPENAIR2_TOP)/LAYER2/PDCP/pdcp_fifo.o

openair_rf-objs +=  $(OPENAIR2_TOP)/LAYER2/RLC/AM/rlc_am.o
openair_rf-objs +=  $(OPENAIR2_TOP)/LAYER2/RLC/AM/rlc_am_segment.o
openair_rf-objs +=  $(OPENAIR2_TOP)/LAYER2/RLC/AM/rlc_am_fsm.o
openair_rf-objs +=  $(OPENAIR2_TOP)/LAYER2/RLC/AM/rlc_am_mux.o
openair_rf-objs +=  $(OPENAIR2_TOP)/LAYER2/RLC/AM/rlc_am_retrans.o
openair_rf-objs +=  $(OPENAIR2_TOP)/LAYER2/RLC/AM/rlc_am_util.o
openair_rf-objs +=  $(OPENAIR2_TOP)/LAYER2/RLC/AM/rlc_am_mac_status.o
openair_rf-objs +=  $(OPENAIR2_TOP)/LAYER2/RLC/AM/rlc_am_demux.o
openair_rf-objs +=  $(OPENAIR2_TOP)/LAYER2/RLC/AM/rlc_am_receiver.o
openair_rf-objs +=  $(OPENAIR2_TOP)/LAYER2/RLC/AM/rlc_am_reassembly.o
openair_rf-objs +=  $(OPENAIR2_TOP)/LAYER2/RLC/AM/rlc_am_discard_rx.o
openair_rf-objs +=  $(OPENAIR2_TOP)/LAYER2/RLC/AM/rlc_am_control_primitives.o
openair_rf-objs +=  $(OPENAIR2_TOP)/LAYER2/RLC/AM/rlc_am_reset.o
openair_rf-objs +=  $(OPENAIR2_TOP)/LAYER2/RLC/AM/rlc_am_status.o
openair_rf-objs +=  $(OPENAIR2_TOP)/LAYER2/RLC/AM/rlc_am_timers.o
openair_rf-objs +=  $(OPENAIR2_TOP)/LAYER2/RLC/AM/rlc_am_discard_notif.o
openair_rf-objs +=  $(OPENAIR2_TOP)/LAYER2/RLC/AM/rlc_am_discard_tx.o

openair_rf-objs +=  $(OPENAIR2_TOP)/LAYER2/RLC/TM/rlc_tm.o
openair_rf-objs +=  $(OPENAIR2_TOP)/LAYER2/RLC/TM/rlc_tm_control_primitives.o
openair_rf-objs +=  $(OPENAIR2_TOP)/LAYER2/RLC/TM/rlc_tm_fsm.o
openair_rf-objs +=  $(OPENAIR2_TOP)/LAYER2/RLC/TM/rlc_tm_segment.o
openair_rf-objs +=  $(OPENAIR2_TOP)/LAYER2/RLC/TM/rlc_tm_reassembly.o

openair_rf-objs +=  $(OPENAIR2_TOP)/LAYER2/RLC/UM_v6.1.0_LITE/rlc_um.o
openair_rf-objs +=  $(OPENAIR2_TOP)/LAYER2/RLC/UM_v6.1.0_LITE/rlc_um_fsm.o
openair_rf-objs +=  $(OPENAIR2_TOP)/LAYER2/RLC/UM_v6.1.0_LITE/rlc_um_control_primitives.o
openair_rf-objs +=  $(OPENAIR2_TOP)/LAYER2/RLC/UM_v6.1.0_LITE/rlc_um_segment.o
openair_rf-objs +=  $(OPENAIR2_TOP)/LAYER2/RLC/UM_v6.1.0_LITE/rlc_um_reassembly.o
openair_rf-objs +=  $(OPENAIR2_TOP)/LAYER2/RLC/UM_v6.1.0_LITE/rlc_um_receiver.o

openair_rf-objs +=  $(OPENAIR2_TOP)/LAYER2/RLC/rlc_mac.o
openair_rf-objs +=  $(OPENAIR2_TOP)/LAYER2/RLC/rlc.o
openair_rf-objs +=  $(OPENAIR2_TOP)/LAYER2/RLC/rlc_rrc.o
openair_rf-objs +=  $(OPENAIR2_TOP)/LAYER2/RLC/rlc_mpls.o

openair_rf-objs +=  $(OPENAIR2_TOP)/UTIL/MEM/mem_block.o

openair_rf-objs +=  $(OPENAIR2_TOP)/RRC/MESH/main.o $(OPENAIR2_TOP)/RRC/MESH/L2_interface.o $(OPENAIR2_TOP)/RRC/MESH/utils.o 

openair_rf-objs +=  $(OPENAIR2_TOP)/PHY_INTERFACE/mac_phy_primitives.o

openair_rf-objs +=  $(OPENAIR2_TOP)/LAYER2/MAC/openair2_proc.o $(OPENAIR2_TOP)/LAYER2/MAC/layer2_scheduler.o $(OPENAIR2_TOP)/LAYER2/MAC/nodeb_scheduler.o $(OPENAIR2_TOP)/LAYER2/MAC/ue_scheduler.o $(OPENAIR2_TOP)/LAYER2/MAC/main.o $(OPENAIR2_TOP)/LAYER2/MAC/ue_control_plane_procedures.o $(OPENAIR2_TOP)/LAYER2/MAC/nodeb_control_plane_procedures.o $(OPENAIR2_TOP)/LAYER2/MAC/lchan_interface.o $(OPENAIR2_TOP)/LAYER2/MAC/rar_tools.o

openair_rf-objs += $(OPENAIR2_TOP)/RRC/L2_INTERFACE/openair_rrc_L2_interface.o
endif



ifdef OPENAIR1_LTE
openair_rf-objs += $(OPENAIR1_TOP)/PHY/INIT/lte_init.o
openair_rf-objs += $(OPENAIR1_TOP)/PHY/ESTIMATION/lte_sync_time.o
endif
