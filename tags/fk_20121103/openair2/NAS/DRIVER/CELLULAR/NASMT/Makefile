# NASMT Driver makefile
#
NAS_UPDIR	:= $(shell /bin/pwd)
RH_DIR=$(TDCDMA_SRC_PATH)
AS := $(RH_DIR)/src/access_stratum
# KERNEL_MAIN_TYPE=$(shell echo `uname -r | cut -d. -f-2  | tr "." "_"`)
# export KERNEL_MAIN_TYPE

####################################################
#      NASMT compilation flags
####################################################
RTAI=1
N_ANT=1
NODE_MT= 1

####################################################
#  D E B U G   F L A G S
####################################################

ifdef NODE_MT
EXTRA_CFLAGS += -DNODE_MT
endif

####################################################
#      EXTRA COMPILER FLAGS
####################################################
EXTRA_CFLAGS += -I$(TIMER_DIR)
EXTRA_CFLAGS += -I$(RAB_MUX_DIR)
EXTRA_CFLAGS += -I$(SCHED_DIR)
EXTRA_CFLAGS += -I$(LIST_DIR)
EXTRA_CFLAGS += -I$(MEM_DIR)
EXTRA_CFLAGS += -I$(MATH_DIR)
EXTRA_CFLAGS += -I$(TRACE_DIR)
EXTRA_CFLAGS += -I$(RF_DIR)
EXTRA_CFLAGS += -I$(L2_DIR)
EXTRA_CFLAGS += -I$(RLC_DIR)
EXTRA_CFLAGS += -I$(RLC_UM_DIR)
EXTRA_CFLAGS += -I$(RLC_TM_DIR)
EXTRA_CFLAGS += -I$(RLC_AM_DIR)
EXTRA_CFLAGS += -I$(PDCP_DIR)
EXTRA_CFLAGS += -I$(MAC_DIR)
EXTRA_CFLAGS += -I$(RB_DIR)
EXTRA_CFLAGS += -I$(RRC_DIR)
EXTRA_CFLAGS += -I$(RRC_CONTRIB_DIR)
EXTRA_CFLAGS += -I$(L1L_CONTROL_DIR)
EXTRA_CFLAGS += -I$(L1L_DSP_DIR)
EXTRA_CFLAGS += -I$(L1H_CONTROL_DIR)
EXTRA_CFLAGS += -I$(L1H_DSP_DIR)
EXTRA_CFLAGS += -I$(L1_DIR)
EXTRA_CFLAGS += -I$(PROBE_DIR)
EXTRA_CFLAGS += -I$(MODULE_DIR)

EXTRA_CFLAGS += -I$(NAS_UPDIR)
EXTRA_CFLAGS += -DNB_ANTENNAS=$(N_ANT)
# EXTRA_CFLAGS += -DKERNEL$(KERNEL_MAIN_TYPE)
EXTRA_CFLAGS += -fno-common
EXTRA_CFLAGS += $(shell rtai-config --module-cflags) -DRTAI -D__IN_RTAI__ -Wall


####################################################
#      LOADABLE MODULE GOALS
####################################################
obj-m += nasmt.o
nasmt-objs += nasmt_device.o
nasmt-objs += nasmt_common.o
nasmt-objs += nasmt_iocontrol.o
nasmt-objs += nasmt_classifier.o
nasmt-objs += nasmt_tool.o

ifdef NODE_MT
nasmt-objs += nasmt_ascontrol.o
endif

####################################################
#      REVOIR LE CLEAN
####################################################
