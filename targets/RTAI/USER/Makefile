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

CFLAGS += -O2
# Debug
#CFLAGS += -O2 -pg
#CFLAGS += -O2 -pg
#EXTRA_CFLAGS += -O0 -g -ggdb
#EXTRA_CFLAGS += -O0 -g -ggdb
CFLAGS += -DDRIVER2013 -I$(OPENAIR_TARGETS)/ARCH/EXMIMO/USERSPACE/LIB/ -I$(OPENAIR_TARGETS)/ARCH/EXMIMO/DEFS -DENABLE_VCD_FIFO

#SRC = lte-enb.c

ifndef RTAI
RTAI=1
endif

ifeq ($(RTAI),1)
CFLAGS += -DENABLE_RTAI_CLOCK -DRTAI
#RTAI_OBJ = sched_dlsch.o sched_ulsch.o sched_rx_pdsch.o  rt_wrapper.o 
RTAI_OBJ = rt_wrapper.o
else
#OBJ = sched_dlsch.o sched_ulsch.o sched_rx_pdsch.o rt_wrapper.o
OBJ = rt_wrapper.o
CFLAGS += -DENABLE_USE_CPU_EXECUTION_TIME
endif


ifndef USRP
USRP=1
endif

ifeq ($(USRP),1)
ifeq ($(RTAI),1)
RTAI_OBJ += lte-enb-usrp.o
else
OBJ += lte-enb-usrp.o
endif

include $(OPENAIR_TARGETS)/ARCH/USRP/USERSPACE/LIB/Makefile.inc
CFLAGS += -I/opt/uhd/include -L/opt/uhd/lib -luhd -lpthread -lstdc++
CFLAGS += -DUSRP
LDFLAGS += -L/opt/uhd/lib -luhd -lpthread -lstdc++ 
endif

ifndef SECU
SECU=1
endif

ifdef SECU
CFLAGS += -DENABLE_SECURITY
LDFLAGS += -lnettle -lssl -lcrypto 
endif


OBJ += $(OPENAIR1_DIR)/SIMULATION/TOOLS/taus.o $(OPENAIR_TARGETS)/SIMU/USER/init_lte.o $(OPENAIR_TARGETS)/ARCH/EXMIMO/USERSPACE/LIB/openair0_lib.o

TOP_DIR = $(OPENAIR1_DIR)
include $(OPENAIR1_DIR)/PHY/Makefile.inc
include $(OPENAIR1_DIR)/SCHED/Makefile.inc

OBJ += $(PHY_OBJS)
ifeq ($(RTAI),1)
RTAI_OBJ += $(SCHED_OBJS)
else
OBJ += $(SCHED_OBJS)
endif

#ifndef rrc_cellular
ifeq ($(rrc_cellular),0)
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

export rrc_cellular

OPENAIR2_TOP = $(OPENAIR2_DIR)
include $(OPENAIR2_DIR)/LAYER2/Makefile.inc
include $(OPENAIR2_DIR)/UTIL/Makefile.inc
include $(OPENAIR2_DIR)/RRC/NAS/Makefile.inc

ifeq ($(RTAI),1)
LOG_OBJS=
RTAI_OBJ+=$(LOG_DIR)/vcd_signal_dumper.o
RTAI_OBJ+=$(LOG_DIR)/log.o
endif

include $(OPENAIR2_DIR)/RRC/LITE/MESSAGES/Makefile.inc

OBJ += $(LOG_OBJS) 

ifndef OPENAIR2
#OPENAIR2=1
endif
ifeq ($(OPENAIR2),1)
ASN1_MSG_OBJS1=$(addprefix $(OPENAIR2_DIR)/RRC/LITE/MESSAGES/, $(ASN1_MSG_OBJS))
OBJ += $(L2_OBJS) $(LIST_OBJ) $(TIMER_OBJ) $(MEM_OBJ) $(OTG_OBJS) $(MATH_OBJS) $(OSA_OBJS)
OBJ += $(OPENAIR1_DIR)/SIMULATION/ETH_TRANSPORT/netlink_init.o
OBJ += $(PDCP_DIR)/pdcp_thread.o
CFLAGS += -DOPENAIR2 -DNO_RRM -DPUCCH -DMAC_CONTEXT=1
endif

CFLAGS += $(L2_incl) $(UTIL_incl)

CFLAGS += -I$(OPENAIR1_DIR) -I$(OPENAIR2_DIR)/RRC/LITE/MESSAGES	#-I$(OPENAIR3_DIR)/MESH -I$(OPENAIR3_DIR)/MESH/RRM

CFLAGS += -DNEW_FFT -DLOG_NO_THREAD
#CFLAGS += -DLOG_NO_THREAD

ifndef XFORMS
XFORMS=1
endif

ifeq ($(XFORMS),1)
CFLAGS += -DXFORMS -I/usr/include/X11
LDFLAGS += -lforms
OBJ += $(OPENAIR1_DIR)/PHY/TOOLS/lte_phy_scope.o
OBJ += stats.o
endif

ifdef SMBV
CFLAGS += -DSMBV
endif

#CPUFLAGS = -mmmx -msse -msse2 -m32 -mssse3 -msse4.1
CPUFLAGS = -mmmx -msse -msse2 -mssse3 -msse4.1
#CFLAGS += -fno-common -mpreferred-stack-boundary=4  
CFLAGS += -Wall -fno-strict-aliasing -DPHYSIM -DUSER_MODE -DPC_TARGET -DPC_DSP -DNB_ANTENNAS_RX=2 -DNB_ANTENNAS_TXRX=2 -DNB_ANTENNAS_TX=2 -DPHY_CONTEXT=1 -rdynamic $(CPUFLAGS) # -Wno-packed-bitfield-compat
CFLAGS += -DOPENAIR_LTE -DENABLE_FXP -DOPENAIR1 #-DDLSCH_THREAD -DULSCH_THREAD

#only for CBMIMO1
ifdef CBMIMO1
CFLAGS += -DFW2011 -DBIT8_TX -DCBMIMO1
else
#only for EXPRESS MIMO
CFLAGS += -DEXMIMO
#CFLAGS += -DEXMIMO -DTIMING_ADVANCE_HW=138 #this is for ExpressMIMO 1
#CFLAGS += -DEXMIMO -DTIMING_ADVANCE_HW=45 #this is for ExpressMIMO 2
endif

ifndef HARD_RT
HARD_RT=0
endif

ifeq ($(HARD_RT),1)
CFLAGS += -DHARD_RT
endif

ifeq ($(EMOS),1)
CFLAGS += -DEMOS -DEMOS_CHANNEL
LDFLAGS += -lgps
endif

ifndef NAS
NAS=1
endif

ifeq ($(NAS),1)
CFLAGS += -DNAS_NETLINK -DLINUX
OBJ += $(NAS_OBJS)
endif

RTAI_CFLAGS += $(shell rtai-config --lxrt-cflags)
LDFLAGS += -lpthread -lm -lforms
ifeq ($(RTAI),1)
CFLAGS += -I/usr/realtime/include -DRTAI
LDFLAGS += $(shell rtai-config --lxrt-ldflags) 
else
LDFLAGS += -lrt
endif

ifndef USE_MME
  USE_MME=R8
endif

ifdef USE_MME
  include $(OPENAIR2_DIR)/S1AP/Makefile.inc
  include $(OPENAIR2_DIR)/S1AP/MESSAGES/Makefile.inc
  CFLAGS += -DENABLE_USE_MME -DENB_MODE
  LDFLAGS += -lsctp
  CFLAGS_S1AP += -DENB_MODE -DENABLE_USE_MME -DEMIT_ASN_DEBUG=1 -DUSER_MODE

  S1AP_Incl += -I$(OPENAIR3_DIR)/OPENAIRMME/SCTP
  S1AP_BUILT_OBJS += $(S1AP_OBJS) $(addprefix $(OPENAIR2_DIR)/S1AP/MESSAGES/, $(S1AP_ASN_MODULE_SOURCES))
  S1AP_BUILT_OBJS += $(OPENAIR3_DIR)/OPENAIRMME/SCTP/sctp_primitives_client.o
endif


ASN1RELDIR=R9.8
ifeq ($(USE_MME), R8)
  ASN1RELDIR=R8.10
else
  CFLAGS_S1AP += -DUPDATE_RELEASE_9
endif
S1AP_DIR=$(OPENAIR2_DIR)/S1AP
ASN1MESSAGESDIR=$(S1AP_DIR)/MESSAGES
ASN1DIR=$(ASN1MESSAGESDIR)/ASN1

ifeq ($(USRP),1)
all: lte-enb-usrp
else
all: lte-enb
endif

$(USRP_OBJ):$(USRP_FILE_OBJ)
	@echo Compiling $<
	@$(CXX) -c  $(USRP_CFLAGS) $(USRP_FILE_OBJ) -o $(USRP_OBJ)

ifeq ($(RTAI),1)
$(RTAI_OBJ): %.o : %.c
	@echo Compiling $< ...
	@$(CC) -c $(CFLAGS) $(EXTRA_CFLAGS) $(RTAI_CFLAGS) -o $@ $<
endif

$(OBJ) $(ASN1_MSG_OBJS1): %.o : %.c
	@echo Compiling $< ...
	@$(CC) -c $(CFLAGS) $(EXTRA_CFLAGS) -o $@ $<

$(ASN1MESSAGESDIR)/s1ap_ieregen.stamp: $(ASN1DIR)/$(ASN1RELDIR)/S1AP-PDU-Contents.asn $(ASN1DIR)/asn1tostruct.py
	echo Timestamp > $@
	(cd $(ASN1DIR) && python $(ASN1DIR)/asn1tostruct.py -f$<)

$(ASN1MESSAGESDIR)/s1ap_asn1regen.stamp: $(ASN1DIR)/$(ASN1RELDIR)/S1AP-CommonDataTypes.asn $(ASN1DIR)/$(ASN1RELDIR)/S1AP-Constants.asn $(ASN1DIR)/$(ASN1RELDIR)/S1AP-IEs.asn $(ASN1DIR)/$(ASN1RELDIR)/S1AP-PDU.asn
	echo Timestamp > $@
	(cd $(ASN1MESSAGESDIR) && asn1c -gen-PER -fnative-types -fskeletons-copy $^)

$(S1AP_BUILT_OBJS): %.o : %.c $(ASN1MESSAGESDIR)/s1ap_asn1regen.stamp
	@echo Compiling $<
	@$(CC) -Wall -c $(CFLAGS_S1AP) $(S1AP_Incl) $(UTIL_incl) -o $@ $<

$(S1AP_DIR)/libs1ap.a: $(ASN1MESSAGESDIR)/s1ap_ieregen.stamp $(ASN1MESSAGESDIR)/s1ap_asn1regen.stamp $(S1AP_BUILT_OBJS)
	@echo Creating $@
	$(AR) rcs $@ $^

$(LFDS_DIR)/bin/liblfds611.a:
	$(MAKE) -C $(LFDS_DIR) -f makefile.linux

ifdef USE_MME
  lte-enb-usrp: $(OBJ) $(USRP_OBJ) $(ASN1_MSG_OBJS1) $(RTAI_OBJ) $(S1AP_DIR)/libs1ap.a $(LFDS_DIR)/bin/liblfds611.a
	$(CC) $(CFLAGS) $(EXTRA_CFLAGS) $(OBJ) $(USRP_OBJ) $(RTAI_OBJ) $(ASN1_MSG_OBJS1) -o lte-enb  $(S1AP_DIR)/libs1ap.a $(LDFLAGS) $(LFDS_DIR)/bin/liblfds611.a
else
  lte-enb-usrp: $(OBJ) $(USRP_OBJ) $(ASN1_MSG_OBJS1) $(RTAI_OBJ) $(LFDS_DIR)/bin/liblfds611.a
	$(CC) $(CFLAGS) $(EXTRA_CFLAGS) $(OBJ) $(USRP_OBJ) $(RTAI_OBJ) $(ASN1_MSG_OBJS1) -o lte-enb  $(LDFLAGS) $(LFDS_DIR)/bin/liblfds611.a
endif

drivers:
	cd $(OPENAIR2_DIR) && make clean && make nasmesh_netlink.ko
	cd $(OPENAIR2_DIR)/NAS/DRIVER/MESH/RB_TOOL && make clean && make
	cd $(OPENAIR_TARGETS)/ARCH/EXMIMO/DRIVER/eurecom && make clean && make 
	cd $(OPENAIR_TARGETS)/ARCH/EXMIMO/USERSPACE/OAI_FW_INIT && make clean && make 

clean:
	rm -rf $(OBJ) $(RTAI_OBJ) $(USRP_OBJ)
	rm -f $(S1AP_BUILT_OBJS)
	rm -f $(S1AP_DIR)/libs1ap.a

cleanasn1:
	rm -f $(ASN1_MSG_OBJS1)
	$(shell cd $(OPENAIR2_DIR)/RRC/LITE/MESSAGES ; rm -f $(ASN_MODULE_SOURCES) $(ASN_MODULE_HEADERS) *.o *.d )
	rm -f $(OPENAIR2_DIR)/RRC/LITE/MESSAGES/Makefile.am.sample
	rm -f $(OPENAIR2_DIR)/RRC/LITE/MESSAGES/Makefile.inc.generated
	rm -f $(OPENAIR2_DIR)/RRC/LITE/MESSAGES/asn1c/ASN1_files/.lock-rel8
	rm -f $(OPENAIR2_DIR)/RRC/LITE/MESSAGES/asn1c/ASN1_files/.lock-rel10

cleanall: clean cleanasn1
	rm -rf condtest synctest lte-softmodem
	rm -rf synctest_eNB synctest_UE

show:
	@echo $(CFLAGS)
	@echo $(EXTRA_CFLAGS)
	@echo $(OBJ) $(RTAI_OBJ)
	@echo rrc_cellular variable is $(rrc_cellular)
	@echo eNB_flag is $(eNB_flag)
	@echo UE_flag is $(UE_flag)
	@echo IS_REL8 is $(IS_REL8)
	@echo IS_REL10 is $(IS_REL10)

beautiful:
	astyle --style=gnu -s2 $(SRC)
