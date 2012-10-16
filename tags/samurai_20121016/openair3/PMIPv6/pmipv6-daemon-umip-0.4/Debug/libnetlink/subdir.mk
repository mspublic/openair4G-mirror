################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
O_SRCS += \
../libnetlink/libnetlink.o 

C_SRCS += \
../libnetlink/libnetlink.c 

OBJS += \
./libnetlink/libnetlink.o 

C_DEPS += \
./libnetlink/libnetlink.d 


# Each subdirectory must supply rules for building sources it contributes
libnetlink/%.o: ../libnetlink/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o"$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


