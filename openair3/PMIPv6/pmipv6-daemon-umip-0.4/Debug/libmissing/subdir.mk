################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
O_SRCS += \
../libmissing/inet6_rth_add.o \
../libmissing/inet6_rth_getaddr.o \
../libmissing/inet6_rth_gettype.o \
../libmissing/inet6_rth_init.o \
../libmissing/inet6_rth_space.o 

C_SRCS += \
../libmissing/inet6_opt_find.c \
../libmissing/inet6_rth_add.c \
../libmissing/inet6_rth_getaddr.c \
../libmissing/inet6_rth_gettype.c \
../libmissing/inet6_rth_init.c \
../libmissing/inet6_rth_space.c 

OBJS += \
./libmissing/inet6_opt_find.o \
./libmissing/inet6_rth_add.o \
./libmissing/inet6_rth_getaddr.o \
./libmissing/inet6_rth_gettype.o \
./libmissing/inet6_rth_init.o \
./libmissing/inet6_rth_space.o 

C_DEPS += \
./libmissing/inet6_opt_find.d \
./libmissing/inet6_rth_add.d \
./libmissing/inet6_rth_getaddr.d \
./libmissing/inet6_rth_gettype.d \
./libmissing/inet6_rth_init.d \
./libmissing/inet6_rth_space.d 


# Each subdirectory must supply rules for building sources it contributes
libmissing/%.o: ../libmissing/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o"$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


