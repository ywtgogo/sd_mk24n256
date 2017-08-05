################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../../../../project_wifi/homeboxk24/board.c \
../../../../project_wifi/homeboxk24/clock_config.c \
../../../../project_wifi/bootloaderk24/sources/pin_mux.c 

OBJS += \
./board/board.o \
./board/clock_config.o \
./board/pin_mux.o 

C_DEPS += \
./board/board.d \
./board/clock_config.d \
./board/pin_mux.d 


# Each subdirectory must supply rules for building sources it contributes
board/board.o: ../../../../project_wifi/homeboxk24/board.c
	@echo 'Building file: $<'
	@echo 'Invoking: Cross ARM C Compiler'
	arm-none-eabi-gcc -mcpu=cortex-m4 -mthumb -mfloat-abi=hard -mfpu=fpv4-sp-d16 -Os -fmessage-length=0 -fsigned-char -ffunction-sections -fdata-sections -fno-common -ffreestanding -fno-builtin -Wall -DNDEBUG -DWIFI_MODULE -DBOOTLOADER -DCPU_MK24FN256VDC12 -DTWR_K24F120M -DTOWER -I../.. -I../../include -I../../../CMSIS/Include -I../../../homeboxk24 -I../../../../crypto -I../../../../protocol -I../../../../devices -I../../../../devices/MK24F25612/drivers -I../../../../devices/MK24F25612/utilities -I../../../../devices/MK24F25612 -std=gnu99 -mapcs $(CFLAGS) -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" -c -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

board/clock_config.o: ../../../../project_wifi/homeboxk24/clock_config.c
	@echo 'Building file: $<'
	@echo 'Invoking: Cross ARM C Compiler'
	arm-none-eabi-gcc -mcpu=cortex-m4 -mthumb -mfloat-abi=hard -mfpu=fpv4-sp-d16 -Os -fmessage-length=0 -fsigned-char -ffunction-sections -fdata-sections -fno-common -ffreestanding -fno-builtin -Wall -DNDEBUG -DWIFI_MODULE -DBOOTLOADER -DCPU_MK24FN256VDC12 -DTWR_K24F120M -DTOWER -I../.. -I../../include -I../../../CMSIS/Include -I../../../homeboxk24 -I../../../../crypto -I../../../../protocol -I../../../../devices -I../../../../devices/MK24F25612/drivers -I../../../../devices/MK24F25612/utilities -I../../../../devices/MK24F25612 -std=gnu99 -mapcs $(CFLAGS) -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" -c -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

board/pin_mux.o: ../../../../project_wifi/bootloaderk24/sources/pin_mux.c
	@echo 'Building file: $<'
	@echo 'Invoking: Cross ARM C Compiler'
	arm-none-eabi-gcc -mcpu=cortex-m4 -mthumb -mfloat-abi=hard -mfpu=fpv4-sp-d16 -Os -fmessage-length=0 -fsigned-char -ffunction-sections -fdata-sections -fno-common -ffreestanding -fno-builtin -Wall -DNDEBUG -DWIFI_MODULE -DBOOTLOADER -DCPU_MK24FN256VDC12 -DTWR_K24F120M -DTOWER -I../.. -I../../include -I../../../CMSIS/Include -I../../../homeboxk24 -I../../../../crypto -I../../../../protocol -I../../../../devices -I../../../../devices/MK24F25612/drivers -I../../../../devices/MK24F25612/utilities -I../../../../devices/MK24F25612 -std=gnu99 -mapcs $(CFLAGS) -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" -c -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


