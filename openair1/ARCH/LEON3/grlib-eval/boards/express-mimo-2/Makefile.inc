# Below are the Express-MIMO-2 features (common to any release, available or to come)
TECHNOLOGY=virtex5
PACKAGE=ff1136
PROMGENPAR=-x xc18v04 xc18v04 xc18v04 -u 0 $(TOP).bit -p mcs -w -o gr-pci-xc2v
MANUFACTURER=Xilinx
MGCTECHNOLOGY=VIRTEX-V
MGCPACKAGE=$(PACKAGE)
# Depending on the release (0, 1 or 2) specified in $PT2/Makefile, include proper board-related information
include $(GRLIB)/boards/$(BOARD)/v$(BOARD_VERSION)/Makefile.inc
