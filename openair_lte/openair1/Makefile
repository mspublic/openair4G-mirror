openair_rf_cbmimo1_softmodem.ko:  
	(cd ARCH/CBMIMO1/DEVICE_DRIVER && $(MAKE) V=1 -C /usr/src/linux  M=`pwd` RTAI=1 && mv openair_rf.ko openair_rf_softmodem.ko)

openair_rf_cbmimo1_softmodem_lte.ko:
	(cd ARCH/CBMIMO1/DEVICE_DRIVER && $(MAKE) V=1 -C /usr/src/linux  M=`pwd` RTAI=1 OPENAIR_LTE=1 && mv openair_rf.ko openair_rf_softmodem_lte.ko)

openair_rf_cbmimo1_softmodem_lte_emos.ko:
	(cd ARCH/CBMIMO1/DEVICE_DRIVER && $(MAKE) V=1 -C /usr/src/linux  M=`pwd` RTAI=1 OPENAIR_LTE=1 EMOS=1 && mv openair_rf.ko openair_rf_softmodem_lte_emos.ko)

openair_rf_cbmimo1_l2.ko:
	(cd ARCH/CBMIMO1/DEVICE_DRIVER && $(MAKE) V=1 -C /usr/src/linux  M=`pwd` RTAI=1 DUALSTREAM=1 OPENAIR2=1 && mv openair_rf.ko openair_rf_l2.ko)

openair_rf_cbmimo1_l2_2615.ko:
	(cd ARCH/CBMIMO1/DEVICE_DRIVER && $(MAKE) V=1 -C /usr/src/linux  M=`pwd` RTAI=1 OPENAIR2=1 && mv openair_rf.ko openair_rf_l2_2615.ko)


openair_rf_cbmimo1_softmodem_duo.ko:  
	(cd ARCH/CBMIMO1/DEVICE_DRIVER/ && $(MAKE) V=1 -C /usr/src/linux  M=`pwd` RTAI=1 DUALSTREAM=1 && mv openair_rf.ko openair_rf_softmodem_duo.ko)

openair_rf_cbmimo1_softmodem_emos.ko:  
	(cd ARCH/CBMIMO1/DEVICE_DRIVER && $(MAKE) V=1 -C /usr/src/linux  M=`pwd` RTAI=1 EMOS=1 && mv openair_rf.ko openair_rf_softmodem_emos.ko)

openair_rf_cbmimo1_softmodem_emos_duo.ko:  
	(cd ARCH/CBMIMO1/DEVICE_DRIVER && $(MAKE) V=1 -C /usr/src/linux  M=`pwd` RTAI=1 EMOS=1 DUALSTREAM=1 && mv openair_rf.ko openair_rf_softmodem_emos_duo.ko)

openair_rf_cbmimo1_softmodem_idromelrf.ko:
	(cd ARCH/CBMIMO1/DEVICE_DRIVER && $(MAKE) V=1 -C /usr/src/linux  M=`pwd` RTAI=1 OPEN_AIR=1 NO_TXMUX=1 IDROMEL_RF=1 && mv openair_rf.ko openair_rf_softmodem.ko)

#Remove all but source files
cleanall: 
	find -name '*.o' -delete -print; \
	find -name '*.ko' -delete -print; \
	find -name '*.bak' -delete -print; \
	find -name '*.cmd' -delete -print; \
	find -name '*~' -delete -print; \
	find -name octave-core -delete -print; \
	rm -f USERSPACE_TOOLS/OPENAIR_RF/openair_rf_cbmimo1; \
	rm -f USERSPACE_TOOLS/SCOPE/chbch_scope; \
	rm -f USERSPACE_TOOLS/SCOPE/sach_scope; \
	rm -f SIMULATION/PHY/CHBCH/chbch_sim; \
	rm -f SIMULATION/PHY/SACH/sach_sim; \

#Remove intermediate files (.o,.bak,~,etc)
clean:
	find -name '*.o' -delete -print; \
	find -name '*.bak' -delete -print; \
	find -name '*~' -delete -print; \
	find -name 'octave-core' -delete -print; 

#Remove files created by SVN, dangerous!!! DO NOT RUN ON YOUR WORKING REPOSITORY!
cleansvn:
	rm -rf `find -name *.svn* -print`; \
	find -name *.svn* -delete -print

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
	if [ $(updatefirmware) = "1" -o $(updatefirmware) = "yes" ] ; then \
		insmod ARCH/CBMIMO1/DEVICE_DRIVER/openair_rf_softmodem.ko updatefirmware=1 ; \
	else \
		insmod ARCH/CBMIMO1/DEVICE_DRIVER/openair_rf_softmodem.ko ; \
	fi

install_cbmimo1_softmodem_lte:
	make fifos
	if [ $(updatefirmware) = "1" -o $(updatefirmware) = "yes" ] ; then \
		insmod ARCH/CBMIMO1/DEVICE_DRIVER/openair_rf_softmodem_lte.ko updatefirmware=1 ; \
	else \
		insmod ARCH/CBMIMO1/DEVICE_DRIVER/openair_rf_softmodem_lte.ko ; \
	fi

install_cbmimo1_softmodem_lte_emos:
	make fifos
	if [ $(updatefirmware) = "1" -o $(updatefirmware) = "yes" ] ; then \
		insmod ARCH/CBMIMO1/DEVICE_DRIVER/openair_rf_softmodem_lte_emos.ko updatefirmware=1 ; \
	else \
		insmod ARCH/CBMIMO1/DEVICE_DRIVER/openair_rf_softmodem_lte_emos.ko ; \
	fi

install_cbmimo1_l2:
	make fifos
	if [ $(updatefirmware) = "1" -o $(updatefirmware) = "yes" ] ; then \
		insmod ARCH/CBMIMO1/DEVICE_DRIVER/openair_rf_l2.ko updatefirmware=1 ; \
	else \
		insmod ARCH/CBMIMO1/DEVICE_DRIVER/openair_rf_l2.ko ; \
	fi
	insmod ../openair2/LAYER2/openair_layer2.ko
	insmod ../openair2/RRC/MESH/openair_RRC.ko

install_cbmimo1_l2_2615:
	make fifos
	if [ $(updatefirmware) = "1" -o $(updatefirmware) = "yes" ] ; then \
		insmod ARCH/CBMIMO1/DEVICE_DRIVER/openair_rf_l2_2615.ko updatefirmware=1 ; \
	else \
		insmod ARCH/CBMIMO1/DEVICE_DRIVER/openair_rf_l2_2615.ko ; \
	fi
	insmod ../openair2/LAYER2/openair_layer2_2615.ko
	insmod ../openair2/RRC/MESH/openair_RRC_2615.ko


install_cbmimo1_softmodem_duo:
	make fifos
	if [ $(updatefirmware) = "1" -o $(updatefirmware) = "yes" ] ; then \
		insmod ARCH/CBMIMO1/DEVICE_DRIVER/openair_rf_softmodem_duo.ko updatefirmware=1 ; \
	else \
		insmod ARCH/CBMIMO1/DEVICE_DRIVER/openair_rf_softmodem_duo.ko ; \
	fi

install_cbmimo1_softmodem_emos:
	make fifos
	if [ $(updatefirmware) = "1" -o $(updatefirmware) = "yes" ] ; then \
		insmod ARCH/CBMIMO1/DEVICE_DRIVER/openair_rf_softmodem_emos.ko updatefirmware=1 ; \
	else \
		insmod ARCH/CBMIMO1/DEVICE_DRIVER/openair_rf_softmodem_emos.ko ; \
	fi

install_cbmimo1_softmodem_emos_duo:
	make fifos
	if [ $(updatefirmware) = "1" -o $(updatefirmware) = "yes" ] ; then \
		insmod ARCH/CBMIMO1/DEVICE_DRIVER/openair_rf_softmodem_emos_duo.ko updatefirmware=1 ; \
	else \
		insmod ARCH/CBMIMO1/DEVICE_DRIVER/openair_rf_softmodem_emos_duo.ko ; \
	fi

remove_cbmimo1:
	rmmod openair_rf

remove_cbmimo1_l2:
	rmmod openair_rrc
	rmmod openair_l2
	rmmod openair_rf
