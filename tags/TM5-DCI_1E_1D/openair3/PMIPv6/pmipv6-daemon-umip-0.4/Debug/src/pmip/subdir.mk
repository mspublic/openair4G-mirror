################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../src/pmip/pmip_cache.c \
../src/pmip/pmip_fsm.c \
../src/pmip/pmip_handler.c \
../src/pmip/pmip_hnp_cache.c \
../src/pmip/pmip_init.c \
../src/pmip/pmip_lma_proc.c \
../src/pmip/pmip_mag_proc.c \
../src/pmip/pmip_msgs.c \
../src/pmip/pmip_pcap.c \
../src/pmip/pmip_tunnel.c 

OBJS += \
./src/pmip/pmip_cache.o \
./src/pmip/pmip_fsm.o \
./src/pmip/pmip_handler.o \
./src/pmip/pmip_hnp_cache.o \
./src/pmip/pmip_init.o \
./src/pmip/pmip_lma_proc.o \
./src/pmip/pmip_mag_proc.o \
./src/pmip/pmip_msgs.o \
./src/pmip/pmip_pcap.o \
./src/pmip/pmip_tunnel.o 

C_DEPS += \
./src/pmip/pmip_cache.d \
./src/pmip/pmip_fsm.d \
./src/pmip/pmip_handler.d \
./src/pmip/pmip_hnp_cache.d \
./src/pmip/pmip_init.d \
./src/pmip/pmip_lma_proc.d \
./src/pmip/pmip_mag_proc.d \
./src/pmip/pmip_msgs.d \
./src/pmip/pmip_pcap.d \
./src/pmip/pmip_tunnel.d 


# Each subdirectory must supply rules for building sources it contributes
src/pmip/%.o: ../src/pmip/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o"$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


