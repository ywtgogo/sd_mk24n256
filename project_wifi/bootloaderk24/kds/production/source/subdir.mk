################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../../../../project_wifi/bootloaderk24/sources/adc.c \
../../../../project_wifi/bootloaderk24/sources/main.c \
../../../../project_wifi/bootloaderk24/sources/spiflash_ll.c \
../../../../project_wifi/bootloaderk24/sources/version.c 

OBJS += \
./source/adc.o \
./source/main.o \
./source/spiflash_ll.o \
./source/version.o 

C_DEPS += \
./source/adc.d \
./source/main.d \
./source/spiflash_ll.d \
./source/version.d 


# Each subdirectory must supply rules for building sources it contributes
source/adc.o: ../../../../project_wifi/bootloaderk24/sources/adc.c
	@echo 'Building file: $<'
	@echo 'Invoking: Cross ARM C Compiler'
	arm-none-eabi-gcc -mcpu=cortex-m4 -mthumb -mfloat-abi=hard -mfpu=fpv4-sp-d16 -Os -fmessage-length=0 -fsigned-char -ffunction-sections -fdata-sections -fno-common -ffreestanding -fno-builtin -Wall -DNDEBUG -DBOOTLOADER -DCPU_MK24FN256VDC12 -DTWR_K24F120M -DTOWER -I../.. -I../../include -I../../../CMSIS/Include -I../../../homeboxk24 -I../../../../crypto -I../../../../protocol -I../../../../devices -I../../../../devices/MK24F25612/drivers -I../../../../devices/MK24F25612/utilities -I../../../../devices/MK24F25612 -std=gnu99 -mapcs $(CFLAGS) -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" -c -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

source/main.o: ../../../../project_wifi/bootloaderk24/sources/main.c
	@echo 'Building file: $<'
	@echo 'Invoking: Cross ARM C Compiler'
	arm-none-eabi-gcc -mcpu=cortex-m4 -mthumb -mfloat-abi=hard -mfpu=fpv4-sp-d16 -Os -fmessage-length=0 -fsigned-char -ffunction-sections -fdata-sections -fno-common -ffreestanding -fno-builtin -Wall -DNDEBUG -DBOOTLOADER -DCPU_MK24FN256VDC12 -DTWR_K24F120M -DTOWER -I../.. -I../../include -I../../../CMSIS/Include -I../../../homeboxk24 -I../../../../crypto -I../../../../protocol -I../../../../devices -I../../../../devices/MK24F25612/drivers -I../../../../devices/MK24F25612/utilities -I../../../../devices/MK24F25612 -std=gnu99 -mapcs $(CFLAGS) -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" -c -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

source/spiflash_ll.o: ../../../../project_wifi/bootloaderk24/sources/spiflash_ll.c
	@echo 'Building file: $<'
	@echo 'Invoking: Cross ARM C Compiler'
	arm-none-eabi-gcc -mcpu=cortex-m4 -mthumb -mfloat-abi=hard -mfpu=fpv4-sp-d16 -Os -fmessage-length=0 -fsigned-char -ffunction-sections -fdata-sections -fno-common -ffreestanding -fno-builtin -Wall -DNDEBUG -DBOOTLOADER -DCPU_MK24FN256VDC12 -DTWR_K24F120M -DTOWER -I../.. -I../../include -I../../../CMSIS/Include -I../../../homeboxk24 -I../../../../crypto -I../../../../protocol -I../../../../devices -I../../../../devices/MK24F25612/drivers -I../../../../devices/MK24F25612/utilities -I../../../../devices/MK24F25612 -std=gnu99 -mapcs $(CFLAGS) -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" -c -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

source/version.o: ../../../../project_wifi/bootloaderk24/sources/version.c
	@echo 'Building file: $<'
	@echo 'Invoking: Cross ARM C Compiler'
	arm-none-eabi-gcc -mcpu=cortex-m4 -mthumb -mfloat-abi=hard -mfpu=fpv4-sp-d16 -Os -fmessage-length=0 -fsigned-char -ffunction-sections -fdata-sections -fno-common -ffreestanding -fno-builtin -Wall -DNDEBUG -DBOOTLOADER -DCPU_MK24FN256VDC12 -DTWR_K24F120M -DTOWER -I../.. -I../../include -I../../../CMSIS/Include -I../../../homeboxk24 -I../../../../crypto -I../../../../protocol -I../../../../devices -I../../../../devices/MK24F25612/drivers -I../../../../devices/MK24F25612/utilities -I../../../../devices/MK24F25612 -std=gnu99 -mapcs $(CFLAGS) -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" -c -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


