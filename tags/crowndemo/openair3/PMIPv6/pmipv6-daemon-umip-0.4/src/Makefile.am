## $Id: Makefile.am 1.55 06/05/07 17:37:11+03:00 anttit@tcs.hut.fi $
## Process this file with automake to produce Makefile.in

AM_CFLAGS = -Wall -g -Wpointer-arith -Wreturn-type -pedantic -std=gnu99 -DUSE_RADIUS -DCACHE_RADIUS
AM_CPPFLAGS = -isystem ../include -I../src -I./pmip -I/usr/local/include
AM_YFLAGS = -d

defpath.h: Makefile.am
	echo "#define DEFAULT_CONFIG_FILE \"$(sysconfdir)/pmip6d.conf\"" > $@

pmgr.c: mk-pmgr_c.sh pmgr.c.in policy.h
	sh mk-pmgr_c.sh

pmgr.h: mk-pmgr_h.sh pmgr.h.in policy.h
	sh mk-pmgr_h.sh

BUILT_SOURCES = defpath.h pmgr.c pmgr.h
EXTRA_DIST = mk-pmgr_c.sh mk-pmgr_h.sh pmgr.c.in pmgr.h.in

sbin_PROGRAMS = pmip6d

pmip6d_SOURCES = pmgr.c pmgr.h defpath.h \
		main.c vars.c mipv6.h \
		conf.c conf.h \
		gram.y scan.l \
		tqueue.c tqueue.h \
		icmp6.c icmp6.h \
		list.h util.h \
		hash.c hash.h \
		debug.c debug.h \
		dhaad_ha.c dhaad_ha.h \
		dhaad_mn.c dhaad_mn.h \
		ndisc.c ndisc.h \
		mh.c mh.h \
		mn.c mn.h \
		bul.c bul.h \
		bcache.c bcache.h \
		retrout.c retrout.h \
		mpdisc_ha.c mpdisc_ha.h \
		mpdisc_mn.c mpdisc_mn.h \
		tunnelctl.c tunnelctl.h \
		rtnl.c rtnl.h \
		ha.c ha.h \
		cn.c cn.h \
		keygen.c keygen.h \
		movement.c movement.h \
		policy.c policy.h \
		xfrm.c xfrm.h \
		proc_sys.c proc_sys.h \
		prefix.c prefix.h \
		ipsec.c ipsec.h \
		./pmip/pmip.h \
        ./pmip/pmip_cache.c \
        ./pmip/pmip_cache.h \
        ./pmip/pmip_config.h \
        ./pmip/pmip_consts.h \
        ./pmip/pmip_fsm.c \
        ./pmip/pmip_fsm.h \
        ./pmip/pmip_handler.c \
        ./pmip/pmip_handler.h \
        ./pmip/pmip_hnp_cache.c \
        ./pmip/pmip_hnp_cache.h \
        ./pmip/pmip_init.c \
        ./pmip/pmip_init.h \
        ./pmip/pmip_lma_proc.c \
        ./pmip/pmip_lma_proc.h \
        ./pmip/pmip_mag_proc.c \
        ./pmip/pmip_mag_proc.h \
        ./pmip/pmip_msgs.c \
        ./pmip/pmip_msgs.h \
        ./pmip/pmip_pcap.c \
        ./pmip/pmip_pcap.h \
        ./pmip/pmip_tunnel.c \
        ./pmip/pmip_tunnel.h \
        ./pmip/pmip_types.h

if ENABLE_VT
pmip6d_SOURCES += vt.c vt.h
endif

if BUILTIN_CRYPTO
pmip6d_SOURCES += crypto.c crypto.h
endif

pmip6d_LDADD =  -lpcap  -lfreeradius-client

if MISSING
pmip6d_LDADD += -L../libmissing -lmissing
endif

if LOCAL_NETLINK
pmip6d_LDADD += -L../libnetlink -lnetlink
endif

noinst_HEADERS = ../include/netinet/icmp6.h \
		 ../include/netinet/in.h \
		 ../include/netinet/ip6.h \
		 ../include/netinet/ip6mh.h
