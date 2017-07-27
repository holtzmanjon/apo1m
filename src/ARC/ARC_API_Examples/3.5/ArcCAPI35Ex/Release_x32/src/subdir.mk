################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../src/ArcCAPI35Ex.c 

OBJS += \
./src/ArcCAPI35Ex.o 

C_DEPS += \
./src/ArcCAPI35Ex.d 


# Each subdirectory must supply rules for building sources it contributes
src/%.o: ../src/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -I/home/streit/Documents/ARC_API/3.5/CArcDevice/src -I/home/streit/Documents/ARC_API/3.5/cfitsio-3330/linux/x32/include -I/home/streit/Documents/ARC_API/3.5/CArcDeinterlace/src -I/home/streit/Documents/ARC_API/3.5/CArcFitsFile/src -O3 -Wall -c -fmessage-length=0 -m32 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o"$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


