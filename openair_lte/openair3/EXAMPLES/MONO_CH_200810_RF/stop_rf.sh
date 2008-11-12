cd $OPENAIR1_DIR/USERSPACE_TOOLS/OPENAIR_RF
./openair_rf_cbmimo1 1 4 1
killall chbch_scope
killall sach_scope
sudo rmmod nasmesh
sudo rmmod openair_rrc
sudo rmmod openair_l2
sudo rmmod openair_rf
