TOOLS_OBJS += $(OPENAIR1_DIR)/PHY/CODING/ccoding_byte.o
TOOLS_OBJS += $(OPENAIR1_DIR)/PHY/CODING/crc_byte.o
TOOLS_OBJS += $(OPENAIR1_DIR)/PHY/CODING/viterbi.o
TOOLS_OBJS += $(OPENAIR1_DIR)/PHY/TOOLS/fft.o
TOOLS_OBJS += $(OPENAIR1_DIR)/PHY/TOOLS/log2_approx.o
TOOLS_OBJS += $(OPENAIR1_DIR)/PHY/TOOLS/cmult_sv.o
TOOLS_OBJS += $(OPENAIR1_DIR)/PHY/TOOLS/cmult_vv.o
TOOLS_OBJS += $(OPENAIR1_DIR)/PHY/TOOLS/cadd_vv.o
TOOLS_OBJS += $(OPENAIR1_DIR)/PHY/TOOLS/signal_energy.o
TOOLS_OBJS += $(OPENAIR1_DIR)/PHY/TOOLS/dB_routines.o
PHY_OBJS += $(OPENAIRITS_DIR)/phy/DOT11/phy_tx_start.o
PHY_OBJS += $(OPENAIRITS_DIR)/phy/DOT11/scrambler.o
PHY_OBJS += $(OPENAIRITS_DIR)/phy/DOT11/interleaver.o
PHY_OBJS += $(OPENAIRITS_DIR)/phy/DOT11/crc32.o
PHY_OBJS += $(OPENAIRITS_DIR)/phy/DOT11/initial_sync.o
PHY_OBJS += $(OPENAIRITS_DIR)/phy/DOT11/data_detection.o