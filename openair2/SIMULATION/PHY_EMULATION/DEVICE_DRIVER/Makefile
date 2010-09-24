CCC = gcc


TOP_DIR = ../../..

# Get the RTAI variables


KERNEL_MAIN_TYPE=$(shell echo `uname -r | cut -d. -f-2  | tr "." "_"`)
export KERNEL_MAIN_TYPE

SUBVERSION=$(shell echo `grep '^SUBLEVEL =' /usr/src/linux/Makefile | sed -e 's, ,,g' | sed -e 's/SUBLEVEL=//'`)
IS_KERNEL_SUBVERSION_GREATER_THAN_20=$(shell if [ $(SUBVERSION) -ge 20 ] ; then echo true ; fi)
KERNEL_ARCH=$(shell echo `uname -m`)
SET_REGPARM=$(shell if [ $(KERNEL_ARCH) = 'i686' -a $(SUBVERSION) -ge 20 ]; then echo true ; fi)
SET_X64=$(shell if [ $(KERNEL_ARCH) = 'x86_64' -a $(SUBVERSION) -ge 20 ]; then echo true ; fi)
ifdef RTAI
# Get the RTAI variables
CCC = $(shell rtai-config --cc)
RTAI_SUBVERSION=$(shell rtai-config --version | sed -e 's/^..\(.\).*$$/\1/')
IS_RTAI_SUBVERSION_LESS_THAN_FIVE=$(shell if [ $(RTAI_SUBVERSION) -lt 5 ] ; then echo true ; fi)

CFLAGS = $(shell rtai-config --module-cflags) -g -DRTAI_ENABLED -DPC_TARGET -Wall -D__KERNEL__ -DMODULE -D_LOOSE_KERNEL_NAMES -I/lib/modules/$(shell uname -r)/build/include -I/lib/modules/$(shell uname -r)/build/include/asm/mach-default -DNB_ANTENNAS=2 -DNB_ANTENNAS_RX=2 -DNB_ANTENNAS_TX=2 -DNB_ANTENNAS_TXRX -DPHY_CONTEXT -I/usr/include $(if $(SET_REGPARM),-mregparm=3 -fno-stack-protector -mpreferred-stack-boundary=4,) $(if $(IS_RTAI_SUBVERSION_LESS_THAN_FIVE),-DRTAI_ISNT_POSIX,)
endif
CCC=gcc





EXTRA_CFLAGS +=   -DKERNEL$(KERNEL_MAIN_TYPE) -DNB_MODULES_MAX=32 -DBIGPHYSAREA -DPHY_CONTEXT -DPHY_EMUL


EXTRA_CFLAGS += -Wall -Wstrict-prototypes $(if $(SET_X64),-fno-stack-protector,)  $(if $(SET_X64),-DARCH_64,) $(if $(SET_X64),-mcmodel=kernel,) $(if $(SET_X64),-m64,) -fno-common -fno-strict-aliasing -pipe -freg-struct-return -ffreestanding -maccumulate-outgoing-args -funroll-loops -mmmx -msse -msse2 -fomit-frame-pointer -nostdinc -DMODULE  -D_LOOSE_KERNEL_NAMES
#-DMAX_MODULES=8 -DBIGPHYSAREA

#
ifdef NO_RRM
EXTRA_CFLAGS += -DNO_RRM
else
ifndef USER_MODE
EXTRA_CFLAGS += -DRRC_RRM_FIFOS_XFACE
endif
endif

ifdef DEBUG_PHY
EXTRA_CFLAGS += -DDEBUG_PHY
endif

ifdef MASTER
EXTRA_CFLAGS += -DNODE_RG
endif

CFLAGS += -I$(OPENAIR2_DIR)  -I$(OPENAIR2_DIR)/COMMON -I$(OPENAIR2_DIR)/PHY_INTERFACE -I$(OPENAIR2_DIR)/UTIL/TIMER -I$(OPENAIR2_DIR)/UTIL/MATH -I$(OPENAIR2_DIR)/UTIL/LISTS -I$(OPENAIR2_DIR)/UTIL/MEM -I$(OPENAIR2_DIR)/LAYER2/RLC -I$(OPENAIR2_DIR)/LAYER2/RLC/UM_v6.1.0_LITE -I$(OPENAIR2_DIR)/LAYER2/RLC/TM -I$(OPENAIR2_DIR)/LAYER2/RLC/AM -I$(OPENAIR2_DIR)/LAYER2/PDCP -I$(OPENAIR2_DIR) -I$(OPENAIR3_DIR)/MESH -I$(OPENAIR3_DIR)/MESH/RRM 

obj-m += openair_emul.o


openair_emul-objs += emul_device.o emul_fileops.o emul_init.o emul_register.o

# PHY Interface routines
openair_emul-objs += $(TOP_DIR)/PHY_INTERFACE/mac_phy_primitives.o

# PHY Abstraction 
openair_emul-objs += $(TOP_DIR)/SIMULATION/PHY_EMULATION/ABSTRACTION/radio_emulation.o 
openair_emul-objs += $(TOP_DIR)/SIMULATION/PHY_EMULATION/ABSTRACTION/dB_routines.o
openair_emul-objs += $(TOP_DIR)/SIMULATION/PHY_EMULATION/ABSTRACTION/gauss.o
openair_emul-objs += $(TOP_DIR)/SIMULATION/PHY_EMULATION/ABSTRACTION/log2_approx.o
openair_emul-objs += $(TOP_DIR)/SIMULATION/PHY_EMULATION/ABSTRACTION/taus.o
openair_emul-objs += $(TOP_DIR)/SIMULATION/PHY_EMULATION/ABSTRACTION/signal_energy.o

# PHY Emulation
openair_emul-objs += $(TOP_DIR)/SIMULATION/PHY_EMULATION/CONTROL/phy_emulation.o

# Top-level scheduler
openair_emul-objs += $(TOP_DIR)/SIMULATION/PHY_EMULATION/SCHED/sched.o

# Transport 
openair_emul-objs += $(TOP_DIR)/SIMULATION/PHY_EMULATION/TRANSPORT/bypass_session_layer.o

# UTIL
openair_emul-objs += $(TOP_DIR)/UTIL/BIGPHYS/bigphys.o


