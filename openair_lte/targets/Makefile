#ifdef cygwin
#  cyg=1
#else
#  cyg=0
#endif

help:
	@echo make check1 performs a simple sanity check
	@echo make check2 performs another simple sanity check
	@echo make check does both the above checks
 
all: help

check1:
	@echo Doing 1 eNB 1 UE test
	(cd SIMU/EXAMPLES/VIRT_EMUL_1eNB && make one_eNB_one_UE )  #cygwin=$(cyg))

check2:
	@echo Doing 1 eNB 4 UE test
	(cd SIMU/EXAMPLES/VIRT_EMUL_1ENB && make one_eNB_four_UE )  #cygwin=$(cyg))


check: check1 check2

