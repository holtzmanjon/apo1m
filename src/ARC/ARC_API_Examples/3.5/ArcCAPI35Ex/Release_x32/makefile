################################################################################
# Automatically-generated file. Do not edit!
################################################################################

-include ../makefile.init

RM := rm -rf

# All of the sources participating in the build are defined here
-include sources.mk
-include subdir.mk
-include src/Release/subdir.mk
-include src/subdir.mk
-include objects.mk

ifneq ($(MAKECMDGOALS),clean)
ifneq ($(strip $(C_DEPS)),)
-include $(C_DEPS)
endif
endif

-include ../makefile.defs

# Add inputs and outputs from these tool invocations to the build variables 

# All Target
all: ArcCAPI35Ex

# Tool invocations
ArcCAPI35Ex: $(OBJS) $(USER_OBJS)
	@echo 'Building target: $@'
	@echo 'Invoking: GCC C Linker'
	gcc -L/home/streit/Documents/ARC_API/3.5/Release/x32 -L/home/streit/Documents/ARC_API/3.5/cfitsio-3330/linux/x32/lib -m32 -o"ArcCAPI35Ex" $(OBJS) $(USER_OBJS) $(LIBS)
	@echo 'Finished building target: $@'
	@echo ' '

# Other Targets
clean:
	-$(RM) $(OBJS)$(C_DEPS)$(EXECUTABLES) ArcCAPI35Ex
	-@echo ' '

.PHONY: all clean dependents
.SECONDARY:

-include ../makefile.targets
