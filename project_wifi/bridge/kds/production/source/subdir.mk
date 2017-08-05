################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../../../protocol/fota.c \
../../../protocol/freertos_dspi.c \
../../../protocol/hb_protocol.c \
../../../homeboxk24/hbb_config.c \
../../../protocol/log_task.c \
../../../protocol/main.c \
../../../protocol/mode_ctl.c \
../../../protocol/pflash_hub.c \
../../../protocol/pflash_sensor.c \
../../../protocol/plc_if.c \
../../../protocol/protocol_2_17.c \
../../../protocol/rtc_fun.c \
../../../protocol/uart_plc.c \
../../../protocol/uart_ppp.c \
../../../bridge/version.c \
../../../protocol/wdog.c \
../../../protocol/wifi_module.c 

OBJS += \
./source/fota.o \
./source/freertos_dspi.o \
./source/hb_protocol.o \
./source/hbb_config.o \
./source/log_task.o \
./source/main.o \
./source/mode_ctl.o \
./source/pflash_hub.o \
./source/pflash_sensor.o \
./source/plc_if.o \
./source/protocol_2_17.o \
./source/rtc_fun.o \
./source/uart_plc.o \
./source/uart_ppp.o \
./source/version.o \
./source/wdog.o \
./source/wifi_module.o 

C_DEPS += \
./source/fota.d \
./source/freertos_dspi.d \
./source/hb_protocol.d \
./source/hbb_config.d \
./source/log_task.d \
./source/main.d \
./source/mode_ctl.d \
./source/pflash_hub.d \
./source/pflash_sensor.d \
./source/plc_if.d \
./source/protocol_2_17.d \
./source/rtc_fun.d \
./source/uart_plc.d \
./source/uart_ppp.d \
./source/version.d \
./source/wdog.d \
./source/wifi_module.d 


# Each subdirectory must supply rules for building sources it contributes
source/fota.o: ../../../protocol/fota.c
	@echo 'Building file: $<'
	@echo 'Invoking: Cross ARM C Compiler'
	arm-none-eabi-gcc -mcpu=cortex-m4 -mthumb -mfloat-abi=hard -mfpu=fpv4-sp-d16 -Os -fmessage-length=0 -fsigned-char -ffunction-sections -fdata-sections -fno-common -ffreestanding -fno-builtin -Wall -DNDEBUG -DPLC -DBRIDGE -DUSE_RTOS=1 -DCPU_MK24FN256VDC12 -DFSL_RTOS_FREE_RTOS -DTWR_K24F120M -DTOWER -I../../.. -I../../../homeboxk24 -I../../../rtos/freertos_8.2.3_k24/Source/portable/GCC/ARM_CM4F -I../../../rtos/freertos_8.2.3_k24/Source/include -I../../../rtos/freertos_8.2.3_k24/Source -I../../../CMSIS/Include -I../../../devices -I../../../devices/MK24F25612 -I../../../devices/MK24F25612/drivers -I../../../devices/MK24F25612/utilities -I../../../7139 -I../../../mqtt -I../../../mqtt/MQTTpacket -I../../../json -I../../../uplink -I../../../middleware/lwip_1.4.1 -I../../../middleware/lwip_1.4.1/port -I../../../middleware/lwip_1.4.1/port/arch -I../../../middleware/lwip_1.4.1/src -I../../../middleware/lwip_1.4.1/src/include -I../../../middleware/lwip_1.4.1/src/include/ipv4 -I../../../middleware/lwip_1.4.1/src/include/ipv4/lwip -I../../../middleware/lwip_1.4.1/src/include/ipv6 -I../../../middleware/lwip_1.4.1/src/include/ipv6/lwip -I../../../middleware/lwip_1.4.1/src/include/lwip -I../../../middleware/lwip_1.4.1/src/include/netif -I../../../middleware/lwip_1.4.1/src/include/posix -I../../../middleware/lwip_1.4.1/src/netif/ppp -I../../../http_client -I../../../modem_gprs -I../../../modem_flash -I../../../protocol -I../../../modem_plc -I../../../modem_plc/drv_st7580 -I../../../crypto -std=gnu99 -mapcs -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" -c -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

source/freertos_dspi.o: ../../../protocol/freertos_dspi.c
	@echo 'Building file: $<'
	@echo 'Invoking: Cross ARM C Compiler'
	arm-none-eabi-gcc -mcpu=cortex-m4 -mthumb -mfloat-abi=hard -mfpu=fpv4-sp-d16 -Os -fmessage-length=0 -fsigned-char -ffunction-sections -fdata-sections -fno-common -ffreestanding -fno-builtin -Wall -DNDEBUG -DPLC -DBRIDGE -DUSE_RTOS=1 -DCPU_MK24FN256VDC12 -DFSL_RTOS_FREE_RTOS -DTWR_K24F120M -DTOWER -I../../.. -I../../../homeboxk24 -I../../../rtos/freertos_8.2.3_k24/Source/portable/GCC/ARM_CM4F -I../../../rtos/freertos_8.2.3_k24/Source/include -I../../../rtos/freertos_8.2.3_k24/Source -I../../../CMSIS/Include -I../../../devices -I../../../devices/MK24F25612 -I../../../devices/MK24F25612/drivers -I../../../devices/MK24F25612/utilities -I../../../7139 -I../../../mqtt -I../../../mqtt/MQTTpacket -I../../../json -I../../../uplink -I../../../middleware/lwip_1.4.1 -I../../../middleware/lwip_1.4.1/port -I../../../middleware/lwip_1.4.1/port/arch -I../../../middleware/lwip_1.4.1/src -I../../../middleware/lwip_1.4.1/src/include -I../../../middleware/lwip_1.4.1/src/include/ipv4 -I../../../middleware/lwip_1.4.1/src/include/ipv4/lwip -I../../../middleware/lwip_1.4.1/src/include/ipv6 -I../../../middleware/lwip_1.4.1/src/include/ipv6/lwip -I../../../middleware/lwip_1.4.1/src/include/lwip -I../../../middleware/lwip_1.4.1/src/include/netif -I../../../middleware/lwip_1.4.1/src/include/posix -I../../../middleware/lwip_1.4.1/src/netif/ppp -I../../../http_client -I../../../modem_gprs -I../../../modem_flash -I../../../protocol -I../../../modem_plc -I../../../modem_plc/drv_st7580 -I../../../crypto -std=gnu99 -mapcs -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" -c -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

source/hb_protocol.o: ../../../protocol/hb_protocol.c
	@echo 'Building file: $<'
	@echo 'Invoking: Cross ARM C Compiler'
	arm-none-eabi-gcc -mcpu=cortex-m4 -mthumb -mfloat-abi=hard -mfpu=fpv4-sp-d16 -Os -fmessage-length=0 -fsigned-char -ffunction-sections -fdata-sections -fno-common -ffreestanding -fno-builtin -Wall -DNDEBUG -DPLC -DBRIDGE -DUSE_RTOS=1 -DCPU_MK24FN256VDC12 -DFSL_RTOS_FREE_RTOS -DTWR_K24F120M -DTOWER -I../../.. -I../../../homeboxk24 -I../../../rtos/freertos_8.2.3_k24/Source/portable/GCC/ARM_CM4F -I../../../rtos/freertos_8.2.3_k24/Source/include -I../../../rtos/freertos_8.2.3_k24/Source -I../../../CMSIS/Include -I../../../devices -I../../../devices/MK24F25612 -I../../../devices/MK24F25612/drivers -I../../../devices/MK24F25612/utilities -I../../../7139 -I../../../mqtt -I../../../mqtt/MQTTpacket -I../../../json -I../../../uplink -I../../../middleware/lwip_1.4.1 -I../../../middleware/lwip_1.4.1/port -I../../../middleware/lwip_1.4.1/port/arch -I../../../middleware/lwip_1.4.1/src -I../../../middleware/lwip_1.4.1/src/include -I../../../middleware/lwip_1.4.1/src/include/ipv4 -I../../../middleware/lwip_1.4.1/src/include/ipv4/lwip -I../../../middleware/lwip_1.4.1/src/include/ipv6 -I../../../middleware/lwip_1.4.1/src/include/ipv6/lwip -I../../../middleware/lwip_1.4.1/src/include/lwip -I../../../middleware/lwip_1.4.1/src/include/netif -I../../../middleware/lwip_1.4.1/src/include/posix -I../../../middleware/lwip_1.4.1/src/netif/ppp -I../../../http_client -I../../../modem_gprs -I../../../modem_flash -I../../../protocol -I../../../modem_plc -I../../../modem_plc/drv_st7580 -I../../../crypto -std=gnu99 -mapcs -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" -c -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

source/hbb_config.o: ../../../homeboxk24/hbb_config.c
	@echo 'Building file: $<'
	@echo 'Invoking: Cross ARM C Compiler'
	arm-none-eabi-gcc -mcpu=cortex-m4 -mthumb -mfloat-abi=hard -mfpu=fpv4-sp-d16 -Os -fmessage-length=0 -fsigned-char -ffunction-sections -fdata-sections -fno-common -ffreestanding -fno-builtin -Wall -DNDEBUG -DPLC -DBRIDGE -DUSE_RTOS=1 -DCPU_MK24FN256VDC12 -DFSL_RTOS_FREE_RTOS -DTWR_K24F120M -DTOWER -I../../.. -I../../../homeboxk24 -I../../../rtos/freertos_8.2.3_k24/Source/portable/GCC/ARM_CM4F -I../../../rtos/freertos_8.2.3_k24/Source/include -I../../../rtos/freertos_8.2.3_k24/Source -I../../../CMSIS/Include -I../../../devices -I../../../devices/MK24F25612 -I../../../devices/MK24F25612/drivers -I../../../devices/MK24F25612/utilities -I../../../7139 -I../../../mqtt -I../../../mqtt/MQTTpacket -I../../../json -I../../../uplink -I../../../middleware/lwip_1.4.1 -I../../../middleware/lwip_1.4.1/port -I../../../middleware/lwip_1.4.1/port/arch -I../../../middleware/lwip_1.4.1/src -I../../../middleware/lwip_1.4.1/src/include -I../../../middleware/lwip_1.4.1/src/include/ipv4 -I../../../middleware/lwip_1.4.1/src/include/ipv4/lwip -I../../../middleware/lwip_1.4.1/src/include/ipv6 -I../../../middleware/lwip_1.4.1/src/include/ipv6/lwip -I../../../middleware/lwip_1.4.1/src/include/lwip -I../../../middleware/lwip_1.4.1/src/include/netif -I../../../middleware/lwip_1.4.1/src/include/posix -I../../../middleware/lwip_1.4.1/src/netif/ppp -I../../../http_client -I../../../modem_gprs -I../../../modem_flash -I../../../protocol -I../../../modem_plc -I../../../modem_plc/drv_st7580 -I../../../crypto -std=gnu99 -mapcs -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" -c -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

source/log_task.o: ../../../protocol/log_task.c
	@echo 'Building file: $<'
	@echo 'Invoking: Cross ARM C Compiler'
	arm-none-eabi-gcc -mcpu=cortex-m4 -mthumb -mfloat-abi=hard -mfpu=fpv4-sp-d16 -Os -fmessage-length=0 -fsigned-char -ffunction-sections -fdata-sections -fno-common -ffreestanding -fno-builtin -Wall -DNDEBUG -DPLC -DBRIDGE -DUSE_RTOS=1 -DCPU_MK24FN256VDC12 -DFSL_RTOS_FREE_RTOS -DTWR_K24F120M -DTOWER -I../../.. -I../../../homeboxk24 -I../../../rtos/freertos_8.2.3_k24/Source/portable/GCC/ARM_CM4F -I../../../rtos/freertos_8.2.3_k24/Source/include -I../../../rtos/freertos_8.2.3_k24/Source -I../../../CMSIS/Include -I../../../devices -I../../../devices/MK24F25612 -I../../../devices/MK24F25612/drivers -I../../../devices/MK24F25612/utilities -I../../../7139 -I../../../mqtt -I../../../mqtt/MQTTpacket -I../../../json -I../../../uplink -I../../../middleware/lwip_1.4.1 -I../../../middleware/lwip_1.4.1/port -I../../../middleware/lwip_1.4.1/port/arch -I../../../middleware/lwip_1.4.1/src -I../../../middleware/lwip_1.4.1/src/include -I../../../middleware/lwip_1.4.1/src/include/ipv4 -I../../../middleware/lwip_1.4.1/src/include/ipv4/lwip -I../../../middleware/lwip_1.4.1/src/include/ipv6 -I../../../middleware/lwip_1.4.1/src/include/ipv6/lwip -I../../../middleware/lwip_1.4.1/src/include/lwip -I../../../middleware/lwip_1.4.1/src/include/netif -I../../../middleware/lwip_1.4.1/src/include/posix -I../../../middleware/lwip_1.4.1/src/netif/ppp -I../../../http_client -I../../../modem_gprs -I../../../modem_flash -I../../../protocol -I../../../modem_plc -I../../../modem_plc/drv_st7580 -I../../../crypto -std=gnu99 -mapcs -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" -c -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

source/main.o: ../../../protocol/main.c
	@echo 'Building file: $<'
	@echo 'Invoking: Cross ARM C Compiler'
	arm-none-eabi-gcc -mcpu=cortex-m4 -mthumb -mfloat-abi=hard -mfpu=fpv4-sp-d16 -Os -fmessage-length=0 -fsigned-char -ffunction-sections -fdata-sections -fno-common -ffreestanding -fno-builtin -Wall -DNDEBUG -DPLC -DBRIDGE -DUSE_RTOS=1 -DCPU_MK24FN256VDC12 -DFSL_RTOS_FREE_RTOS -DTWR_K24F120M -DTOWER -I../../.. -I../../../homeboxk24 -I../../../rtos/freertos_8.2.3_k24/Source/portable/GCC/ARM_CM4F -I../../../rtos/freertos_8.2.3_k24/Source/include -I../../../rtos/freertos_8.2.3_k24/Source -I../../../CMSIS/Include -I../../../devices -I../../../devices/MK24F25612 -I../../../devices/MK24F25612/drivers -I../../../devices/MK24F25612/utilities -I../../../7139 -I../../../mqtt -I../../../mqtt/MQTTpacket -I../../../json -I../../../uplink -I../../../middleware/lwip_1.4.1 -I../../../middleware/lwip_1.4.1/port -I../../../middleware/lwip_1.4.1/port/arch -I../../../middleware/lwip_1.4.1/src -I../../../middleware/lwip_1.4.1/src/include -I../../../middleware/lwip_1.4.1/src/include/ipv4 -I../../../middleware/lwip_1.4.1/src/include/ipv4/lwip -I../../../middleware/lwip_1.4.1/src/include/ipv6 -I../../../middleware/lwip_1.4.1/src/include/ipv6/lwip -I../../../middleware/lwip_1.4.1/src/include/lwip -I../../../middleware/lwip_1.4.1/src/include/netif -I../../../middleware/lwip_1.4.1/src/include/posix -I../../../middleware/lwip_1.4.1/src/netif/ppp -I../../../http_client -I../../../modem_gprs -I../../../modem_flash -I../../../protocol -I../../../modem_plc -I../../../modem_plc/drv_st7580 -I../../../crypto -std=gnu99 -mapcs -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" -c -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

source/mode_ctl.o: ../../../protocol/mode_ctl.c
	@echo 'Building file: $<'
	@echo 'Invoking: Cross ARM C Compiler'
	arm-none-eabi-gcc -mcpu=cortex-m4 -mthumb -mfloat-abi=hard -mfpu=fpv4-sp-d16 -Os -fmessage-length=0 -fsigned-char -ffunction-sections -fdata-sections -fno-common -ffreestanding -fno-builtin -Wall -DNDEBUG -DPLC -DBRIDGE -DUSE_RTOS=1 -DCPU_MK24FN256VDC12 -DFSL_RTOS_FREE_RTOS -DTWR_K24F120M -DTOWER -I../../.. -I../../../homeboxk24 -I../../../rtos/freertos_8.2.3_k24/Source/portable/GCC/ARM_CM4F -I../../../rtos/freertos_8.2.3_k24/Source/include -I../../../rtos/freertos_8.2.3_k24/Source -I../../../CMSIS/Include -I../../../devices -I../../../devices/MK24F25612 -I../../../devices/MK24F25612/drivers -I../../../devices/MK24F25612/utilities -I../../../7139 -I../../../mqtt -I../../../mqtt/MQTTpacket -I../../../json -I../../../uplink -I../../../middleware/lwip_1.4.1 -I../../../middleware/lwip_1.4.1/port -I../../../middleware/lwip_1.4.1/port/arch -I../../../middleware/lwip_1.4.1/src -I../../../middleware/lwip_1.4.1/src/include -I../../../middleware/lwip_1.4.1/src/include/ipv4 -I../../../middleware/lwip_1.4.1/src/include/ipv4/lwip -I../../../middleware/lwip_1.4.1/src/include/ipv6 -I../../../middleware/lwip_1.4.1/src/include/ipv6/lwip -I../../../middleware/lwip_1.4.1/src/include/lwip -I../../../middleware/lwip_1.4.1/src/include/netif -I../../../middleware/lwip_1.4.1/src/include/posix -I../../../middleware/lwip_1.4.1/src/netif/ppp -I../../../http_client -I../../../modem_gprs -I../../../modem_flash -I../../../protocol -I../../../modem_plc -I../../../modem_plc/drv_st7580 -I../../../crypto -std=gnu99 -mapcs -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" -c -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

source/pflash_hub.o: ../../../protocol/pflash_hub.c
	@echo 'Building file: $<'
	@echo 'Invoking: Cross ARM C Compiler'
	arm-none-eabi-gcc -mcpu=cortex-m4 -mthumb -mfloat-abi=hard -mfpu=fpv4-sp-d16 -Os -fmessage-length=0 -fsigned-char -ffunction-sections -fdata-sections -fno-common -ffreestanding -fno-builtin -Wall -DNDEBUG -DPLC -DBRIDGE -DUSE_RTOS=1 -DCPU_MK24FN256VDC12 -DFSL_RTOS_FREE_RTOS -DTWR_K24F120M -DTOWER -I../../.. -I../../../homeboxk24 -I../../../rtos/freertos_8.2.3_k24/Source/portable/GCC/ARM_CM4F -I../../../rtos/freertos_8.2.3_k24/Source/include -I../../../rtos/freertos_8.2.3_k24/Source -I../../../CMSIS/Include -I../../../devices -I../../../devices/MK24F25612 -I../../../devices/MK24F25612/drivers -I../../../devices/MK24F25612/utilities -I../../../7139 -I../../../mqtt -I../../../mqtt/MQTTpacket -I../../../json -I../../../uplink -I../../../middleware/lwip_1.4.1 -I../../../middleware/lwip_1.4.1/port -I../../../middleware/lwip_1.4.1/port/arch -I../../../middleware/lwip_1.4.1/src -I../../../middleware/lwip_1.4.1/src/include -I../../../middleware/lwip_1.4.1/src/include/ipv4 -I../../../middleware/lwip_1.4.1/src/include/ipv4/lwip -I../../../middleware/lwip_1.4.1/src/include/ipv6 -I../../../middleware/lwip_1.4.1/src/include/ipv6/lwip -I../../../middleware/lwip_1.4.1/src/include/lwip -I../../../middleware/lwip_1.4.1/src/include/netif -I../../../middleware/lwip_1.4.1/src/include/posix -I../../../middleware/lwip_1.4.1/src/netif/ppp -I../../../http_client -I../../../modem_gprs -I../../../modem_flash -I../../../protocol -I../../../modem_plc -I../../../modem_plc/drv_st7580 -I../../../crypto -std=gnu99 -mapcs -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" -c -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

source/pflash_sensor.o: ../../../protocol/pflash_sensor.c
	@echo 'Building file: $<'
	@echo 'Invoking: Cross ARM C Compiler'
	arm-none-eabi-gcc -mcpu=cortex-m4 -mthumb -mfloat-abi=hard -mfpu=fpv4-sp-d16 -Os -fmessage-length=0 -fsigned-char -ffunction-sections -fdata-sections -fno-common -ffreestanding -fno-builtin -Wall -DNDEBUG -DPLC -DBRIDGE -DUSE_RTOS=1 -DCPU_MK24FN256VDC12 -DFSL_RTOS_FREE_RTOS -DTWR_K24F120M -DTOWER -I../../.. -I../../../homeboxk24 -I../../../rtos/freertos_8.2.3_k24/Source/portable/GCC/ARM_CM4F -I../../../rtos/freertos_8.2.3_k24/Source/include -I../../../rtos/freertos_8.2.3_k24/Source -I../../../CMSIS/Include -I../../../devices -I../../../devices/MK24F25612 -I../../../devices/MK24F25612/drivers -I../../../devices/MK24F25612/utilities -I../../../7139 -I../../../mqtt -I../../../mqtt/MQTTpacket -I../../../json -I../../../uplink -I../../../middleware/lwip_1.4.1 -I../../../middleware/lwip_1.4.1/port -I../../../middleware/lwip_1.4.1/port/arch -I../../../middleware/lwip_1.4.1/src -I../../../middleware/lwip_1.4.1/src/include -I../../../middleware/lwip_1.4.1/src/include/ipv4 -I../../../middleware/lwip_1.4.1/src/include/ipv4/lwip -I../../../middleware/lwip_1.4.1/src/include/ipv6 -I../../../middleware/lwip_1.4.1/src/include/ipv6/lwip -I../../../middleware/lwip_1.4.1/src/include/lwip -I../../../middleware/lwip_1.4.1/src/include/netif -I../../../middleware/lwip_1.4.1/src/include/posix -I../../../middleware/lwip_1.4.1/src/netif/ppp -I../../../http_client -I../../../modem_gprs -I../../../modem_flash -I../../../protocol -I../../../modem_plc -I../../../modem_plc/drv_st7580 -I../../../crypto -std=gnu99 -mapcs -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" -c -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

source/plc_if.o: ../../../protocol/plc_if.c
	@echo 'Building file: $<'
	@echo 'Invoking: Cross ARM C Compiler'
	arm-none-eabi-gcc -mcpu=cortex-m4 -mthumb -mfloat-abi=hard -mfpu=fpv4-sp-d16 -Os -fmessage-length=0 -fsigned-char -ffunction-sections -fdata-sections -fno-common -ffreestanding -fno-builtin -Wall -DNDEBUG -DPLC -DBRIDGE -DUSE_RTOS=1 -DCPU_MK24FN256VDC12 -DFSL_RTOS_FREE_RTOS -DTWR_K24F120M -DTOWER -I../../.. -I../../../homeboxk24 -I../../../rtos/freertos_8.2.3_k24/Source/portable/GCC/ARM_CM4F -I../../../rtos/freertos_8.2.3_k24/Source/include -I../../../rtos/freertos_8.2.3_k24/Source -I../../../CMSIS/Include -I../../../devices -I../../../devices/MK24F25612 -I../../../devices/MK24F25612/drivers -I../../../devices/MK24F25612/utilities -I../../../7139 -I../../../mqtt -I../../../mqtt/MQTTpacket -I../../../json -I../../../uplink -I../../../middleware/lwip_1.4.1 -I../../../middleware/lwip_1.4.1/port -I../../../middleware/lwip_1.4.1/port/arch -I../../../middleware/lwip_1.4.1/src -I../../../middleware/lwip_1.4.1/src/include -I../../../middleware/lwip_1.4.1/src/include/ipv4 -I../../../middleware/lwip_1.4.1/src/include/ipv4/lwip -I../../../middleware/lwip_1.4.1/src/include/ipv6 -I../../../middleware/lwip_1.4.1/src/include/ipv6/lwip -I../../../middleware/lwip_1.4.1/src/include/lwip -I../../../middleware/lwip_1.4.1/src/include/netif -I../../../middleware/lwip_1.4.1/src/include/posix -I../../../middleware/lwip_1.4.1/src/netif/ppp -I../../../http_client -I../../../modem_gprs -I../../../modem_flash -I../../../protocol -I../../../modem_plc -I../../../modem_plc/drv_st7580 -I../../../crypto -std=gnu99 -mapcs -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" -c -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

source/protocol_2_17.o: ../../../protocol/protocol_2_17.c
	@echo 'Building file: $<'
	@echo 'Invoking: Cross ARM C Compiler'
	arm-none-eabi-gcc -mcpu=cortex-m4 -mthumb -mfloat-abi=hard -mfpu=fpv4-sp-d16 -Os -fmessage-length=0 -fsigned-char -ffunction-sections -fdata-sections -fno-common -ffreestanding -fno-builtin -Wall -DNDEBUG -DPLC -DBRIDGE -DUSE_RTOS=1 -DCPU_MK24FN256VDC12 -DFSL_RTOS_FREE_RTOS -DTWR_K24F120M -DTOWER -I../../.. -I../../../homeboxk24 -I../../../rtos/freertos_8.2.3_k24/Source/portable/GCC/ARM_CM4F -I../../../rtos/freertos_8.2.3_k24/Source/include -I../../../rtos/freertos_8.2.3_k24/Source -I../../../CMSIS/Include -I../../../devices -I../../../devices/MK24F25612 -I../../../devices/MK24F25612/drivers -I../../../devices/MK24F25612/utilities -I../../../7139 -I../../../mqtt -I../../../mqtt/MQTTpacket -I../../../json -I../../../uplink -I../../../middleware/lwip_1.4.1 -I../../../middleware/lwip_1.4.1/port -I../../../middleware/lwip_1.4.1/port/arch -I../../../middleware/lwip_1.4.1/src -I../../../middleware/lwip_1.4.1/src/include -I../../../middleware/lwip_1.4.1/src/include/ipv4 -I../../../middleware/lwip_1.4.1/src/include/ipv4/lwip -I../../../middleware/lwip_1.4.1/src/include/ipv6 -I../../../middleware/lwip_1.4.1/src/include/ipv6/lwip -I../../../middleware/lwip_1.4.1/src/include/lwip -I../../../middleware/lwip_1.4.1/src/include/netif -I../../../middleware/lwip_1.4.1/src/include/posix -I../../../middleware/lwip_1.4.1/src/netif/ppp -I../../../http_client -I../../../modem_gprs -I../../../modem_flash -I../../../protocol -I../../../modem_plc -I../../../modem_plc/drv_st7580 -I../../../crypto -std=gnu99 -mapcs -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" -c -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

source/rtc_fun.o: ../../../protocol/rtc_fun.c
	@echo 'Building file: $<'
	@echo 'Invoking: Cross ARM C Compiler'
	arm-none-eabi-gcc -mcpu=cortex-m4 -mthumb -mfloat-abi=hard -mfpu=fpv4-sp-d16 -Os -fmessage-length=0 -fsigned-char -ffunction-sections -fdata-sections -fno-common -ffreestanding -fno-builtin -Wall -DNDEBUG -DPLC -DBRIDGE -DUSE_RTOS=1 -DCPU_MK24FN256VDC12 -DFSL_RTOS_FREE_RTOS -DTWR_K24F120M -DTOWER -I../../.. -I../../../homeboxk24 -I../../../rtos/freertos_8.2.3_k24/Source/portable/GCC/ARM_CM4F -I../../../rtos/freertos_8.2.3_k24/Source/include -I../../../rtos/freertos_8.2.3_k24/Source -I../../../CMSIS/Include -I../../../devices -I../../../devices/MK24F25612 -I../../../devices/MK24F25612/drivers -I../../../devices/MK24F25612/utilities -I../../../7139 -I../../../mqtt -I../../../mqtt/MQTTpacket -I../../../json -I../../../uplink -I../../../middleware/lwip_1.4.1 -I../../../middleware/lwip_1.4.1/port -I../../../middleware/lwip_1.4.1/port/arch -I../../../middleware/lwip_1.4.1/src -I../../../middleware/lwip_1.4.1/src/include -I../../../middleware/lwip_1.4.1/src/include/ipv4 -I../../../middleware/lwip_1.4.1/src/include/ipv4/lwip -I../../../middleware/lwip_1.4.1/src/include/ipv6 -I../../../middleware/lwip_1.4.1/src/include/ipv6/lwip -I../../../middleware/lwip_1.4.1/src/include/lwip -I../../../middleware/lwip_1.4.1/src/include/netif -I../../../middleware/lwip_1.4.1/src/include/posix -I../../../middleware/lwip_1.4.1/src/netif/ppp -I../../../http_client -I../../../modem_gprs -I../../../modem_flash -I../../../protocol -I../../../modem_plc -I../../../modem_plc/drv_st7580 -I../../../crypto -std=gnu99 -mapcs -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" -c -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

source/uart_plc.o: ../../../protocol/uart_plc.c
	@echo 'Building file: $<'
	@echo 'Invoking: Cross ARM C Compiler'
	arm-none-eabi-gcc -mcpu=cortex-m4 -mthumb -mfloat-abi=hard -mfpu=fpv4-sp-d16 -Os -fmessage-length=0 -fsigned-char -ffunction-sections -fdata-sections -fno-common -ffreestanding -fno-builtin -Wall -DNDEBUG -DPLC -DBRIDGE -DUSE_RTOS=1 -DCPU_MK24FN256VDC12 -DFSL_RTOS_FREE_RTOS -DTWR_K24F120M -DTOWER -I../../.. -I../../../homeboxk24 -I../../../rtos/freertos_8.2.3_k24/Source/portable/GCC/ARM_CM4F -I../../../rtos/freertos_8.2.3_k24/Source/include -I../../../rtos/freertos_8.2.3_k24/Source -I../../../CMSIS/Include -I../../../devices -I../../../devices/MK24F25612 -I../../../devices/MK24F25612/drivers -I../../../devices/MK24F25612/utilities -I../../../7139 -I../../../mqtt -I../../../mqtt/MQTTpacket -I../../../json -I../../../uplink -I../../../middleware/lwip_1.4.1 -I../../../middleware/lwip_1.4.1/port -I../../../middleware/lwip_1.4.1/port/arch -I../../../middleware/lwip_1.4.1/src -I../../../middleware/lwip_1.4.1/src/include -I../../../middleware/lwip_1.4.1/src/include/ipv4 -I../../../middleware/lwip_1.4.1/src/include/ipv4/lwip -I../../../middleware/lwip_1.4.1/src/include/ipv6 -I../../../middleware/lwip_1.4.1/src/include/ipv6/lwip -I../../../middleware/lwip_1.4.1/src/include/lwip -I../../../middleware/lwip_1.4.1/src/include/netif -I../../../middleware/lwip_1.4.1/src/include/posix -I../../../middleware/lwip_1.4.1/src/netif/ppp -I../../../http_client -I../../../modem_gprs -I../../../modem_flash -I../../../protocol -I../../../modem_plc -I../../../modem_plc/drv_st7580 -I../../../crypto -std=gnu99 -mapcs -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" -c -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

source/uart_ppp.o: ../../../protocol/uart_ppp.c
	@echo 'Building file: $<'
	@echo 'Invoking: Cross ARM C Compiler'
	arm-none-eabi-gcc -mcpu=cortex-m4 -mthumb -mfloat-abi=hard -mfpu=fpv4-sp-d16 -Os -fmessage-length=0 -fsigned-char -ffunction-sections -fdata-sections -fno-common -ffreestanding -fno-builtin -Wall -DNDEBUG -DPLC -DBRIDGE -DUSE_RTOS=1 -DCPU_MK24FN256VDC12 -DFSL_RTOS_FREE_RTOS -DTWR_K24F120M -DTOWER -I../../.. -I../../../homeboxk24 -I../../../rtos/freertos_8.2.3_k24/Source/portable/GCC/ARM_CM4F -I../../../rtos/freertos_8.2.3_k24/Source/include -I../../../rtos/freertos_8.2.3_k24/Source -I../../../CMSIS/Include -I../../../devices -I../../../devices/MK24F25612 -I../../../devices/MK24F25612/drivers -I../../../devices/MK24F25612/utilities -I../../../7139 -I../../../mqtt -I../../../mqtt/MQTTpacket -I../../../json -I../../../uplink -I../../../middleware/lwip_1.4.1 -I../../../middleware/lwip_1.4.1/port -I../../../middleware/lwip_1.4.1/port/arch -I../../../middleware/lwip_1.4.1/src -I../../../middleware/lwip_1.4.1/src/include -I../../../middleware/lwip_1.4.1/src/include/ipv4 -I../../../middleware/lwip_1.4.1/src/include/ipv4/lwip -I../../../middleware/lwip_1.4.1/src/include/ipv6 -I../../../middleware/lwip_1.4.1/src/include/ipv6/lwip -I../../../middleware/lwip_1.4.1/src/include/lwip -I../../../middleware/lwip_1.4.1/src/include/netif -I../../../middleware/lwip_1.4.1/src/include/posix -I../../../middleware/lwip_1.4.1/src/netif/ppp -I../../../http_client -I../../../modem_gprs -I../../../modem_flash -I../../../protocol -I../../../modem_plc -I../../../modem_plc/drv_st7580 -I../../../crypto -std=gnu99 -mapcs -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" -c -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

source/version.o: ../../../bridge/version.c
	@echo 'Building file: $<'
	@echo 'Invoking: Cross ARM C Compiler'
	arm-none-eabi-gcc -mcpu=cortex-m4 -mthumb -mfloat-abi=hard -mfpu=fpv4-sp-d16 -Os -fmessage-length=0 -fsigned-char -ffunction-sections -fdata-sections -fno-common -ffreestanding -fno-builtin -Wall -DNDEBUG -D__SVN_BRANCH=\"$(SVN_BRANCH)\" -D__SVN_REV=\"$(SVN_REV)\" -DPLC -DBRIDGE -DUSE_RTOS=1 -DCPU_MK24FN256VDC12 -DFSL_RTOS_FREE_RTOS -DTWR_K24F120M -DTOWER -I../../.. -I../../../homeboxk24 -I../../../rtos/freertos_8.2.3_k24/Source/portable/GCC/ARM_CM4F -I../../../rtos/freertos_8.2.3_k24/Source/include -I../../../rtos/freertos_8.2.3_k24/Source -I../../../CMSIS/Include -I../../../devices -I../../../devices/MK24F25612 -I../../../devices/MK24F25612/drivers -I../../../devices/MK24F25612/utilities -I../../../7139 -I../../../mqtt -I../../../mqtt/MQTTpacket -I../../../json -I../../../uplink -I../../../middleware/lwip_1.4.1 -I../../../middleware/lwip_1.4.1/port -I../../../middleware/lwip_1.4.1/port/arch -I../../../middleware/lwip_1.4.1/src -I../../../middleware/lwip_1.4.1/src/include -I../../../middleware/lwip_1.4.1/src/include/ipv4 -I../../../middleware/lwip_1.4.1/src/include/ipv4/lwip -I../../../middleware/lwip_1.4.1/src/include/ipv6 -I../../../middleware/lwip_1.4.1/src/include/ipv6/lwip -I../../../middleware/lwip_1.4.1/src/include/lwip -I../../../middleware/lwip_1.4.1/src/include/netif -I../../../middleware/lwip_1.4.1/src/include/posix -I../../../middleware/lwip_1.4.1/src/netif/ppp -I../../../http_client -I../../../modem_gprs -I../../../modem_flash -I../../../protocol -I../../../modem_plc -I../../../modem_plc/drv_st7580 -I../../../crypto -std=gnu99 -mapcs -MMD -MP -MF"$(@:%.o=%.d)" -MT"source/version.d" -c -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

source/wdog.o: ../../../protocol/wdog.c
	@echo 'Building file: $<'
	@echo 'Invoking: Cross ARM C Compiler'
	arm-none-eabi-gcc -mcpu=cortex-m4 -mthumb -mfloat-abi=hard -mfpu=fpv4-sp-d16 -Os -fmessage-length=0 -fsigned-char -ffunction-sections -fdata-sections -fno-common -ffreestanding -fno-builtin -Wall -DNDEBUG -DPLC -DBRIDGE -DUSE_RTOS=1 -DCPU_MK24FN256VDC12 -DFSL_RTOS_FREE_RTOS -DTWR_K24F120M -DTOWER -I../../.. -I../../../homeboxk24 -I../../../rtos/freertos_8.2.3_k24/Source/portable/GCC/ARM_CM4F -I../../../rtos/freertos_8.2.3_k24/Source/include -I../../../rtos/freertos_8.2.3_k24/Source -I../../../CMSIS/Include -I../../../devices -I../../../devices/MK24F25612 -I../../../devices/MK24F25612/drivers -I../../../devices/MK24F25612/utilities -I../../../7139 -I../../../mqtt -I../../../mqtt/MQTTpacket -I../../../json -I../../../uplink -I../../../middleware/lwip_1.4.1 -I../../../middleware/lwip_1.4.1/port -I../../../middleware/lwip_1.4.1/port/arch -I../../../middleware/lwip_1.4.1/src -I../../../middleware/lwip_1.4.1/src/include -I../../../middleware/lwip_1.4.1/src/include/ipv4 -I../../../middleware/lwip_1.4.1/src/include/ipv4/lwip -I../../../middleware/lwip_1.4.1/src/include/ipv6 -I../../../middleware/lwip_1.4.1/src/include/ipv6/lwip -I../../../middleware/lwip_1.4.1/src/include/lwip -I../../../middleware/lwip_1.4.1/src/include/netif -I../../../middleware/lwip_1.4.1/src/include/posix -I../../../middleware/lwip_1.4.1/src/netif/ppp -I../../../http_client -I../../../modem_gprs -I../../../modem_flash -I../../../protocol -I../../../modem_plc -I../../../modem_plc/drv_st7580 -I../../../crypto -std=gnu99 -mapcs -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" -c -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

source/wifi_module.o: ../../../protocol/wifi_module.c
	@echo 'Building file: $<'
	@echo 'Invoking: Cross ARM C Compiler'
	arm-none-eabi-gcc -mcpu=cortex-m4 -mthumb -mfloat-abi=hard -mfpu=fpv4-sp-d16 -Os -fmessage-length=0 -fsigned-char -ffunction-sections -fdata-sections -fno-common -ffreestanding -fno-builtin -Wall -DNDEBUG -DPLC -DBRIDGE -DUSE_RTOS=1 -DCPU_MK24FN256VDC12 -DFSL_RTOS_FREE_RTOS -DTWR_K24F120M -DTOWER -I../../.. -I../../../homeboxk24 -I../../../rtos/freertos_8.2.3_k24/Source/portable/GCC/ARM_CM4F -I../../../rtos/freertos_8.2.3_k24/Source/include -I../../../rtos/freertos_8.2.3_k24/Source -I../../../CMSIS/Include -I../../../devices -I../../../devices/MK24F25612 -I../../../devices/MK24F25612/drivers -I../../../devices/MK24F25612/utilities -I../../../7139 -I../../../mqtt -I../../../mqtt/MQTTpacket -I../../../json -I../../../uplink -I../../../middleware/lwip_1.4.1 -I../../../middleware/lwip_1.4.1/port -I../../../middleware/lwip_1.4.1/port/arch -I../../../middleware/lwip_1.4.1/src -I../../../middleware/lwip_1.4.1/src/include -I../../../middleware/lwip_1.4.1/src/include/ipv4 -I../../../middleware/lwip_1.4.1/src/include/ipv4/lwip -I../../../middleware/lwip_1.4.1/src/include/ipv6 -I../../../middleware/lwip_1.4.1/src/include/ipv6/lwip -I../../../middleware/lwip_1.4.1/src/include/lwip -I../../../middleware/lwip_1.4.1/src/include/netif -I../../../middleware/lwip_1.4.1/src/include/posix -I../../../middleware/lwip_1.4.1/src/netif/ppp -I../../../http_client -I../../../modem_gprs -I../../../modem_flash -I../../../protocol -I../../../modem_plc -I../../../modem_plc/drv_st7580 -I../../../crypto -std=gnu99 -mapcs -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" -c -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


