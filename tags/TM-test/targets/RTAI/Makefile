# command line flags are automatically exported and thus passed down to any sub-make. 
# here we set the default flags in case on command line flags are provided
# these flags have to be explicitely exported so that the sub-make is aware of them


ifndef OPENAIR_LTE
export OPENAIR_LTE=1
export OPENAIR_EMU=0
endif

ifndef OPENAIR1
export OPENAIR1=1
endif

ifndef OPENAIR2
export OPENAIR2=1
endif

ifndef OPENAIR3
export OPENAIR3=1
endif

ifndef EMOS
export EMOS=0
endif

ifndef rrc_cellular
export rrc_cellular = 0
export rrc_cellular_eNB = 0
export rrc_cellular_UE = 0
else
 export rrc_cellular = 1	
  ifeq ($(eNB_flag),1)
    export rrc_cellular_eNB=1
  endif
  ifeq ($(UE_flag),1)
    export rrc_cellular_UE=1
  endif
endif

all: openair_rf_cbmimo1_softmodem.ko asn1_msg_kern.ko nasmesh.ko rb_tool openair_rf scope

openair_rf_cbmimo1_softmodem.ko:  
	(cd $(OPENAIR1_DIR)/ARCH/CBMIMO1/DEVICE_DRIVER && $(MAKE) -C /usr/src/linux V=1 M=`pwd` RTAI=1 CBMIMO1=1 && mv openair_rf.ko openair_rf_softmodem.ko)

asn1_msg_kern.ko:
	(cd $(OPENAIR2_DIR)/RRC/LITE/MESSAGES && $(MAKE) -C /usr/src/linux V=0 M=`pwd` RTAI=1 -j2)

nasmesh.ko:
	(cd $(OPENAIR2_DIR)/NAS/DRIVER/MESH && $(MAKE) V=1 -C /usr/src/linux M=`pwd`)

rb_tool:
	(cd $(OPENAIR2_DIR)/NAS/DRIVER/MESH/RB_TOOL && $(MAKE))

openair_rf:
	(cd $(OPENAIR1_DIR)/USERSPACE_TOOLS/OPENAIR_RF && $(MAKE) clean CBMIMO1=1 && $(MAKE) all CBMIMO1=1 && $(MAKE) clean CBMIMO1=1)

scope:
	(cd $(OPENAIR1_DIR)/USERSPACE_TOOLS/SCOPE && $(MAKE) clean && $(MAKE) all && $(MAKE) clean )	

#Remove all but source files
cleanall: 
	(cd $(OPENAIR1_DIR) && $(MAKE) cleanall)
	(cd $(OPENAIR2_DIR) && $(MAKE) cleanall)

fifos:
	@for i in `seq 0 64`;\
	do \
	have_rtfX=`ls /dev/ |grep -c rtf$$i`;\
	if [ "$$have_rtfX" -eq 0 ] ;then \
	mknod -m 666 /dev/rtf$$i c 150 $$i; \
	fi;\
	done

openair0:
	mknod /dev/openair0 c 127 0
	chmod a+rw /dev/openair0

# By default, we set 'updatefirmware' option of make to be void
# (if the user specify option updatefirmware=1, this will take precedence
# over the default value)
updatefirmware=0
install_cbmimo1_softmodem:
	make fifos
ifeq ($(OPENAIR2),1)
	insmod $(OPENAIR2_DIR)/RRC/LITE/MESSAGES/asn1_msg_kern.ko
endif
	if [ $(updatefirmware) = "1" -o $(updatefirmware) = "yes" ] ; then \
		insmod $(OPENAIR1_DIR)/ARCH/CBMIMO1/DEVICE_DRIVER/openair_rf_softmodem.ko updatefirmware=1 ; \
	else \
		insmod $(OPENAIR1_DIR)/ARCH/CBMIMO1/DEVICE_DRIVER/openair_rf_softmodem.ko ; \
	fi

install_cbmimo1_softmodem1:
	make fifos
	if [ $(updatefirmware) = "1" -o $(updatefirmware) = "yes" ] ; then \
		insmod $(OPENAIR1_DIR)/ARCH/CBMIMO1/DEVICE_DRIVER/openair_rf_softmodem_layer1.ko updatefirmware=1 ; \
	else \
		insmod $(OPENAIR1_DIR)/ARCH/CBMIMO1/DEVICE_DRIVER/openair_rf_softmodem_layer1.ko ; \
	fi

remove_cbmimo1:
	rmmod openair_rf
ifeq ($(OPENAIR2),1)
	rmmod asn1_msg_kern
endif

test:
	(cd $(OPENAIR1_DIR)/ARCH/CBMIMO1/DEVICE_DRIVER && $(MAKE) test RTAI=1 CBMIMO1=1)	
	(cd $(OPENAIR2_DIR)/RRC/LITE/MESSAGES && $(MAKE) test)

clean:
	(cd $(OPENAIR1_DIR)/ARCH/CBMIMO1/DEVICE_DRIVER && $(MAKE) clean RTAI=1 CBMIMO1=1)
	(cd $(OPENAIR2_DIR)/RRC/LITE/MESSAGES && $(MAKE) clean)
