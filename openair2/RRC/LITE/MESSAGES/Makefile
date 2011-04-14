OPENAIR2_TOP = ../../..
include $(OPENAIR2_DIR)/RRC/LITE/MESSAGES/Makefile.inc

EXTRA_CFLAGS += -I$(OPENAIR1_DIR) -I/usr/include -I/usr/realtime/include -DNDEBUG  -D__need_Emath #-D__NO_ASSERT_H__ 

obj-m = asn1_msg_kern.o
asn1_msg_kern-objs = $(ASN1_CONVERTER_OBJS) $(ASN1_MODULE_OBJS) bsearch.o rtai_mem.o

clean:
	rm -rf $(asn1_msg_kern-objs) $(obj-m)

test:
	echo $(asn1_msg_kern-objs) $(obj-m)