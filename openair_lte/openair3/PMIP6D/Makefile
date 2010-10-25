#commands
CC=gcc

#flags & variables
#KERNEL_INCLUDE=/usr/src/linux-2.6.20.rtai.bigphys.openairumts.mpls1.956.softhandover.mnm/include
#KERNEL_INCLUDE=./kernel/include
#KERNEL_INCLUDE=/usr/src/linux-headers-`uname -r`/include
KERNEL_INCLUDE=/usr/src/linux/include
#INCLUDES=-I./ -I./include -I./mipl -I./freeradius-client-1.1.6/include -I./freeradius-client-1.1.6 -isystem $(KERNEL_INCLUDE)
INCLUDES=-I./ -I./include -I./mipl -I./mipl/include -isystem $(KERNEL_INCLUDE)  -I/usr/include/ -I./freeradius-client-1.1.6/include -I./freeradius-client-1.1.6
LIBS=-lpthread -ldl -lrt -lfreeradius-client -lpcap
DEFINES= -DHAVE_STRUCT_IP6_EXT -D_GNU_SOURCE -DENABLE_VT -DFSM_DEBUG -DPMIP_CACHE_DEBUG -DPMIP_HANDLER_DEBUG \
-DPACKAGE_BUGREPORT='"URL:http://svn.eurecom.fr/openairsvn/openair3"' \
-DPACKAGE='"PMIP6D"' \
-DPACKAGE_NAME='"Proxy Mobile IPv6 for Linux (Built on top of MIPL)"' \
-DPACKAGE_VERSION='"2.0.2"' \
-DPACKAGE_COPYRIGHT='"(C)"' \
-DUSE_RADIUS
CFLAGS=-Wall -g $(DEFINES)
LDFLAGS=$(LIBS)

#objects & targets
COMMON_OBJS = mipl/hash.o mipl/conf.o mipl/policy.o \
	mipl/pmgr.o mipl/crypto.o mipl/debug.o mipl/mh.o mipl/icmp6.o \
	libmissing/inet6_rth_gettype.o libnetlink/libnetlink.o \
	pmip_dummy.o mipl/bcache.o \
	mipl/keygen.o \
	mipl/bul.o  \
	mipl/rtnl.o \
	mipl/tunnelctl.o \
	mipl/dhaad_ha.o mipl/ha.o mipl/mpdisc_ha.o mipl/proc_sys.o \
	mipl/scan.o mipl/vars.o \
	mipl/dhaad_mn.o mipl/mn.o     mipl/mpdisc_mn.o \
	mipl/retrout.o   mipl/tqueue.o mipl/cn.o mipl/gram.o \
	mipl/movement.o mipl/ndisc.o    mipl/prefix.o \
	mipl/ipsec.o \
	mipl/vt.o mipl/xfrm.o 

#pmip_ro_cache.o
MR_OBJS = pmip6d.o \
	pmip_tunnel.o pmip_cache.o pmip_hnp_cache.o pmip_handler.o pmip_msgs.o pmip_fsm.o  pmip_mag_proc.o pmip_lma_proc.o pmip_pcap.o cmm_if.o $(COMMON_OBJS)
MR_SRCS = $(MR_OBJS,.o=.c)
TARGETS = pmip6d

all: check-kernel $(TARGETS)

pmip6d: $(MR_OBJS)
	$(CC) $(LDFLAGS) $(MR_OBJS)  -o $@

.c.o:
	$(CC) $(CFLAGS) $(INCLUDES) $< -c -o $@

install: $(TARGETS)
	cp pmip6d $(VMS)/CH/hostfs/
	cp pmip6d $(VMS)/MR1/hostfs/
	cp pmip6d $(VMS)/MR2/hostfs/
	cp pmip6d $(VMS)/MN1/hostfs/
	cp pmip6d $(VMS)/MN2/hostfs/
	cp match  $(VMS)/CH/hostfs/


install_inter: $(TARGETS)
	cp pmip6d $(VMS)/CH1/hostfs/
	cp pmip6d $(VMS)/CH2/hostfs/
	cp pmip6d $(VMS)/MR1/hostfs/
	cp pmip6d $(VMS)/MR2/hostfs/

check-kernel:
ifeq ($(KERNEL_INCLUDE),)
	@echo "Please, set correct KERNEL_INCLUDE"; false
else
	@set -e; \
	if [ ! -r $(KERNEL_INCLUDE)/linux/autoconf.h ]; then \
		echo "Please, set correct KERNEL_INCLUDE"; false; fi
endif

clean:
	@rm -f *.o ./mipl/*.o $(TARGETS)




