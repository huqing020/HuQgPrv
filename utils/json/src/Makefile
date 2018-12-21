################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 

ALL_OBJS += \
$(UTILDIR)/utils/json/src/json_reader.o	\
$(UTILDIR)/utils/json/src/json_value.o	\
$(UTILDIR)/utils/json/src/json_writer.o	


CPP_DEPS += \
$(UTILDIR)/utils/json/src/json_reader.d	\
$(UTILDIR)/utils/json/src/json_value.d	\
$(UTILDIR)/utils/json/src/json_writer.d	

INCLUDE_JSON = -I"$(UTILDIR)/utils/"

# Each subdirectory must supply rules for building sources it contributes
$(UTILDIR)/utils/json/src/%.o : $(UTILDIR)/utils/json/src/%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C++ Compiler'
	g++ $(FPICFLAG) -O0 -g3 -Wall $(INCLUDE_JSON) -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o"$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '
