################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../src/Fcalculate.c \
../src/dataacq.c \
../src/flow_calculation.c \
../src/gpio.c \
../src/i2cwork.c \
../src/readpara.c \
../src/spi.c \
../src/spmath.c \
../src/yangpai.c 

OBJS += \
./src/Fcalculate.o \
./src/dataacq.o \
./src/flow_calculation.o \
./src/gpio.o \
./src/i2cwork.o \
./src/readpara.o \
./src/spi.o \
./src/spmath.o \
./src/yangpai.o 

C_DEPS += \
./src/Fcalculate.d \
./src/dataacq.d \
./src/flow_calculation.d \
./src/gpio.d \
./src/i2cwork.d \
./src/readpara.d \
./src/spi.d \
./src/spmath.d \
./src/yangpai.d 


# Each subdirectory must supply rules for building sources it contributes
src/%.o: ../src/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: Cross GCC Compiler'
	arm-fsl-linux-gnueabi-gcc -I/usr/local/mysql/include/mysql -I/opt/gcc-4.4.4-glibc-2.11.1-multilib-1.0/arm-fsl-linux-gnueabi -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


