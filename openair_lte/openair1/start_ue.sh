cd USERSPACE_TOOLS/OPENAIR_RF
./openair_rf_cbmimo1_lte 0 4 1
rmmod -f openair_rf
cd ../..
make install_cbmimo1_softmodem_lte_emos_l2
cd USERSPACE_TOOLS/OPENAIR_RF
./openair_rf_cbmimo1_lte 0 0 0 1
./openair_rf_cbmimo1_lte 0 6 250 250 140 140
./openair_rf_cbmimo1_lte 0 25 80 
./openair_rf_cbmimo1_lte 0 15 53  # Card v2_3 synched to sector 1
#./openair_rf_cbmimo1_lte 0 15 208  # Card v2_3 synched to v2_13
#./openair_rf_cbmimo1_lte 0 15 217  # Card v2_7 synched to v2_13
#./openair_rf_cbmimo1_lte 0 15 143  # Card v2_7 synched to v2_8
./openair_rf_cbmimo1_lte 0 26 0
./openair_rf_cbmimo1_lte 0 14 1 #RF mode 1 = mixer low gain, lna on
#./openair_rf_cbmimo1_lte 0 14 2 #RF mode 2 = mixer high gain, lna on
#./openair_rf_cbmimo1_lte 0 14 0 #RF mode 2 = mixer low gain, lna off
./openair_rf_cbmimo1_lte 0 3 1 8
cd ../..
