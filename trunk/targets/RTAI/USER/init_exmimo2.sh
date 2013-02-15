sudo rmmod openair_rf
sudo insmod $OPENAIR_TARGETS/ARCH/EXMIMO/DRIVER/openair_rf.ko
sudo mknod /dev/openair0 c 127 0
sudo chmod a+rw /dev/openair0
sleep 1

$OPENAIR_TARGETS/ARCH/EXMIMO/USERSPACE/OAI_FW_INIT/updatefw -s 0x43fffff0 -b -f $OPENAIR0_DIR/express-mimo/software/sdr/exmimo2/main 

#$OPENAIR_TARGETS/ARCH/EXMIMO/USERSPACE/OAI_FW_INIT/updatefw -s 0x43fffff0 -b -f $OPENAIR_TARGETS/ARCH/EXMIMO/USERSPACE/OAI_FW_INIT/exmimo1_leon 


#sudo rmmod openair_rf
#sudo insmod $OPENAIR1_DIR/ARCH/CBMIMO1/DEVICE_DRIVER/openair_rf_softmodem.ko

#sudo rmmod nasmesh
#sudo insmod $OPENAIR2_DIR/NAS/DRIVER/MESH/nasmesh.ko

