################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
E:/sbox_all/project_wifi/modem_plc/test_ping_common.c \
E:/sbox_all/project_wifi/modem_plc/test_ping_sn.c 

OBJS += \
./modem_plc/test_ping_common.o \
./modem_plc/test_ping_sn.o 

C_DEPS += \
./modem_plc/test_ping_common.d \
./modem_plc/test_ping_sn.d 


# Each subdirectory must supply rules for building sources it contributes
modem_plc/test_ping_common.o: E:/sbox_all/project_wifi/modem_plc/test_ping_common.c
	@echo 'Building file: $<'
	@echo 'Invoking: Cross ARM C Compiler'
	arm-none-eabi-gcc -mcpu=cortex-m4 -mthumb -mfloat-abi=hard -mfpu=fpv4-sp-d16 -O0 -fmessage-length=0 -fsigned-char -ffunction-sections -fdata-sections -fno-common -ffreestanding -fno-builtin -Wall -Wredundant-decls  -g -DDEBUG -DWIFI_MODULE -DUSE_RTOS=1 -DCPU_MK24FN256VDC12 -DFSL_RTOS_FREE_RTOS -DTWR_K24F120M -DTOWER -DAUDIO_PLAY -DPRODUCE_TEST -I../../../../rtos/freertos_8.2.3_k24/Source/portable/GCC/ARM_CM4F -I../../../../rtos/freertos_8.2.3_k24/Source/include -I../../../../rtos/freertos_8.2.3_k24/Source -I../../../CMSIS/Include -I../../../../devices -I../../../../devices/MK24F25612 -I../../../../devices/MK24F25612/drivers -I../../../../devices/MK24F25612/utilities -I../.. -I../../../7139 -I../../../mqtt -I../../../mqtt/MQTTpacket -I../../../json -I../../../uplink -I../../../middleware/lwip_1.4.1 -I../../../middleware/lwip_1.4.1/port -I../../../middleware/lwip_1.4.1/port/arch -I../../../middleware/lwip_1.4.1/src -I../../../middleware/lwip_1.4.1/src/include -I../../../middleware/lwip_1.4.1/src/include/ipv4 -I../../../middleware/lwip_1.4.1/src/include/ipv4/lwip -I../../../middleware/lwip_1.4.1/src/include/ipv6 -I../../../middleware/lwip_1.4.1/src/include/ipv6/lwip -I../../../middleware/lwip_1.4.1/src/include/lwip -I../../../middleware/lwip_1.4.1/src/include/netif -I../../../middleware/lwip_1.4.1/src/include/posix -I../../../middleware/lwip_1.4.1/src/netif/ppp -I../../../http_client -I../../../modem_gprs -I../../../modem_flash -I../../../modem_plc/drv_st7580 -I../../../modem_plc/ -I../../../protocol -I../../../../protocol -I../../../../crypto -std=gnu99 -mapcs $(CFLAGS) -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" -c -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

modem_plc/test_ping_sn.o: E:/sbox_all/project_wifi/modem_plc/test_ping_sn.c
	@echo 'Building file: $<'
	@echo 'Invoking: Cross ARM C Compiler'
	arm-none-eabi-gcc -mcpu=cortex-m4 -mthumb -mfloat-abi=hard -mfpu=fpv4-sp-d16 -O0 -fmessage-length=0 -fsigned-char -ffunction-sections -fdata-sections -fno-common -ffreestanding -fno-builtin -Wall -Wredundant-decls  -g -DDEBUG -DWIFI_MODULE -DUSE_RTOS=1 -DCPU_MK24FN256VDC12 -DFSL_RTOS_FREE_RTOS -DTWR_K24F120M -DTOWER -DAUDIO_PLAY -DPRODUCE_TEST -I../../../../rtos/freertos_8.2.3_k24/Source/portable/GCC/ARM_CM4F -I../../../../rtos/freertos_8.2.3_k24/Source/include -I../../../../rtos/freertos_8.2.3_k24/Source -I../../../CMSIS/Include -I../../../../devices -I../../../../devices/MK24F25612 -I../../../../devices/MK24F25612/drivers -I../../../../devices/MK24F25612/utilities -I../.. -I../../../7139 -I../../../mqtt -I../../../mqtt/MQTTpacket -I../../../json -I../../../uplink -I../../../middleware/lwip_1.4.1 -I../../../middleware/lwip_1.4.1/port -I../../../middleware/lwip_1.4.1/port/arch -I../../../middleware/lwip_1.4.1/src -I../../../middleware/lwip_1.4.1/src/include -I../../../middleware/lwip_1.4.1/src/include/ipv4 -I../../../middleware/lwip_1.4.1/src/include/ipv4/lwip -I../../../middleware/lwip_1.4.1/src/include/ipv6 -I../../../middleware/lwip_1.4.1/src/include/ipv6/lwip -I../../../middleware/lwip_1.4.1/src/include/lwip -I../../../middleware/lwip_1.4.1/src/include/netif -I../../../middleware/lwip_1.4.1/src/include/posix -I../../../middleware/lwip_1.4.1/src/netif/ppp -I../../../http_client -I../../../modem_gprs -I../../../modem_flash -I../../../modem_plc/drv_st7580 -I../../../modem_plc/ -I../../../protocol -I../../../../protocol -I../../../../crypto -std=gnu99 -mapcs $(CFLAGS) -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" -c -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


