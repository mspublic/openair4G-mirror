
# These 2 files are postprocessing of softconfig.h & softregs.h (resp.), which are 2 files
# coming from the grlib-eval hard & soft design.
GRLIBPATH=../../../../grlib-eval
DESIGNPATH=$(GRLIBPATH)/designs/leon3-eurecom-platform-2
SOFTCONFIG_H=$(DESIGNPATH)/devhardsoft/softconfig.h
SOFTREGS_H=$(DESIGNPATH)/devsoft/src/softregs.h
GRPCI_SOFTCONFIG_H=grpci_softconfig.h
GRPCI_SOFTREGS_H=grpci_softregs.h

# If KERNELRELEASE is defined, we've been invoked from
# the kernel build system and can use its language.
ifneq ($(KERNELRELEASE),)
  obj-m := grpci.o

#$(obj-m): $(GRPCI_SOFTCONFIG_H) $(GRPCI_SOFTREGS_H)

# Otherwise we were called directly from the command
# line; invoke the kernel build system.
else

  KERNELDIR ?= /lib/modules/$(shell uname -r)/build
  PWD := $(shell pwd)

default: $(GRPCI_SOFTREGS_H) $(GRPCI_SOFTCONFIG_H)
	@#echo "Considering $(GRPCI_SOFTCONFIG_H) and $(GRPCI_SOFTREGS_H)..."
	@#$(MAKE) -s -C . $(GRPCI_SOFTCONFIG_H) $(GRPCI_SOFTREGS_H)
	@#echo "...done"
	@$(MAKE) -C $(KERNELDIR) M=$(PWD) modules 2>&1 \
	  | sed -e 's/\/homes\/khalfall\/work\/openairinterface\/arch\/openair_CardBus_MIMO1\/LEON3\/hostpc\/kernel\/modules\/grpci/$$GRPCIK/g'

endif

EXTRA_CFLAGS += -Wall -Wstrict-prototypes -fno-common -pipe -fno-strict-aliasing -fno-strength-reduce -mmmx -msse -msse2 -falign-loops=2 -falign-jumps=2 -falign-functions=2 -DCPU=686 -DMODULE -D_LOOSE_KERNEL_NAMES -O2

ifeq ($(debug),yes)
EXTRA_CFLAGS += -DGRPCI_DEBUG
endif

ifdef bigphyspages
EXTRA_CFLAGS+=-DGRPCI_BIGPHYS_PAGES=$(bigphyspages)
endif

$(GRPCI_SOFTCONFIG_H): $(SOFTCONFIG_H)
	@echo "/* Automatically generated based on $(notdir $<) from Grlib . Don't edit. */" >| $@
	@echo "/* "`date`" */" >> $@
	@awk 'NR>2 {print $0}' $< >> $@
	@sed -i -e 's/#ifndef \([^ ]\{1,\}\)/#ifndef FROM_GRLIB_\1/' $@
	@sed -i -e 's/#define \([^ ]\{1,\}\)/#define FROM_GRLIB_\1/' $@
	@sed -i -e 's/#endif \/\* \([^ ]\{1,\}\)/#endif \/\* FROM_GRLIB_\1/' $@
	@echo "Generated $@"

$(GRPCI_SOFTREGS_H): $(SOFTREGS_H)
	@echo "/* Automatically generated based on $(notdir $<) from Grlib . Don't edit. */" >| $@
	@echo "/* "`date`" */" >> $@
	@cat $< >> $@
	@sed -i -e 's/#ifndef \([^ ]\{1,\}\)/#ifndef FROM_GRLIB_\1/' $@
	@sed -i -e 's/#define \([^ ]\{1,\}\)/#define FROM_GRLIB_\1/' $@
	@sed -i -e 's/#endif \/\* \([^ ]\{1,\}\)/#endif \/\* FROM_GRLIB_\1/' $@
	@echo "Generated $@"

clean:
	rm -Rf grpci.ko .grpci.ko.cmd grpci.mod.c grpci.mod.o .grpci.mod.o.cmd grpci.o .grpci.o.cmd
	rm -Rf .tmp_versions/
	rm -f $(GRPCI_SOFTCONFIG_H) $(GRPCI_SOFTREGS_H)
