################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
O_SRCS += \
../src/bcache.o \
../src/bul.o \
../src/cn.o \
../src/conf.o \
../src/debug.o \
../src/dhaad_ha.o \
../src/dhaad_mn.o \
../src/gram.o \
../src/ha.o \
../src/hash.o \
../src/icmp6.o \
../src/ipsec.o \
../src/keygen.o \
../src/main.o \
../src/mh.o \
../src/mn.o \
../src/movement.o \
../src/mpdisc_ha.o \
../src/mpdisc_mn.o \
../src/ndisc.o \
../src/pmgr.o \
../src/pmip_cache.o \
../src/pmip_fsm.o \
../src/pmip_handler.o \
../src/pmip_hnp_cache.o \
../src/pmip_init.o \
../src/pmip_lma_proc.o \
../src/pmip_mag_proc.o \
../src/pmip_msgs.o \
../src/pmip_pcap.o \
../src/pmip_tunnel.o \
../src/policy.o \
../src/prefix.o \
../src/proc_sys.o \
../src/retrout.o \
../src/rtnl.o \
../src/scan.o \
../src/tqueue.o \
../src/tunnelctl.o \
../src/vars.o \
../src/xfrm.o 

C_SRCS += \
../src/bcache.c \
../src/bul.c \
../src/cn.c \
../src/conf.c \
../src/crypto.c \
../src/debug.c \
../src/dhaad_ha.c \
../src/dhaad_mn.c \
../src/gram.c \
../src/ha.c \
../src/hash.c \
../src/icmp6.c \
../src/ipsec.c \
../src/keygen.c \
../src/main.c \
../src/mh.c \
../src/mn.c \
../src/movement.c \
../src/mpdisc_ha.c \
../src/mpdisc_mn.c \
../src/ndisc.c \
../src/pmgr.c \
../src/policy.c \
../src/prefix.c \
../src/proc_sys.c \
../src/retrout.c \
../src/rtnl.c \
../src/scan.c \
../src/tqueue.c \
../src/tunnelctl.c \
../src/vars.c \
../src/vt.c \
../src/xfrm.c 

OBJS += \
./src/bcache.o \
./src/bul.o \
./src/cn.o \
./src/conf.o \
./src/crypto.o \
./src/debug.o \
./src/dhaad_ha.o \
./src/dhaad_mn.o \
./src/gram.o \
./src/ha.o \
./src/hash.o \
./src/icmp6.o \
./src/ipsec.o \
./src/keygen.o \
./src/main.o \
./src/mh.o \
./src/mn.o \
./src/movement.o \
./src/mpdisc_ha.o \
./src/mpdisc_mn.o \
./src/ndisc.o \
./src/pmgr.o \
./src/policy.o \
./src/prefix.o \
./src/proc_sys.o \
./src/retrout.o \
./src/rtnl.o \
./src/scan.o \
./src/tqueue.o \
./src/tunnelctl.o \
./src/vars.o \
./src/vt.o \
./src/xfrm.o 

C_DEPS += \
./src/bcache.d \
./src/bul.d \
./src/cn.d \
./src/conf.d \
./src/crypto.d \
./src/debug.d \
./src/dhaad_ha.d \
./src/dhaad_mn.d \
./src/gram.d \
./src/ha.d \
./src/hash.d \
./src/icmp6.d \
./src/ipsec.d \
./src/keygen.d \
./src/main.d \
./src/mh.d \
./src/mn.d \
./src/movement.d \
./src/mpdisc_ha.d \
./src/mpdisc_mn.d \
./src/ndisc.d \
./src/pmgr.d \
./src/policy.d \
./src/prefix.d \
./src/proc_sys.d \
./src/retrout.d \
./src/rtnl.d \
./src/scan.d \
./src/tqueue.d \
./src/tunnelctl.d \
./src/vars.d \
./src/vt.d \
./src/xfrm.d 


# Each subdirectory must supply rules for building sources it contributes
src/%.o: ../src/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o"$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


