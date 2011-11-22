####################################################
#       USER SPACE CODE GENERATION
####################################################

#---------------------------------------------------
#
#---------------------------------------------------


#LM_LICENSE_FILE=
#export LM_LICENSE_FILE

KERNEL_NAME=$(shell uname -r)

#SUBDIRS_openair_USERRF  =   $(MAIN_RT_DIR) $(CONFIG_DIR)

#openair_USERRFDIR  = openair_RFOBJS
#openair_USERRFLIBS = $(addsuffix /${openair_USERRFDIR}/Lib.a, ${SUBDIRS_openair_USERRF})
#openair_USERRFOBJS = $(addsuffix /${openair_USERRFDIR}/*.o, ${SUBDIRS_openair_USERRF})
#openair_USERRFDEPS = $(addsuffix /${openair_USERRFDIR}/*.d, ${SUBDIRS_openair_USERRF})

mac_sim_no_netlink:
	( cd SIMULATION/USER_TOOLS/LAYER2_SIM && make mac_sim MASTER=1)

mac_sim:
	( cd SIMULATION/USER_TOOLS/LAYER2_SIM && make mac_sim MASTER=1 NETLINK=1 NO_RRM=1)

mac_sim_rrm:
	( cd SIMULATION/USER_TOOLS/LAYER2_SIM && make mac_sim_rrm MASTER=1 NETLINK=1)

mac_sim_mt_cellular:
	( cd SIMULATION/USER_TOOLS/LAYER2_SIM && make mac_sim_mt CELLULAR=1 NO_RRM=1)

mac_sim_rg_cellular:
	( cd SIMULATION/USER_TOOLS/LAYER2_SIM && make mac_sim_rg MASTER=1 CELLULAR=1 NO_RRM=1)

openair_layer2_emul.ko:
	(cd LAYER2 && $(MAKE) V=1 -C /usr/src/linux-$(shell echo `uname -r`)  M=`pwd` NO_RRM=1 PCTARGET=1 RTAI=1 MASTER=1 OPEN_AIR=1 PHYEMUL=1 && mv openair_l2.ko openair_layer2.ko)

openair_layer2_emul_rrm.ko:
	(cd LAYER2 && $(MAKE) V=1 -C /usr/src/linux-$(shell echo `uname -r`)  M=`pwd` PCTARGET=1 RTAI=1 MASTER=1 OPEN_AIR=1 PHYEMUL=1 && mv openair_l2.ko openair_layer2.ko)


openair_layer2.ko:
	(cd LAYER2 && $(MAKE) V=1 -C /usr/src/linux-$(shell echo `uname -r`)  M=`pwd` NO_RRM=1 PCTARGET=1 RTAI=1 MASTER=1 OPEN_AIR=1 OPENAIR1=1 && mv openair_l2.ko openair_layer2.ko)

openair_layer2_rrm.ko:
	(cd LAYER2 && $(MAKE) V=1 -C /usr/src/linux-$(shell echo `uname -r`)  M=`pwd` PCTARGET=1 RTAI=1 MASTER=1 OPEN_AIR=1 OPENAIR1=1 && mv openair_l2.ko openair_layer2.ko)

openair_layer2_2615.ko:
	(cd LAYER2 && $(MAKE) V=1 -C /usr/src/linux-$(shell echo `uname -r`)  M=`pwd` PCTARGET=1 RTAI=1 MASTER=1 OPEN_AIR=1 OPENAIR1=1 && mv openair_l2.ko openair_layer2_2615.ko)


openair_rrc.ko:
	(cd RRC/MESH && $(MAKE) V=1 -C /usr/src/linux-$(shell echo `uname -r`)  M=`pwd` PCTARGET=1 RTAI=1 OPEN_AIR=1 MASTER=1 NO_RRM=1 && mv openair_rrc.ko openair_RRC.ko)

openair_rrc_rrm.ko:
	(cd RRC/MESH && $(MAKE) V=1 -C /usr/src/linux-$(shell echo `uname -r`)  M=`pwd` PCTARGET=1 RTAI=1 OPEN_AIR=1 MASTER=1 && mv openair_rrc.ko openair_RRC.ko)


openair_rrc_2615.ko:
	(cd RRC/MESH && $(MAKE) V=1 -C /usr/src/linux-$(shell echo `uname -r`) M=`pwd` PCTARGET=1 RTAI=1 OPEN_AIR=1 MASTER=1 && mv openair_rrc.ko openair_RRC_2615.ko)

openair_rrc_emul.ko:
	(cd RRC/MESH && $(MAKE) V=1 -C /usr/src/linux-$(shell echo `uname -r`)  M=`pwd` PCTARGET=1 RTAI=1 OPEN_AIR=1 MASTER=1 PHYEMUL=1 NO_RRM=1)

openair_rrc_emul_rrm.ko:
	(cd RRC/MESH && $(MAKE) V=1 -C /usr/src/linux-$(shell echo `uname -r`)  M=`pwd` PCTARGET=1 RTAI=1 OPEN_AIR=1 MASTER=1 PHYEMUL=1)


openair_emul.ko:
	(cd SIMULATION/PHY_EMULATION/DEVICE_DRIVER/ && $(MAKE) V=1 -C /usr/src/linux-$(shell echo `uname -r`)  M=`pwd` PCTARGET=1 RTAI=1 OPEN_AIR=1 MASTER=1 NO_RRM=1)

openair_emul_rrm.ko:
	(cd SIMULATION/PHY_EMULATION/DEVICE_DRIVER/ && $(MAKE) V=1 -C /usr/src/linux-$(shell echo `uname -r`)  M=`pwd` PCTARGET=1 RTAI=1 OPEN_AIR=1 MASTER=1)



create_device:
	rm -f /dev/openair0
	mknod /dev/openair0 c 127 0
	chmod a+rw /dev/openair0


install_emul:
	make devs
	make fifos
	(insmod SIMULATION/PHY_EMULATION/DEVICE_DRIVER/openair_emul.ko)
	(insmod LAYER2/openair_layer2.ko)
	(insmod RRC/MESH/openair_rrc.ko)
	(insmod NAS/DRIVER/MESH/nasmesh.ko)

remove_emul:
	rmmod  nasmesh
	sleep 1
	rmmod  openair_rrc
	sleep 1
	rmmod  openair_l2
	sleep 1
	rmmod  openair_emul




modules_install_platon:
	cp arch/platon/device_driver/openair_rf.ko /lib/modules/$(KERNEL_NAME)/openair/
	depmod -aq


nasmesh_address_fix.ko:
	(cd NAS/DRIVER/MESH && $(MAKE) V=1 ADDRESS_FIX=1 -C /usr/src/linux M=`pwd`)


nasmesh.ko:

	(cd NAS/DRIVER/MESH && $(MAKE) V=1 -C /usr/src/linux M=`pwd`)

#(cd NAS/DRIVER/MESH && $(MAKE) V=1 -C /usr/src/linux-$(shell echo `uname -r`) M=`pwd`)


nasmesh_netlink.ko:
	(cd NAS/DRIVER/MESH && $(MAKE) NAS_NETLINK=1 V=1 -C /usr/src/linux M=`pwd` modules)

nasmesh_netlink_address_fix.ko:
	(cd NAS/DRIVER/MESH && $(MAKE) NAS_NETLINK=1 ADDRESS_FIX=1 V=1 -C /usr/src/linux M=`pwd` modules)

nasmesh_netlink_loopback.ko:
	(cd NAS/DRIVER/MESH && $(MAKE) NAS_NETLINK=1 LOOPBACK=1 V=1 -C /usr/src/linux-$(shell echo `uname -r`) M=`pwd` modules)

rt_emul:
	(cd SIMULATION/USER_TOOLS/RT_EMUL_LAUNCHER && $(MAKE))

bypass_phy:
	(cd SIMULATION/PHY_EMULATION/TRANSPORT/KERNEL_INTERFACE && $(MAKE))

rb_tool:
	(cd NAS/DRIVER/MESH/RB_TOOL && $(MAKE))


# By default, we set 'updatefirmware' option of make to be void
# (if the user specify option updatefirmware=1, this will take precedence
# over the default value)
updatefirmware=0
install_cbmimo1_softmodem:
	make fifos
	if [ $(updatefirmware) = "1" -o $(updatefirmware) = "yes" ] ; then \
      insmod arch/openair_CardBus_MIMO1/device_driver/openair_rf_softmodem.ko updatefirmware=1 ; \
    else \
      insmod arch/openair_CardBus_MIMO1/device_driver/openair_rf_softmodem.ko ; \
    fi
	insmod mac/openair_wireless3g4free/openair_w3g4free.ko	
install_cbmimo1_softmodem_emos:
	make fifos
	if [ $(updatefirmware) = "1" -o $(updatefirmware) = "yes" ] ; then \
      insmod arch/openair_CardBus_MIMO1/device_driver/openair_rf_softmodem_emos.ko updatefirmware=1 ; \
    else \
      insmod arch/openair_CardBus_MIMO1/device_driver/openair_rf_softmodem_emos.ko ; \
    fi

install_cbmimo1_dlc_nodeb:
	make devs
	make fifos
	(cd mac/openair_widens ; ./launch_script_clusterhead)
	make install_cbmimo1_softmodem

install_cbmimo1_dlc_ue:
	make devs
	make fifos
	(cd mac/openair_widens ; ./launch_script_node 5)
	make install_cbmimo1_softmodem

remove_cbmimo1_dlc:
	(cd mac/openair_widens ; rmmod test_dlc_mod ; rmmod dlc_mod ; rmmod ipcs_mod)

remove_cbmimo1:
	rmmod openair_w3g4free
	rmmod openair_rf	
remove_cbmimo1_emos:
	rmmod openair_rf

install_platon:
	insmod arch/platon/device_driver/openair_rf.ko

fifos:
	@for i in `seq 0 64`;\
	do \
	have_rtfX=`ls /dev/ |grep -c rtf$$i`;\
	if [ "$$have_rtfX" -eq 0 ] ;then \
	mknod -m 666 /dev/rtf$$i c 150 $$i; \
	fi;\
	done

#	cp arch/openair_CardBus_MIMO1/device_driver/openair_rf.ko /lib/modules/$(KERNEL_NAME)/openair/
#	depmod -aq

devs:
	if ! test -a /dev/openair0;\
	then \
	mknod /dev/openair0 c 127 0;\
	chmod a+rw /dev/openair0;\
	fi;	

load_sounder_ue: install_cbmimo1_ublaze
	insmod mac/chansounder/openair_chansounder_ue.ko

load_sounder_nodeb: install_platon
	(cd openair_rf && ./openair_rf_platon 1917600 0 ../arch/platon/rbt_files/DAQ_B.rbt 30p 15p 10p 25p && ./openair_rf_platon 1917600 0 ../arch/platon/rbt_files/daq_platon_11_2005.rbt 30p 15p 10p 25p)


stop_sounder: 
	rmmod openair_chansounder
	rmmod openair_rf

stop_sounder_nodeb:
	(cd openair_rf && ./openair_rf_platon 1907600 4)
	rmmod openair_chansounder
	rmmod openair_rf


emul_rt:
	(cd SIMULATION/USER_TOOLS/RT_EMUL_LAUNCHER && $(MAKE))
clean: 
	find -name *.o -delete -print
#find -name *~ -delete -print

cleanall: 
	find -name *.o -delete -print
	find -name *.ko -delete -print
	find -name *~ -delete -print
	find -name *.bak -delete -print
	rm -rf NAS/DRIVER/MESH/RB_TOOL/rb_tool


cleansvn:
	rm -rf `find -name *.svn* -print` ; \
	find -name *.svn* -delete -print

tgz:
	mkdir /tmp/openair2_tmp; \
	$(MAKE) cleanall; \
	cp -r . /tmp/openair2_tmp; \
	(cd /tmp/openair2_tmp && $(MAKE) cleansvn && cd .. && tar czfv openair2.tgz openair2_tmp) ; \
	cp /tmp/openair2.tgz . ;\
	rm -rf /tmp/openair2_tmp

