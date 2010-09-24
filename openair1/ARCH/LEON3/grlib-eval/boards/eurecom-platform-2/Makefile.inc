# Below are the Cardbus-MIMO-1 features shared by the 3 releases (Version 0, 1 and 2)
TECHNOLOGY=virtex2
PACKAGE=fg676
PROMGENPAR=-x xc18v04 xc18v04 xc18v04 -u 0 $(TOP).bit -p mcs -w -o gr-pci-xc2v
MANUFACTURER=Xilinx
MGCTECHNOLOGY=VIRTEX-II
MGCPACKAGE=$(PACKAGE)
# Depending on the release (0, 1 or 2) specified in $PT2/Makefile, include proper board-related information
include $(GRLIB)/boards/$(BOARD)/v$(BOARD_VERSION)/Makefile.inc
