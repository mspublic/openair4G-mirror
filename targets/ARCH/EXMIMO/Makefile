CCC = gcc
KERNEL_MAIN_TYPE=$(shell echo `uname -r | cut -d. -f-2  | tr "." "_"`)
export KERNEL_MAIN_TYPE

SUBVERSION=$(shell echo `grep '^SUBLEVEL =' /usr/src/linux/Makefile | sed -e 's, ,,g' | sed -e 's/SUBLEVEL=//'`)
IS_KERNEL_SUBVERSION_GREATER_THAN_20=$(shell if [ $(SUBVERSION) -ge 20 ] ; then echo true ; fi)

EXTRA_CFLAGS = -ggdb -D__KERNEL__ -DMODULE -D_LOOSE_KERNEL_NAMES -I/lib/modules/$(shell uname -r)/build/include -I/lib/modules/$(shell uname -r)/build/include/asm/mach-default -include /lib/modules/$(shell uname -r)/build/include/linux/autoconf.h 

ifndef OPENAIR0_DIR
  $(error Environment variable OPENAIR0_DIR must be set!)
else
  EXTRA_CFLAGS += -I$(OPENAIR0_DIR)/express-mimo/software/pcie
endif


obj-m += openair_rf.o

openair_rf-objs += module_main.o irq.o fileops.o exmimo_fw.o

all:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules

clean:
	rm -rf *.o
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean

