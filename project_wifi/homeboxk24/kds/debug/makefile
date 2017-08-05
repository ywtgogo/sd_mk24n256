################################################################################
# Automatically-generated file. Do not edit!
################################################################################

-include ../makefile.init

RM := rm -rf

# All of the sources participating in the build are defined here
-include sources.mk
-include utilities/subdir.mk
-include uplink/subdir.mk
-include startup/subdir.mk
-include source/subdir.mk
-include rtos/freertos_8.2.3_k24/Source/portable/GCC/ARM_CM4F/subdir.mk
-include rtos/freertos_8.2.3_k24/Source/subdir.mk
-include mqtt/MQTTpacket/subdir.mk
-include mqtt/subdir.mk
-include modem_plc/drv_st7580/subdir.mk
-include modem_plc/subdir.mk
-include modem_gprs/subdir.mk
-include modem_flash/subdir.mk
-include lwip_1.4.1/src/netif/subdir.mk
-include lwip_1.4.1/port/subdir.mk
-include json/subdir.mk
-include http_client/subdir.mk
-include drivers/subdir.mk
-include crypto/subdir.mk
-include console/subdir.mk
-include board/subdir.mk
-include 7139/subdir.mk
-include subdir.mk
-include objects.mk

ifneq ($(MAKECMDGOALS),clean)
ifneq ($(strip $(C++_DEPS)),)
-include $(C++_DEPS)
endif
ifneq ($(strip $(C_DEPS)),)
-include $(C_DEPS)
endif
ifneq ($(strip $(ASM_DEPS)),)
-include $(ASM_DEPS)
endif
ifneq ($(strip $(CC_DEPS)),)
-include $(CC_DEPS)
endif
ifneq ($(strip $(CPP_DEPS)),)
-include $(CPP_DEPS)
endif
ifneq ($(strip $(CXX_DEPS)),)
-include $(CXX_DEPS)
endif
ifneq ($(strip $(C_UPPER_DEPS)),)
-include $(C_UPPER_DEPS)
endif
ifneq ($(strip $(S_UPPER_DEPS)),)
-include $(S_UPPER_DEPS)
endif
endif

-include ../makefile.defs

# Add inputs and outputs from these tool invocations to the build variables 
SECONDARY_FLASH += \
project_k24.srec \

SECONDARY_SIZE += \
project_k24.siz \


# All Target
all: project_k24.elf secondary-outputs

# Tool invocations
project_k24.elf: $(OBJS) $(USER_OBJS)
	@echo 'Building target: $@'
	@echo 'Invoking: Cross ARM C++ Linker'
	arm-none-eabi-g++ -mcpu=cortex-m4 -mthumb -mfloat-abi=hard -mfpu=fpv4-sp-d16 -O0 -fmessage-length=0 -fsigned-char -ffunction-sections -fdata-sections -fno-common -ffreestanding -fno-builtin -Wall -Wredundant-decls  -g -T ../settings/MK24FN256xxx12_flash.ld -Xlinker --gc-sections -Wl,-Map,"project_k24.map" --specs=nano.specs -mapcs  -Xlinker -static  -Xlinker -z  -Xlinker muldefs -o "project_k24.elf" -Xlinker --start-group $(OBJS) $(USER_OBJS) $(LIBS) -Xlinker --end-group
	@echo 'Finished building target: $@'
	@echo ' '

project_k24.srec: project_k24.elf
	@echo 'Invoking: Cross ARM GNU Create Flash Image'
	arm-none-eabi-objcopy -O srec "project_k24.elf"  "project_k24.srec"
	@echo 'Finished building: $@'
	@echo ' '

project_k24.siz: project_k24.elf
	@echo 'Invoking: Cross ARM GNU Print Size'
	arm-none-eabi-size --format=berkeley "project_k24.elf"
	@echo 'Finished building: $@'
	@echo ' '

# Other Targets
clean:
	-$(RM) $(SECONDARY_SIZE)$(C++_DEPS)$(OBJS)$(C_DEPS)$(ASM_DEPS)$(CC_DEPS)$(SECONDARY_FLASH)$(CPP_DEPS)$(CXX_DEPS)$(C_UPPER_DEPS)$(S_UPPER_DEPS) project_k24.elf
	-@echo ' '

secondary-outputs: $(SECONDARY_FLASH) $(SECONDARY_SIZE)

.PHONY: all clean dependents
.SECONDARY:

-include ../makefile.targets
