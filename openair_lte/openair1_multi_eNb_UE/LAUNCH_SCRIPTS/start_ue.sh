cd ../USERSPACE_TOOLS/OPENAIR_RF
./openair_rf_cbmimo1_lte 0 4 1
#rmmod -f openair_rf
#cd ../..
#make install_cbmimo1_softmodem_lte_emos_l2
#cd USERSPACE_TOOLS/OPENAIR_RF
./openair_rf_cbmimo1_lte 0 0 0 1
./openair_rf_cbmimo1_lte 0 6 120 120 120 120
#./openair_rf_cbmimo1_lte 0 25 0 
./openair_rf_cbmimo1_lte 0 15 117 # Card v15 synched to card v5
#./openair_rf_cbmimo1_lte 0 15 135 # Card v37 synched to card v5
#./openair_rf_cbmimo1_lte 0 26 400
#./openair_rf_cbmimo1_lte 0 14 0 #RF mode 0 = mixer low gain, lna off
#./openair_rf_cbmimo1_lte 0 14 1 #RF mode 1 = mixer low gain, lna on
#./openair_rf_cbmimo1_lte 0 14 2 #RF mode 2 = mixer high gain, lna on
./openair_rf_cbmimo1_lte 0 3 1 8
cd ../..
