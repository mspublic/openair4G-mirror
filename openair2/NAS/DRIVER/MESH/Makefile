# NASMESH Driver makefile
#
NAS_UPDIR	:= $(shell /bin/pwd)


KERNEL_MAIN_TYPE=$(shell echo `uname -r | cut -d. -f-2  | tr "." "_"`)
export KERNEL_MAIN_TYPE
SUBVERSION=$(shell echo `grep '^SUBLEVEL =' /usr/src/linux/Makefile | sed -e 's, ,,g' | sed -e 's/SUBLEVEL=//'`)
IS_KERNEL_SUBVERSION_GREATER_THAN_20=$(shell if [ $(SUBVERSION) -ge 20 ] ; then echo true ; fi)
KERNEL_ARCH=$(shell echo `uname -m`)
#SET_REGPARM=$(shell if [ $(KERNEL_ARCH) = 'i686' -a $(SUBVERSION) -ge 20 ]; then echo true ; fi)
SET_X64=$(shell if [ $(KERNEL_ARCH) = 'x86_64' -a $(SUBVERSION) -ge 20 ]; then echo true ; fi)

IS_KERNEL_SUBVERSION_GREATER_THAN_22=$(shell if [ $(SUBVERSION) -ge 22 ] ; then echo true ; fi)
IS_KERNEL_SUBVERSION_GREATER_THAN_29=$(shell if [ $(SUBVERSION) -ge 29 ] ; then echo true ; fi)
IS_KERNEL_SUBVERSION_GREATER_THAN_30=$(shell if [ $(SUBVERSION) -ge 30 ] ; then echo true ; fi)
GT2622= $(if $(IS_KERNEL_SUBVERSION_GREATER_THAN_22),-DKERNEL_VERSION_GREATER_THAN_2622=1)
GT2629= $(if $(IS_KERNEL_SUBVERSION_GREATER_THAN_29),-DKERNEL_VERSION_GREATER_THAN_2629=1)
GT2630= $(if $(IS_KERNEL_SUBVERSION_GREATER_THAN_30),-DKERNEL_VERSION_GREATER_THAN_2630=1)

####################################################
#      NASMESH compilation flags
####################################################
#RTAI=1

####################################################
#  D E B U G   F L A G S
####################################################



####################################################
#      EXTRA COMPILER FLAGS
####################################################
EXTRA_CFLAGS = -fno-common $(if $(IS_KERNEL_SUBVERSION_GREATER_THAN_20),-mregparm=3  -fno-stack-protector -mpreferred-stack-boundary=4)  $(if $(SET_X64),-DARCH_64,) $(if $(SET_X64),-mcmodel=kernel,) $(if $(SET_X64),-m64,) $(GT2622) $(GT2629) $(GT2630) 

ifdef NAS_NETLINK
EXTRA_CFLAGS += -DNAS_NETLINK
else
EXTRA_CFLAGS += $(shell rtai-config --module-cflags) -DRTAI -D__IN_RTAI__ -Wall
endif

ifdef LOOPBACK
EXTRA_CFLAGS += -DLOOPBACK_TEST
endif

ifdef ADDRESS_FIX
EXTRA_CFLAGS += -DNAS_ADDRESS_FIX
endif

####################################################
#      LOADABLE MODULE GOALS
####################################################
obj-m += nasmesh.o
nasmesh-objs += device.o
nasmesh-objs += common.o
nasmesh-objs += ioctl.o
nasmesh-objs += classifier.o
nasmesh-objs += tool.o
nasmesh-objs += mesh.o
ifdef NAS_NETLINK
nasmesh-objs += netlink.o
endif

####################################################
#      REVOIR LE CLEAN
####################################################

#netlink.ko: 
	#make $(x)$(y) NAS_NETLINK=1 V=1 -C /usr/src/linux-$(shell echo `uname -r`) M=`pwd` modules

#nasmesh.ko: 
#	make  V=1 -C /usr/src/linux-$(shell echo `uname -r`) M=`pwd` modules
print:
	@echo $(IS_KERNEL_SUBVERSION_GREATER_THAN_22)
	@echo $(GT2622)
	@echo $(IS_KERNEL_SUBVERSION_GREATER_THAN_29)
	@echo $(GT2629)
	@echo $(IS_KERNEL_SUBVERSION_GREATER_THAN_30)
	@echo $(GT2630)
#clean:
#	make -C /usr/src/linux-$(shell echo `uname -r`) M=`pwd` modules