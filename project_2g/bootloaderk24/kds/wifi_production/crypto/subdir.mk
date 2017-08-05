################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../../../crypto/aes256.c \
../../../crypto/sha1.c 

OBJS += \
./crypto/aes256.o \
./crypto/sha1.o 

C_DEPS += \
./crypto/aes256.d \
./crypto/sha1.d 


# Each subdirectory must supply rules for building sources it contributes
crypto/aes256.o: ../../../crypto/aes256.c
	@echo 'Building file: $<'
	@echo 'Invoking: Cross ARM C Compiler'
	arm-none-eabi-gcc -mcpu=cortex-m4 -mthumb -mfloat-abi=hard -mfpu=fpv4-sp-d16 -Os -fmessage-length=0 -fsigned-char -ffunction-sections -fdata-sections -fno-common -ffreestanding -fno-builtin -Wall -DNDEBUG -DWIFI_MODULE -DBOOTLOADER -DCPU_MK24FN256VDC12 -DTWR_K24F120M -DTOWER -I../.. -I../../include -I../../../protocol -I../../../CMSIS/Include -I../../../homeboxk24 -I../../../crypto -I../../../devices -I../../../devices/MK24F25612/drivers -I../../../devices/MK24F25612/utilities -I../../../devices/MK24F25612 -std=gnu99 -mapcs $(CFLAGS) -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" -c -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

crypto/sha1.o: ../../../crypto/sha1.c
	@echo 'Building file: $<'
	@echo 'Invoking: Cross ARM C Compiler'
	arm-none-eabi-gcc -mcpu=cortex-m4 -mthumb -mfloat-abi=hard -mfpu=fpv4-sp-d16 -Os -fmessage-length=0 -fsigned-char -ffunction-sections -fdata-sections -fno-common -ffreestanding -fno-builtin -Wall -DNDEBUG -DWIFI_MODULE -DBOOTLOADER -DCPU_MK24FN256VDC12 -DTWR_K24F120M -DTOWER -I../.. -I../../include -I../../../protocol -I../../../CMSIS/Include -I../../../homeboxk24 -I../../../crypto -I../../../devices -I../../../devices/MK24F25612/drivers -I../../../devices/MK24F25612/utilities -I../../../devices/MK24F25612 -std=gnu99 -mapcs $(CFLAGS) -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" -c -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


