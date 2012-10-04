CC = gcc

CPUFLAGS = -mmmx -msse -msse2 -m32  #-msse4
CPUFLAGS += $(shell if grep --silent ssse3 /proc/cpuinfo ; then echo "-mssse3" ; else echo ""; fi)
CPUFLAGS += $(shell if grep --silent sse4 /proc/cpuinfo ; then echo "-msse4" ; else echo ""; fi)
linux = $(shell if [ `uname` = "Linux" ] ; then echo "1" ; else echo "0" ; fi)

TOP_DIR      = $(OPENAIR1_DIR)
OPENAIR1_TOP = $(OPENAIR1_DIR)
OPENAIR2_TOP = $(OPENAIR2_DIR)
OPENAIR3_TOP = $(OPENAIR3_DIR)
OPENAIR3     = $(OPENAIR3_DIR)

CFLAGS = -Wall -fno-strict-aliasing -DPHYSIM -DUSER_MODE -DPC_TARGET -DPC_DSP -DNB_ANTENNAS_RX=2 -DNB_ANTENNAS_TXRX=2 -DNB_ANTENNAS_TX=2 -g -ggdb $(CPUFLAGS) -I/usr/include/X11 #-Wno-packed-bitfield-compat 

GCCVERSION = $(shell gcc --version | grep ^gcc | sed 's/^.* //g')
ifeq "$(GCCVERSION)"  "4.4.3"
    CFLAGS += -Wno-packed-bitfield-compat
endif
ifeq "$(GCCVERSION)" "4.5.2"
    CFLAGS += -Wno-packed-bitfield-compat
endif
ifeq "$(GCCVERSION)" "4.6.1"
    CFLAGS += -Wno-packed-bitfield-compat
endif
ifeq "$(GCCVERSION)" "4.6.3"
    CFLAGS += -Wno-packed-bitfield-compat
endif


ASN1_MSG_INC = $(OPENAIR2_DIR)/RRC/LITE/MESSAGES

CFLAGS += -DOPENAIR_LTE -DPUCCH #-DOFDMA_ULSCH -DIFFT_FPGA -DIFFT_FPGA_UE  
CFLAGS += -DMAC_CONTEXT=1 -DPHY_CONTEXT=1 -DPHY_ABSTRACTION #-DRLC_UM_TEST_TRAFFIC=1  

ifndef OPENAIR2
OPENAIR2=1
endif

ifndef OPENAIR_EMU
export OPENAIR_EMU=1
endif

ifdef EMOS
CFLAGS+=-DEMOS
endif

ifdef TRAFFIC_TM5
CFLAGS += -DRLC_UM_TEST_TRAFFIC=1 #-DFULL_BUFFER=1
endif

# activate OCG and libxml only under linux
ifeq ($(linux),1) 
CFLAGS += -I/usr/include/libxml2 -L/usr/local/lib -I/usr/include/atlas -L/usr/X11R6/lib 
OCG_FLAG = 1
cygwin=0
CFLAGS += -DLINUX
else
cygwin=1
OCG_FLAG = 0
endif


ifdef XFORMS
CFLAGS += -DXFORMS
endif

ifdef PRINT_STATS
CFLAGS += -DPRINT_STATS
endif

ifndef rrc_cellular
rrc_cellular = 0
rrc_cellular_eNB = 0
rrc_cellular_UE = 0
else
  ifeq ($(eNB_flag),1)
    rrc_cellular_eNB=1
  endif
  ifeq ($(UE_flag),1)
    rrc_cellular_UE=1
  endif
endif

ifdef DEBUG_PHY
CFLAGS += -DDEBUG_PHY
endif

# ln -sf EUTRA-RRC-Definitions-a20.asn EUTRA-RRC-Definitions.asn
# make cleanasn1
#make all Rel10=1
export IS_REL10=$(shell if [ -f .lock-rel10 ] ; then  echo "1" ; else  echo "0" ; fi)
export IS_REL8=$(shell if [ -f .lock-rel8 ] ; then  echo "1" ; else  echo "0" ; fi)

ifdef  Rel10
CFLAGS += -DRel10
ifeq ($(IS_REL10), 0) 
 $(shell cd $(OPENAIR2_DIR)/RRC/LITE/MESSAGES/asn1c/ASN1_files/ ; rm EUTRA-RRC-Definitions.asn ; ln -s EUTRA-RRC-Definitions-a20.asn EUTRA-RRC-Definitions.asn ; cd $(OPENAIR_TARGETS)/SIMU/USER/ ; rm -f $(ASN1_MSG_OBJS1) ; rm -f $(OPENAIR2_DIR)/RRC/LITE/MESSAGES/Makefile.inc.generated ; rm -f $(L2_OBJS) ; rm -f *.o ; rm -f oaisim ; touch .lock-rel10 ; rm .lock-rel8 ;)
endif
else # default is rel 8
ifeq ($(IS_REL8), 0) 
 $(shell cd $(OPENAIR2_DIR)/RRC/LITE/MESSAGES/asn1c/ASN1_files/ ; rm EUTRA-RRC-Definitions.asn ; ln -s EUTRA-RRC-Definitions-86.asn EUTRA-RRC-Definitions.asn ;  cd $(OPENAIR_TARGETS)/SIMU/USER/ ; rm -f $(ASN1_MSG_OBJS1) ; rm -f $(OPENAIR2_DIR)/RRC/LITE/MESSAGES/Makefile.inc.generated ; rm -f $(L2_OBJS); rm -f *.o ; rm -f oaisim ; touch .lock-rel8 ; rm .lock-rel10 ; )
endif
endif


CFLAGS += -DNAS_NETLINK -DLINUX
NAS_FLAG=1
ifdef TEST_OMG
CFLAGS += -DTEST_OMG
#CFLAGS += -DDEBUG_OMG
endif
# after the execution, a profiling output "gmon.out" will be created, 
#then we need to run "gprof ./oaisim > gmon.txt" in the same directory as "gmon.out"
# you could also use the script "grpof2dot.py gmon.txt" for visual diagrams 
ifdef GPROF
CFLAGS += -pg
endif

#require kernel patch for oai driver to enable address autoconfiguration (IPv6 only)
ifdef ADDCONF
CFLAGS+=-DADDRCONF
endif

CFLAGS += $(shell if [ `uname -o` = "Cygwin" ] ; then echo "-DCYGWIN" ;fi)

CFLAGS += -DENABLE_FXP -DOAI_EMU -DENABLE_USE_CPU_EXECUTION_TIME

include $(OPENAIR1_DIR)/PHY/Makefile.inc
include $(OPENAIR1_DIR)/SCHED/Makefile.inc
include $(OPENAIR2_DIR)/LAYER2/Makefile.inc
include $(OPENAIR1_DIR)/SIMULATION/ETH_TRANSPORT/Makefile.inc
include $(OPENAIR2_DIR)/RRC/LITE/MESSAGES/Makefile.inc
include $(OPENAIR2_DIR)/RRC/NAS/Makefile.inc
include $(OPENAIR2_DIR)/UTIL/Makefile.inc

SIMULATION_OBJS  = $(TOP_DIR)/SIMULATION/TOOLS/gauss.o  
SIMULATION_OBJS += $(TOP_DIR)/SIMULATION/TOOLS/random_channel.o  
SIMULATION_OBJS += $(TOP_DIR)/SIMULATION/TOOLS/rangen_double.o  
SIMULATION_OBJS += $(TOP_DIR)/SIMULATION/TOOLS/taus.o  
SIMULATION_OBJS += $(TOP_DIR)/SIMULATION/TOOLS/multipath_channel.o
SIMULATION_OBJS += $(TOP_DIR)/SIMULATION/TOOLS/abstraction.o
SIMULATION_OBJS += $(TOP_DIR)/SIMULATION/RF/rf.o
SIMULATION_OBJS += $(TOP_DIR)/SIMULATION/RF/adc.o
SIMULATION_OBJS += $(TOP_DIR)/SIMULATION/RF/dac.o

ASN1_MSG_OBJS1=$(addprefix $(OPENAIR2_DIR)/RRC/LITE/MESSAGES/, $(ASN1_MSG_OBJS))
#L2_OBJS =
#EXTRA_CFLAGS =

OAISIM_OBJS = channel_sim.o 
OAISIM_OBJS += init_lte.o 
OAISIM_OBJS += oaisim_config.o
OAISIM_OBJS += sinr_sim.o 
OAISIM_OBJS += cor_SF_sim.o
OAISIM_OBJS += phy_procedures_sim_form.o
ifdef XFORMS
OAISIM_OBJS += $(OPENAIR1_DIR)/USERSPACE_TOOLS/SCOPE/lte_scope.o
#OAISIM_OBJS += lte_scope.o
endif 

ifdef PROC
INT_OBJS +=$(OPENAIR_TARGETS)/SIMU/PROC/channel_sim_proc.o
INT_OBJS +=$(OPENAIR_TARGETS)/SIMU/PROC/Process.o
CFLAGS +=-DPROC
endif 

OBJ = $(PHY_OBJS) $(SIMULATION_OBJS) $(ETHERNET_TRANSPORT_OBJS) $(TOOLS_OBJS) $(SCHED_OBJS) $(STATS_OBJS) $(OAISIM_OBJS) $(NAS_OBJS) $(INT_OBJS) $(UTIL_OBJ)
ifeq ($(OPENAIR2),1)
OBJ += $(L2_OBJS)
endif

all: oaisim

userclean: cleanall oaisim

printvars:
	@echo LINUX variable is $(linux)
	@echo CYGWIN variable is $(cygwin)
	@echo rrc_cellular variable is $(rrc_cellular)
	@echo EXTRA_CFLAGS = $(EXTRA_CFLAGS)
	@echo OAI Config generator is $(OCG_FLAG)	
	@echo OAI NETLINK FLAG is $(NAS_FLAG)
	@echo SSE3 proc is $(SSE3PROC)
	@echo L2 objs are $(L2_OBJS)
	@echo eNB_flag is $(eNB_flag)
	@echo UE_flag is $(UE_flag)

$(OBJ) $(ASN1_MSG_OBJS1) : %.o : %.c
	@echo Compiling $<
	@$(CC) -c $(CFLAGS) $(EXTRA_CFLAGS) -DPHY_CONTEXT=1 -I$(TOP_DIR) $(L2_incl) $(UTIL_incl) -I$(ASN1_MSG_INC) -o $@ $<

oaisim : $(OBJ) $(ASN1_MSG_OBJS1) oaisim.c  
	@echo "Compiling oaisim.c ..."
	@$(CC)  oaisim.c -I$(TOP_DIR) $(L2_incl) $(UTIL_incl) -I$(ASN1_MSG_INC) -o oaisim $(CFLAGS) $(EXTRA_CFLAGS) $(OBJ) $(ASN1_MSG_OBJS1) -lm -lblas -lpthread -llapack_atlas -lforms -lxml2 -lX11 -lXpm -lrt 

oaisimCROWN : $(OBJ) $(ASN1_MSG_OBJS1) oaisimCROWN.c  
	@echo "Compiling oaisimCROWN.c ..."
	@$(CC)  oaisimCROWN.c -I$(TOP_DIR) $(L2_incl) $(UTIL_incl) -I$(ASN1_MSG_INC) -o oaisimCROWN $(CFLAGS) $(EXTRA_CFLAGS) $(OBJ) $(ASN1_MSG_OBJS1) -lm -lblas -lpthread -llapack_atlas -lforms -lxml2 -lX11 -lXpm -lrt 


ifeq ($(rrc_cellular_eNB),1)
	mv oaisim oaisim_eNB
endif
ifeq ($(rrc_cellular_UE),1)
	mv oaisim oaisim_UE
endif

nasmesh_fix:
	(cd $(OPENAIR2_DIR)/NAS/DRIVER/MESH/RB_TOOL && make)
	(cd $(OPENAIR2_DIR) && make nasmesh_netlink_address_fix.ko)
	(sudo insmod $(OPENAIR2_DIR)/NAS/DRIVER/MESH/nasmesh.ko)
nasmesh_nl:
	(cd $(OPENAIR2_DIR)/NAS/DRIVER/MESH/RB_TOOL && make)
	(cd $(OPENAIR2_DIR) && make nasmesh_netlink.ko)	
	(sudo insmod $(OPENAIR2_DIR)/NAS/DRIVER/MESH/nasmesh.ko)
rb_tool:
	(cd $(OPENAIR2_DIR)/NAS/DRIVER/MESH/RB_TOOL && make)

nasmesh_install:
	(sudo rmmod nasmesh)
	(sudo insmod $(OPENAIR2_DIR)/NAS/DRIVER/MESH/nasmesh.ko)

nas_sim_mt_cellular:
	( cd $(OPENAIR2_DIR)/NAS/SIMU_CELLULAR && make nasmt_sim CELLULAR=1 NO_RRM=1)

nas_sim_rg_cellular:
	( cd $(OPENAIR2_DIR)/NAS/SIMU_CELLULAR && make nasrg_sim MASTER=1 CELLULAR=1 NO_RRM=1)

cleanall: clean cleanasn1

clean:
	rm -f oaisim
	rm -f $(OBJ)
	rm -f *.o
	rm -f *.exe*
	rm $(ASN1_MSG_INC)/asn1_msg.o
cleanl1:
	rm -f oaisim
	rm -f $(PHY_OBJS) $(SCHED_OBJS)
	rm -f *.o
	rm -f *.exe

cleanl2:
	rm -f $(L2_OBJS)
	rm $(ASN1_MSG_INC)/asn1_msg.o	
cleanasn1:
	rm -f $(ASN1_MSG_OBJS1)
	rm -f $(OPENAIR2_DIR)/RRC/LITE/MESSAGES/Makefile.inc.generated

cleancell:
	rm -f $(OPENAIR2_DIR)/RRC/CELLULAR/*.o
	rm -f $(OPENAIR2_DIR)/RRC/L2_INTERFACE/*.o
	rm -f $(OPENAIR2_DIR)/NAS/SIMU_CELLULAR/*.o

print:
	@echo $(cygwin)
	@echo $(OCG_FLAG)
	@echo $(CFLAGS)
	@echo $(EXTRA_CFLAGS)
	@echo $(OBJ)
	@echo $(IS_REL10)
	@echo $(IS_REL8)

showcflags:
	@echo $(CFLAGS)
