# command line flags are automatically exported and thus passed down to any sub-make. 
# here we set the default flags in case on command line flags are provided
# these flags have to be explicitely exported so that the sub-make is aware of them


ifndef OPENAIR_LTE
export OPENAIR_LTE=1
endif

ifndef OPENAIR1
export OPENAIR1=1
endif

ifndef OPENAIR2
export OPENAIR2=1
endif

ifndef EMOS
export EMOS=0
endif

all: openair_rf_cbmimo1_softmodem.ko asn1_msg_kern.ko

openair_rf_cbmimo1_softmodem.ko:  
	(cd ARCH/CBMIMO1/DEVICE_DRIVER && $(MAKE) -C /usr/src/linux V=1 M=`pwd` RTAI=1 CBMIMO1=1 && mv openair_rf.ko openair_rf_softmodem.ko)

asn1_msg_kern.ko:
	(cd ../openair2/RRC/LITE/MESSAGES && $(MAKE) -C /usr/src/linux V=0 M=`pwd` RTAI=1 -j2)

#Remove all but source files
cleanall: 
	find -name '*.o' -delete -print; \
	find -name '*.ko' -delete -print; \
	find -name '*.bak' -delete -print; \
	find -name '*.cmd' -delete -print; \
	find -name '*~' -delete -print; \
	find -name octave-core -delete -print; \

#Remove files created by SVN, dangerous!!! DO NOT RUN ON YOUR WORKING REPOSITORY!
#FK: Don't do this! Use svn export instead.
#cleansvn:
#	rm -rf `find -name *.svn* -print`; \
#	find -name *.svn* -delete -print

tgz:
	mkdir /tmp/openair1_tmp; \
	$(MAKE) cleanall; \
	cp -r . /tmp/openair1_tmp; \
	(cd /tmp/openair1_tmp && $(MAKE) cleansvn && cd .. && tar czfv openair1.tgz openair1_tmp) ; \
	cp /tmp/openair1.tgz . ;\
	rm -rf /tmp/openair1_tmp

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
	insmod ../openair2/RRC/LITE/MESSAGES/asn1_msg_kern.ko
endif
	if [ $(updatefirmware) = "1" -o $(updatefirmware) = "yes" ] ; then \
		insmod ARCH/CBMIMO1/DEVICE_DRIVER/openair_rf_softmodem.ko updatefirmware=1 ; \
	else \
		insmod ARCH/CBMIMO1/DEVICE_DRIVER/openair_rf_softmodem.ko ; \
	fi

install_cbmimo1_softmodem1:
	make fifos
	if [ $(updatefirmware) = "1" -o $(updatefirmware) = "yes" ] ; then \
		insmod ARCH/CBMIMO1/DEVICE_DRIVER/openair_rf_softmodem_layer1.ko updatefirmware=1 ; \
	else \
		insmod ARCH/CBMIMO1/DEVICE_DRIVER/openair_rf_softmodem_layer1.ko ; \
	fi

remove_cbmimo1:
	rmmod openair_rf
ifeq ($(OPENAIR2),1)
	rmmod asn1_msg_kern
endif

test:
	(cd ARCH/CBMIMO1/DEVICE_DRIVER && $(MAKE) test RTAI=1 CBMIMO1=1)	
	(cd ../openair2/RRC/LITE/MESSAGES && $(MAKE) test)

clean: clean_cbmimo1 clean_asn1

clean_cbmimo1:
	(cd ARCH/CBMIMO1/DEVICE_DRIVER && $(MAKE) clean RTAI=1 CBMIMO1=1)

clean_asn1:
	(cd ../openair2/RRC/LITE/MESSAGES && $(MAKE) clean)
