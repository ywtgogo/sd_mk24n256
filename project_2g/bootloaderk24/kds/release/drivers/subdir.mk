################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../../../devices/MK24F25612/drivers/fsl_adc16.c \
../../../devices/MK24F25612/drivers/fsl_clock.c \
../../../devices/MK24F25612/drivers/fsl_common.c \
../../../devices/MK24F25612/drivers/fsl_dspi.c \
../../../devices/MK24F25612/drivers/fsl_flash.c \
../../../devices/MK24F25612/drivers/fsl_gpio.c \
../../../devices/MK24F25612/drivers/fsl_pmc.c \
../../../devices/MK24F25612/drivers/fsl_smc.c \
../../../devices/MK24F25612/drivers/fsl_uart.c 

OBJS += \
./drivers/fsl_adc16.o \
./drivers/fsl_clock.o \
./drivers/fsl_common.o \
./drivers/fsl_dspi.o \
./drivers/fsl_flash.o \
./drivers/fsl_gpio.o \
./drivers/fsl_pmc.o \
./drivers/fsl_smc.o \
./drivers/fsl_uart.o 

C_DEPS += \
./drivers/fsl_adc16.d \
./drivers/fsl_clock.d \
./drivers/fsl_common.d \
./drivers/fsl_dspi.d \
./drivers/fsl_flash.d \
./drivers/fsl_gpio.d \
./drivers/fsl_pmc.d \
./drivers/fsl_smc.d \
./drivers/fsl_uart.d 


# Each subdirectory must supply rules for building sources it contributes
drivers/fsl_adc16.o: ../../../devices/MK24F25612/drivers/fsl_adc16.c
	@echo 'Building file: $<'
	@echo 'Invoking: Cross ARM C Compiler'
	arm-none-eabi-gcc -mcpu=cortex-m4 -mthumb -mfloat-abi=hard -mfpu=fpv4-sp-d16 -Os -fmessage-length=0 -fsigned-char -ffunction-sections -fdata-sections -fno-common -ffreestanding -fno-builtin -Wall -DNDEBUG -DBOOTLOADER -DCPU_MK24FN256VDC12 -DTWR_K24F120M -DTOWER -I../.. -I../../include -I../../../CMSIS/Include -I../../../homeboxk24 -I../../../crypto -I../../../devices -I../../../devices/MK24F25612/drivers -I../../../devices/MK24F25612/utilities -I../../../devices/MK24F25612 -std=gnu99 -mapcs -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" -c -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

drivers/fsl_clock.o: ../../../devices/MK24F25612/drivers/fsl_clock.c
	@echo 'Building file: $<'
	@echo 'Invoking: Cross ARM C Compiler'
	arm-none-eabi-gcc -mcpu=cortex-m4 -mthumb -mfloat-abi=hard -mfpu=fpv4-sp-d16 -Os -fmessage-length=0 -fsigned-char -ffunction-sections -fdata-sections -fno-common -ffreestanding -fno-builtin -Wall -DNDEBUG -DBOOTLOADER -DCPU_MK24FN256VDC12 -DTWR_K24F120M -DTOWER -I../.. -I../../include -I../../../CMSIS/Include -I../../../homeboxk24 -I../../../crypto -I../../../devices -I../../../devices/MK24F25612/drivers -I../../../devices/MK24F25612/utilities -I../../../devices/MK24F25612 -std=gnu99 -mapcs -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" -c -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

drivers/fsl_common.o: ../../../devices/MK24F25612/drivers/fsl_common.c
	@echo 'Building file: $<'
	@echo 'Invoking: Cross ARM C Compiler'
	arm-none-eabi-gcc -mcpu=cortex-m4 -mthumb -mfloat-abi=hard -mfpu=fpv4-sp-d16 -Os -fmessage-length=0 -fsigned-char -ffunction-sections -fdata-sections -fno-common -ffreestanding -fno-builtin -Wall -DNDEBUG -DBOOTLOADER -DCPU_MK24FN256VDC12 -DTWR_K24F120M -DTOWER -I../.. -I../../include -I../../../CMSIS/Include -I../../../homeboxk24 -I../../../crypto -I../../../devices -I../../../devices/MK24F25612/drivers -I../../../devices/MK24F25612/utilities -I../../../devices/MK24F25612 -std=gnu99 -mapcs -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" -c -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

drivers/fsl_dspi.o: ../../../devices/MK24F25612/drivers/fsl_dspi.c
	@echo 'Building file: $<'
	@echo 'Invoking: Cross ARM C Compiler'
	arm-none-eabi-gcc -mcpu=cortex-m4 -mthumb -mfloat-abi=hard -mfpu=fpv4-sp-d16 -Os -fmessage-length=0 -fsigned-char -ffunction-sections -fdata-sections -fno-common -ffreestanding -fno-builtin -Wall -DNDEBUG -DBOOTLOADER -DCPU_MK24FN256VDC12 -DTWR_K24F120M -DTOWER -I../.. -I../../include -I../../../CMSIS/Include -I../../../homeboxk24 -I../../../crypto -I../../../devices -I../../../devices/MK24F25612/drivers -I../../../devices/MK24F25612/utilities -I../../../devices/MK24F25612 -std=gnu99 -mapcs -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" -c -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

drivers/fsl_flash.o: ../../../devices/MK24F25612/drivers/fsl_flash.c
	@echo 'Building file: $<'
	@echo 'Invoking: Cross ARM C Compiler'
	arm-none-eabi-gcc -mcpu=cortex-m4 -mthumb -mfloat-abi=hard -mfpu=fpv4-sp-d16 -Os -fmessage-length=0 -fsigned-char -ffunction-sections -fdata-sections -fno-common -ffreestanding -fno-builtin -Wall -DNDEBUG -DBOOTLOADER -DCPU_MK24FN256VDC12 -DTWR_K24F120M -DTOWER -I../.. -I../../include -I../../../CMSIS/Include -I../../../homeboxk24 -I../../../crypto -I../../../devices -I../../../devices/MK24F25612/drivers -I../../../devices/MK24F25612/utilities -I../../../devices/MK24F25612 -std=gnu99 -mapcs -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" -c -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

drivers/fsl_gpio.o: ../../../devices/MK24F25612/drivers/fsl_gpio.c
	@echo 'Building file: $<'
	@echo 'Invoking: Cross ARM C Compiler'
	arm-none-eabi-gcc -mcpu=cortex-m4 -mthumb -mfloat-abi=hard -mfpu=fpv4-sp-d16 -Os -fmessage-length=0 -fsigned-char -ffunction-sections -fdata-sections -fno-common -ffreestanding -fno-builtin -Wall -DNDEBUG -DBOOTLOADER -DCPU_MK24FN256VDC12 -DTWR_K24F120M -DTOWER -I../.. -I../../include -I../../../CMSIS/Include -I../../../homeboxk24 -I../../../crypto -I../../../devices -I../../../devices/MK24F25612/drivers -I../../../devices/MK24F25612/utilities -I../../../devices/MK24F25612 -std=gnu99 -mapcs -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" -c -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

drivers/fsl_pmc.o: ../../../devices/MK24F25612/drivers/fsl_pmc.c
	@echo 'Building file: $<'
	@echo 'Invoking: Cross ARM C Compiler'
	arm-none-eabi-gcc -mcpu=cortex-m4 -mthumb -mfloat-abi=hard -mfpu=fpv4-sp-d16 -Os -fmessage-length=0 -fsigned-char -ffunction-sections -fdata-sections -fno-common -ffreestanding -fno-builtin -Wall -DNDEBUG -DBOOTLOADER -DCPU_MK24FN256VDC12 -DTWR_K24F120M -DTOWER -I../.. -I../../include -I../../../CMSIS/Include -I../../../homeboxk24 -I../../../crypto -I../../../devices -I../../../devices/MK24F25612/drivers -I../../../devices/MK24F25612/utilities -I../../../devices/MK24F25612 -std=gnu99 -mapcs -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" -c -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

drivers/fsl_smc.o: ../../../devices/MK24F25612/drivers/fsl_smc.c
	@echo 'Building file: $<'
	@echo 'Invoking: Cross ARM C Compiler'
	arm-none-eabi-gcc -mcpu=cortex-m4 -mthumb -mfloat-abi=hard -mfpu=fpv4-sp-d16 -Os -fmessage-length=0 -fsigned-char -ffunction-sections -fdata-sections -fno-common -ffreestanding -fno-builtin -Wall -DNDEBUG -DBOOTLOADER -DCPU_MK24FN256VDC12 -DTWR_K24F120M -DTOWER -I../.. -I../../include -I../../../CMSIS/Include -I../../../homeboxk24 -I../../../crypto -I../../../devices -I../../../devices/MK24F25612/drivers -I../../../devices/MK24F25612/utilities -I../../../devices/MK24F25612 -std=gnu99 -mapcs -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" -c -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

drivers/fsl_uart.o: ../../../devices/MK24F25612/drivers/fsl_uart.c
	@echo 'Building file: $<'
	@echo 'Invoking: Cross ARM C Compiler'
	arm-none-eabi-gcc -mcpu=cortex-m4 -mthumb -mfloat-abi=hard -mfpu=fpv4-sp-d16 -Os -fmessage-length=0 -fsigned-char -ffunction-sections -fdata-sections -fno-common -ffreestanding -fno-builtin -Wall -DNDEBUG -DBOOTLOADER -DCPU_MK24FN256VDC12 -DTWR_K24F120M -DTOWER -I../.. -I../../include -I../../../CMSIS/Include -I../../../homeboxk24 -I../../../crypto -I../../../devices -I../../../devices/MK24F25612/drivers -I../../../devices/MK24F25612/utilities -I../../../devices/MK24F25612 -std=gnu99 -mapcs -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" -c -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


