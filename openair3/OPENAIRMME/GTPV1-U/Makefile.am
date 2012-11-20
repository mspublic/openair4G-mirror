AM_CFLAGS = @ADD_CFLAGS@ 	\
	-I$(top_srcdir)/INTERTASK_INTERFACE	\
	-I$(top_srcdir)/UTILS

noinst_LTLIBRARIES = libgtpv1u.la

libgtpv1u_la_LDFLAGS = -all-static
libgtpv1u_la_SOURCES = \
	gtpv1_u.h		\
	gtpv1_u_message.h	\
	gtpv1_u_message_tx.c	\
	gtpv1_u_task.c

gtpv1_u_test_LDADD = libgtpv1u.la

bin_PROGRAMS = gtpv1_u_test