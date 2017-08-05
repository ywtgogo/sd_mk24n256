################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../../../../devices/MK24F25612/system_MK24F25612.c 

S_UPPER_SRCS += \
../../../../devices/MK24F25612/gcc/startup_MK24F25612.S 

OBJS += \
./startup/startup_MK24F25612.o \
./startup/system_MK24F25612.o 

C_DEPS += \
./startup/system_MK24F25612.d 

S_UPPER_DEPS += \
./startup/startup_MK24F25612.d 


# Each subdirectory must supply rules for building sources it contributes
startup/startup_MK24F25612.o: ../../../../devices/MK24F25612/gcc/startup_MK24F25612.S
	@echo 'Building file: $<'
	@echo 'Invoking: Cross ARM GNU Assembler'
	arm-none-eabi-gcc -mcpu=cortex-m4 -mthumb -mfloat-abi=hard -mfpu=fpv4-sp-d16 -Os -fmessage-length=0 -fsigned-char -ffunction-sections -fdata-sections -fno-common -ffreestanding -fno-builtin -Wall -x assembler-with-cpp -D__STARTUP_CLEAR_BSS -mapcs -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" -c -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

startup/system_MK24F25612.o: ../../../../devices/MK24F25612/system_MK24F25612.c
	@echo 'Building file: $<'
	@echo 'Invoking: Cross ARM C Compiler'
	arm-none-eabi-gcc -mcpu=cortex-m4 -mthumb -mfloat-abi=hard -mfpu=fpv4-sp-d16 -Os -fmessage-length=0 -fsigned-char -ffunction-sections -fdata-sections -fno-common -ffreestanding -fno-builtin -Wall -DNDEBUG -DBOOTLOADER -DCPU_MK24FN256VDC12 -DTWR_K24F120M -DTOWER -I../.. -I../../include -I../../../../protocol -I../../../CMSIS/Include -I../../../homeboxk24 -I../../../../crypto -I../../../../devices -I../../../../devices/MK24F25612/drivers -I../../../../devices/MK24F25612/utilities -I../../../../devices/MK24F25612 -std=gnu99 -mapcs $(CFLAGS) -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" -c -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


