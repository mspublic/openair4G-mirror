lte_dfts: lte_dfts.c
	gcc -O2 -msse4.1 -o lte_dfts lte_dfts.c fft.c ../../SIMULATION/TOOLS/taus.c -I$$OPENAIR1_DIR -I$$OPENAIR_TARGETS -DUSER_MODE -DMR_MAIN -DNB_ANTENNAS_RX=1 #-DD64STATS

dft_cycles: lte_dfts
	./lte_dfts | egrep cycles
