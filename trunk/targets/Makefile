#ifdef cygwin
#  cyg=1
#else
#  cyg=0
#endif

help:
	@echo make check1 performs a simple sanity check
	@echo make check2 performs another simple sanity check
	@echo make check does both the above checks
 
all: help

check1:
	@echo Doing 1 eNB 1 UE test
	(cd SIMU/EXAMPLES/VIRT_EMUL_1eNB && make one_eNB_one_UE )

check2:
	@echo Doing 1 eNB 4 UE test
	(cd SIMU/EXAMPLES/VIRT_EMUL_1eNB && make one_eNB_four_UE )

install:
	@echo Generating ASN1 files
	@which asn1c > /dev/null 
	@shell if [ $? -eq 0 ] ; then echo "asn1c is installed" ; else echo "Please install asn1c (version 0.9.22 or greater)" ; fi
	(cd $(OPENAIR2_DIR)/RRC/LITE/MESSAGES && asn1c -gen-PER -fcompound-names -fnative-types -fskeletons-copy $(OPENAIR2_DIR)/RRC/LITE/MESSAGES/asn1c/ASN1_files/EUTRA-RRC-Definitions.asn)

check: check1 check2

clean:
	(cd SIMU/USER && make clean)
