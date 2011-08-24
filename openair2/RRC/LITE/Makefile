#/*________________________Makefile________________________

# Authors : Hicham Anouar
# Company : EURECOM
# Emails  : anouar@eurecom.fr
#________________________________________________________________*/


CCC = gcc
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
EXTRA_CFLAGS = $(shell rtai-config --module-cflags) -DRTAI_ENABLED -DPC_TARGET -Wall -D__KERNEL__ -DMODULE -D_LOOSE_KERNEL_NAMES -I/lib/modules/$(shell uname -r)/build/include -I/lib/modules/$(shell uname -r)/build/include/asm/mach-default  $(if $(SET_REGPARM),-mregparm=3 -fno-stack-protector -mpreferred-stack-boundary=4,) $(if $(IS_RTAI_SUBVERSION_LESS_THAN_FIVE),-DRTAI_ISNT_POSIX,)
endif
CCC=gcc
#--------------------------------------------------------

AS_RLC_UM_DIR                 =../../LAYER2/RLC/UM_v6.1.0_LITE
AS_RLC_AM_DIR                 =../../LAYER2/RLC/AM
AS_RLC_TM_DIR                 =../../LAYER2/RLC/TM
AS_RLC_DIR                    =../../LAYER2/RLC
AS_PDCP_DIR                   =../../LAYER2/PDCP
AS_MEM_DIR                    =../../UTIL/MEM
AS_LIST_DIR                   =../../UTIL/LISTS
AS_MATH_DIR                   =../../UTIL/MATH
AS_TIMER_DIR                  =../../UTIL/TIMER
AS_RRM_CONFIG_DIR             =../../COMMON

export AS_RLC_UM_DIR
export AS_RLC_AM_DIR
export AS_RLC_TM_DIR
export AS_RLC_DIR
export AS_PDCP_DIR
export AS_MPLS_DIR
export AS_MEM_DIR
export AS_LIST_DIR
export AS_MATH_DIR
export AS_TIMER_DIR
export AS_RRM_CONFIG_DIR

ifdef MASTER
EXTRA_CFLAGS += -DNODE_RG
else
EXTRA_CFLAGS += -DNODE_MT
endif

ifdef PHYEMUL
EXTRA_CFLAGS += -DPHY_EMUL
else
EXTRA_CFLAGS += -DOPENAIR2 
endif

ifdef NO_RRM
EXTRA_CFLAGS += -DNO_RRM
else
ifndef USER_MODE
EXTRA_CFLAGS += -DRRC_RRM_FIFOS_XFACE
endif
endif

EXTRA_CFLAGS +=   -DKERNEL$(KERNEL_MAIN_TYPE)  -DBIGPHYSAREA -DNB_ANTENNAS_RX=2 -DNB_ANTENNAS_TX=2  -DNB_ANTENNAS_RXTX=2 # -DNO_RRM #-DRRC_RRM_XFACE -DRRC_RRM_FIFOS_XFACE

EXTRA_CFLAGS += -Wall -Wstrict-prototypes  $(if $(SET_X64),-mcmodel=kernel,) $(if $(SET_X64),-DARCH_64,) $(if $(SET_X64),-m64,) -fno-common -fno-strict-aliasing -pipe -freg-struct-return -ffreestanding -maccumulate-outgoing-args -funroll-loops -mmmx -msse -msse2 -fomit-frame-pointer -nostdinc -DMODULE  -D_LOOSE_KERNEL_NAMES -O2
#-mregparm=3 -fno-stack-protector -mpreferred-stack-boundary=4

EXTRA_CFLAGS += -I$(obj)/$(AS_RRM_CONFIG_DIR) -I$(obj)/$(AS_TIMER_DIR) -I$(obj)/$(AS_MATH_DIR) -I$(obj)/$(AS_LIST_DIR) -I$(obj)/$(AS_MEM_DIR) -I$(obj)/$(AS_RLC_DIR) -I$(obj)/$(AS_RLC_UM_DIR) -I$(obj)/$(AS_RLC_TM_DIR) -I$(obj)/$(AS_RLC_AM_DIR) -I$(obj)/$(AS_PDCP_DIR) -I$(obj)/$(AS_MPLS_DIR) -I$(OPENAIR2_DIR) -I$(OPENAIR1_DIR) -I$(OPENAIR3_DIR)/MESH -I$(OPENAIR3_DIR)/MESH/RRM 

obj-m += openair_rrc.o

openair_rrc-objs +=  mesh_top.o 

ifndef NO_RRM
openair_rrc-objs +=  rrc_config.o rrc_2_rrm_msg.o rrm_2_rrc_msg.o rrc_rrm_interface.o 
endif

openair_rrc-objs += main.o L2_interface.o utils.o ../L2_INTERFACE/openair_rrc_L2_interface.o











