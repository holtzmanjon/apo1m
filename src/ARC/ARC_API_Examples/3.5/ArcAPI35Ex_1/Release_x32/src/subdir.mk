################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../src/ArcAPI35Ex_1.cpp 

OBJS += \
./src/ArcAPI35Ex_1.o 

CPP_DEPS += \
./src/ArcAPI35Ex_1.d 


# Each subdirectory must supply rules for building sources it contributes
src/%.o: ../src/%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C++ Compiler'
	g++ -I/home/streit/Documents/ARC_API/3.5/CArcDeinterlace/src -I/home/streit/Documents/ARC_API/3.5/CArcDevice/src -I/home/streit/Documents/ARC_API/3.5/CArcFitsFile/src -I/home/streit/Documents/ARC_API/3.5/cfitsio-3330/linux/x32/include -O3 -Wall -c -fmessage-length=0 -std=c++0x -m32 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o"$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


