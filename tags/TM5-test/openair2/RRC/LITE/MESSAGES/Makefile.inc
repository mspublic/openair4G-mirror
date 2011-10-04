
tmp:=$(shell cd $(OPENAIR2_TOP)/RRC/LITE/MESSAGES ; if [ ! -s Makefile.inc.generated ] ; then asn1c -gen-PER -fcompound-names -fnative-types -fskeletons-copy ./asn1c/ASN1_files/EUTRA-RRC-Definitions.asn ; patch -p1 < asn1_patch ; awk '/converter-sample.c/ {exit} // {print}' Makefile.am.sample > Makefile.inc.generated ; fi )

include $(OPENAIR2_TOP)/RRC/LITE/MESSAGES/Makefile.inc.generated

ASN_CONVERTER_SOURCES+=asn1_msg.c
 
EXTRA_CFLAGS += -I. -I$(OPENAIR2_DIR)/RRC/LITE/MESSAGES -g -ggdb -DEMIT_ASN_DEBUG=0 #-Wall
ASN1_MODULE_OBJS=${ASN_MODULE_SOURCES:.c=.o}
ASN1_CONVERTER_OBJS=${ASN_CONVERTER_SOURCES:.c=.o} 
ASN1_MSG_OBJS=$(ASN1_MODULE_OBJS) $(ASN1_CONVERTER_OBJS)

