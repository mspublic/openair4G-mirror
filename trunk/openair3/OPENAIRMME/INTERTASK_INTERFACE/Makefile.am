AM_CFLAGS = @ADD_CFLAGS@ 	\
	-I${top_srcdir}/UTILS

noinst_LTLIBRARIES = libitti.la

libitti_la_LDFLAGS = -all-static
libitti_la_SOURCES = \
	gtpu_messages_def.h	gtpu_messages_id.h	\
	s1ap_messages_def.h	s1ap_messages_id.h	\
	sctp_messages_def.h	sctp_messages_id.h	\
	intertask_interface.c 	intertask_interface.h	\
	intertask_interface_dump.c intertask_interface_dump.h