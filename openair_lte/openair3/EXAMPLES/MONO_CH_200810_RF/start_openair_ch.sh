
echo Removing openair_rf,openair_l2 and openair_rrc modules
sudo rmmod -f openair_rrc
sudo rmmod -f openair_l2
sudo rmmod -f openair_rf


sudo insmod $OPENAIR1_DIR/ARCH/CBMIMO1/DEVICE_DRIVER/openair_rf_l2.ko
sudo insmod $OPENAIR2_DIR/LAYER2/openair_layer2.ko
sudo insmod $OPENAIR2_DIR/RRC/MESH/openair_RRC.ko

cd $OPENAIR1_DIR/USERSPACE_TOOLS/OPENAIR_RF
source txgains.sh
./openair_rf_cbmimo1 1 0
./openair_rf_cbmimo1 1 6 $CBMIMO1_m10_dBm 
./openair_rf_cbmimo1 1 1 1 0

#watch -n .1 "cat /proc/openair2/lchan_stats ; cat /proc/openair1/bch_stats"

#./stop_rf.sh
