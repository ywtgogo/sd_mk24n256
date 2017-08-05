################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../../../bootloaderk24/sources/adc.c \
../../../bootloaderk24/sources/main.c \
../../../bootloaderk24/sources/spiflash_ll.c 

OBJS += \
./source/adc.o \
./source/main.o \
./source/spiflash_ll.o 

C_DEPS += \
./source/adc.d \
./source/main.d \
./source/spiflash_ll.d 


# Each subdirectory must supply rules for building sources it contributes
source/adc.o: ../../../bootloaderk24/sources/adc.c
	@echo 'Building file: $<'
	@echo 'Invoking: Cross ARM C Compiler'
	arm-none-eabi-gcc -mcpu=cortex-m4 -mthumb -mfloat-abi=hard -mfpu=fpv4-sp-d16 -Os -fmessage-length=0 -fsigned-char -ffunction-sections -fdata-sections -fno-common -ffreestanding -fno-builtin -Wall -DNDEBUG -DBOOTLOADER -DCPU_MK24FN256VDC12 -DTWR_K24F120M -DTOWER -I../.. -I../../include -I../../../CMSIS/Include -I../../../homeboxk24 -I../../../crypto -I../../../devices -I../../../devices/MK24F25612/drivers -I../../../devices/MK24F25612/utilities -I../../../devices/MK24F25612 -std=gnu99 -mapcs -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" -c -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

source/main.o: ../../../bootloaderk24/sources/main.c
	@echo 'Building file: $<'
	@echo 'Invoking: Cross ARM C Compiler'
	arm-none-eabi-gcc -mcpu=cortex-m4 -mthumb -mfloat-abi=hard -mfpu=fpv4-sp-d16 -Os -fmessage-length=0 -fsigned-char -ffunction-sections -fdata-sections -fno-common -ffreestanding -fno-builtin -Wall -DNDEBUG -DBOOTLOADER -DCPU_MK24FN256VDC12 -DTWR_K24F120M -DTOWER -I../.. -I../../include -I../../../CMSIS/Include -I../../../homeboxk24 -I../../../crypto -I../../../devices -I../../../devices/MK24F25612/drivers -I../../../devices/MK24F25612/utilities -I../../../devices/MK24F25612 -std=gnu99 -mapcs -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" -c -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

source/spiflash_ll.o: ../../../bootloaderk24/sources/spiflash_ll.c
	@echo 'Building file: $<'
	@echo 'Invoking: Cross ARM C Compiler'
	arm-none-eabi-gcc -mcpu=cortex-m4 -mthumb -mfloat-abi=hard -mfpu=fpv4-sp-d16 -Os -fmessage-length=0 -fsigned-char -ffunction-sections -fdata-sections -fno-common -ffreestanding -fno-builtin -Wall -DNDEBUG -DBOOTLOADER -DCPU_MK24FN256VDC12 -DTWR_K24F120M -DTOWER -I../.. -I../../include -I../../../CMSIS/Include -I../../../homeboxk24 -I../../../crypto -I../../../devices -I../../../devices/MK24F25612/drivers -I../../../devices/MK24F25612/utilities -I../../../devices/MK24F25612 -std=gnu99 -mapcs -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" -c -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


