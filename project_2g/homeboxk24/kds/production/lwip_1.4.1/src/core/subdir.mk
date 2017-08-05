################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../../../../project_2g/middleware/lwip_1.4.1/src/core/def.c \
../../../../project_2g/middleware/lwip_1.4.1/src/core/dhcp.c \
../../../../project_2g/middleware/lwip_1.4.1/src/core/dns.c \
../../../../project_2g/middleware/lwip_1.4.1/src/core/init.c \
../../../../project_2g/middleware/lwip_1.4.1/src/core/lwip_timers.c \
../../../../project_2g/middleware/lwip_1.4.1/src/core/mem.c \
../../../../project_2g/middleware/lwip_1.4.1/src/core/memp.c \
../../../../project_2g/middleware/lwip_1.4.1/src/core/netif.c \
../../../../project_2g/middleware/lwip_1.4.1/src/core/pbuf.c \
../../../../project_2g/middleware/lwip_1.4.1/src/core/raw.c \
../../../../project_2g/middleware/lwip_1.4.1/src/core/stats.c \
../../../../project_2g/middleware/lwip_1.4.1/src/core/sys.c \
../../../../project_2g/middleware/lwip_1.4.1/src/core/tcp.c \
../../../../project_2g/middleware/lwip_1.4.1/src/core/tcp_in.c \
../../../../project_2g/middleware/lwip_1.4.1/src/core/tcp_out.c \
../../../../project_2g/middleware/lwip_1.4.1/src/core/udp.c 

OBJS += \
./lwip_1.4.1/src/core/def.o \
./lwip_1.4.1/src/core/dhcp.o \
./lwip_1.4.1/src/core/dns.o \
./lwip_1.4.1/src/core/init.o \
./lwip_1.4.1/src/core/lwip_timers.o \
./lwip_1.4.1/src/core/mem.o \
./lwip_1.4.1/src/core/memp.o \
./lwip_1.4.1/src/core/netif.o \
./lwip_1.4.1/src/core/pbuf.o \
./lwip_1.4.1/src/core/raw.o \
./lwip_1.4.1/src/core/stats.o \
./lwip_1.4.1/src/core/sys.o \
./lwip_1.4.1/src/core/tcp.o \
./lwip_1.4.1/src/core/tcp_in.o \
./lwip_1.4.1/src/core/tcp_out.o \
./lwip_1.4.1/src/core/udp.o 

C_DEPS += \
./lwip_1.4.1/src/core/def.d \
./lwip_1.4.1/src/core/dhcp.d \
./lwip_1.4.1/src/core/dns.d \
./lwip_1.4.1/src/core/init.d \
./lwip_1.4.1/src/core/lwip_timers.d \
./lwip_1.4.1/src/core/mem.d \
./lwip_1.4.1/src/core/memp.d \
./lwip_1.4.1/src/core/netif.d \
./lwip_1.4.1/src/core/pbuf.d \
./lwip_1.4.1/src/core/raw.d \
./lwip_1.4.1/src/core/stats.d \
./lwip_1.4.1/src/core/sys.d \
./lwip_1.4.1/src/core/tcp.d \
./lwip_1.4.1/src/core/tcp_in.d \
./lwip_1.4.1/src/core/tcp_out.d \
./lwip_1.4.1/src/core/udp.d 


# Each subdirectory must supply rules for building sources it contributes
lwip_1.4.1/src/core/def.o: ../../../../project_2g/middleware/lwip_1.4.1/src/core/def.c
	@echo 'Building file: $<'
	@echo 'Invoking: Cross ARM C Compiler'
	arm-none-eabi-gcc -mcpu=cortex-m4 -mthumb -mfloat-abi=hard -mfpu=fpv4-sp-d16 -Os -fmessage-length=0 -fsigned-char -ffunction-sections -fdata-sections -fno-common -ffreestanding -fno-builtin -Wall -Wredundant-decls -DNDEBUG -DUSE_RTOS=1 -DCPU_MK24FN256VDC12 -DFSL_RTOS_FREE_RTOS -DTWR_K24F120M -DTOWER -DAUDIO_PLAY -I../../../../rtos/freertos_8.2.3_k24/Source/portable/GCC/ARM_CM4F -I../../../mqtt/MQTTpacket -I../../../../rtos/freertos_8.2.3_k24/Source/include -I../../../../rtos/freertos_8.2.3_k24/Source -I../../../CMSIS/Include -I../../../../devices -I../../../../devices/MK24F25612 -I../../../../devices/MK24F25612/drivers -I../../../../devices/MK24F25612/utilities -I../.. -I../../../7139 -I../../../middleware/lwip_1.4.1 -I../../../middleware/lwip_1.4.1/port -I../../../middleware/lwip_1.4.1/port/arch -I../../../middleware/lwip_1.4.1/src -I../../../middleware/lwip_1.4.1/src/include -I../../../middleware/lwip_1.4.1/src/include/ipv4 -I../../../middleware/lwip_1.4.1/src/include/ipv4/lwip -I../../../middleware/lwip_1.4.1/src/include/ipv6 -I../../../middleware/lwip_1.4.1/src/include/ipv6/lwip -I../../../middleware/lwip_1.4.1/src/include/lwip -I../../../middleware/lwip_1.4.1/src/include/netif -I../../../middleware/lwip_1.4.1/src/include/posix -I../../../middleware/lwip_1.4.1/src/netif/ppp -I../../../http_client -I../../../modem_gprs -I../../../modem_flash -I../../../modem_plc -I../../../modem_plc/drv_st7580 -I../../../protocol -I../../../../protocol -I../../../json -I../../../mqtt -I../../../uplink -I../../../../crypto -std=gnu99 -mapcs $(CFLAGS) -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" -c -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

lwip_1.4.1/src/core/dhcp.o: ../../../../project_2g/middleware/lwip_1.4.1/src/core/dhcp.c
	@echo 'Building file: $<'
	@echo 'Invoking: Cross ARM C Compiler'
	arm-none-eabi-gcc -mcpu=cortex-m4 -mthumb -mfloat-abi=hard -mfpu=fpv4-sp-d16 -Os -fmessage-length=0 -fsigned-char -ffunction-sections -fdata-sections -fno-common -ffreestanding -fno-builtin -Wall -Wredundant-decls -DNDEBUG -DUSE_RTOS=1 -DCPU_MK24FN256VDC12 -DFSL_RTOS_FREE_RTOS -DTWR_K24F120M -DTOWER -DAUDIO_PLAY -I../../../../rtos/freertos_8.2.3_k24/Source/portable/GCC/ARM_CM4F -I../../../mqtt/MQTTpacket -I../../../../rtos/freertos_8.2.3_k24/Source/include -I../../../../rtos/freertos_8.2.3_k24/Source -I../../../CMSIS/Include -I../../../../devices -I../../../../devices/MK24F25612 -I../../../../devices/MK24F25612/drivers -I../../../../devices/MK24F25612/utilities -I../.. -I../../../7139 -I../../../middleware/lwip_1.4.1 -I../../../middleware/lwip_1.4.1/port -I../../../middleware/lwip_1.4.1/port/arch -I../../../middleware/lwip_1.4.1/src -I../../../middleware/lwip_1.4.1/src/include -I../../../middleware/lwip_1.4.1/src/include/ipv4 -I../../../middleware/lwip_1.4.1/src/include/ipv4/lwip -I../../../middleware/lwip_1.4.1/src/include/ipv6 -I../../../middleware/lwip_1.4.1/src/include/ipv6/lwip -I../../../middleware/lwip_1.4.1/src/include/lwip -I../../../middleware/lwip_1.4.1/src/include/netif -I../../../middleware/lwip_1.4.1/src/include/posix -I../../../middleware/lwip_1.4.1/src/netif/ppp -I../../../http_client -I../../../modem_gprs -I../../../modem_flash -I../../../modem_plc -I../../../modem_plc/drv_st7580 -I../../../protocol -I../../../../protocol -I../../../json -I../../../mqtt -I../../../uplink -I../../../../crypto -std=gnu99 -mapcs $(CFLAGS) -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" -c -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

lwip_1.4.1/src/core/dns.o: ../../../../project_2g/middleware/lwip_1.4.1/src/core/dns.c
	@echo 'Building file: $<'
	@echo 'Invoking: Cross ARM C Compiler'
	arm-none-eabi-gcc -mcpu=cortex-m4 -mthumb -mfloat-abi=hard -mfpu=fpv4-sp-d16 -Os -fmessage-length=0 -fsigned-char -ffunction-sections -fdata-sections -fno-common -ffreestanding -fno-builtin -Wall -Wredundant-decls -DNDEBUG -DUSE_RTOS=1 -DCPU_MK24FN256VDC12 -DFSL_RTOS_FREE_RTOS -DTWR_K24F120M -DTOWER -DAUDIO_PLAY -I../../../../rtos/freertos_8.2.3_k24/Source/portable/GCC/ARM_CM4F -I../../../mqtt/MQTTpacket -I../../../../rtos/freertos_8.2.3_k24/Source/include -I../../../../rtos/freertos_8.2.3_k24/Source -I../../../CMSIS/Include -I../../../../devices -I../../../../devices/MK24F25612 -I../../../../devices/MK24F25612/drivers -I../../../../devices/MK24F25612/utilities -I../.. -I../../../7139 -I../../../middleware/lwip_1.4.1 -I../../../middleware/lwip_1.4.1/port -I../../../middleware/lwip_1.4.1/port/arch -I../../../middleware/lwip_1.4.1/src -I../../../middleware/lwip_1.4.1/src/include -I../../../middleware/lwip_1.4.1/src/include/ipv4 -I../../../middleware/lwip_1.4.1/src/include/ipv4/lwip -I../../../middleware/lwip_1.4.1/src/include/ipv6 -I../../../middleware/lwip_1.4.1/src/include/ipv6/lwip -I../../../middleware/lwip_1.4.1/src/include/lwip -I../../../middleware/lwip_1.4.1/src/include/netif -I../../../middleware/lwip_1.4.1/src/include/posix -I../../../middleware/lwip_1.4.1/src/netif/ppp -I../../../http_client -I../../../modem_gprs -I../../../modem_flash -I../../../modem_plc -I../../../modem_plc/drv_st7580 -I../../../protocol -I../../../../protocol -I../../../json -I../../../mqtt -I../../../uplink -I../../../../crypto -std=gnu99 -mapcs $(CFLAGS) -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" -c -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

lwip_1.4.1/src/core/init.o: ../../../../project_2g/middleware/lwip_1.4.1/src/core/init.c
	@echo 'Building file: $<'
	@echo 'Invoking: Cross ARM C Compiler'
	arm-none-eabi-gcc -mcpu=cortex-m4 -mthumb -mfloat-abi=hard -mfpu=fpv4-sp-d16 -Os -fmessage-length=0 -fsigned-char -ffunction-sections -fdata-sections -fno-common -ffreestanding -fno-builtin -Wall -Wredundant-decls -DNDEBUG -DUSE_RTOS=1 -DCPU_MK24FN256VDC12 -DFSL_RTOS_FREE_RTOS -DTWR_K24F120M -DTOWER -DAUDIO_PLAY -I../../../../rtos/freertos_8.2.3_k24/Source/portable/GCC/ARM_CM4F -I../../../mqtt/MQTTpacket -I../../../../rtos/freertos_8.2.3_k24/Source/include -I../../../../rtos/freertos_8.2.3_k24/Source -I../../../CMSIS/Include -I../../../../devices -I../../../../devices/MK24F25612 -I../../../../devices/MK24F25612/drivers -I../../../../devices/MK24F25612/utilities -I../.. -I../../../7139 -I../../../middleware/lwip_1.4.1 -I../../../middleware/lwip_1.4.1/port -I../../../middleware/lwip_1.4.1/port/arch -I../../../middleware/lwip_1.4.1/src -I../../../middleware/lwip_1.4.1/src/include -I../../../middleware/lwip_1.4.1/src/include/ipv4 -I../../../middleware/lwip_1.4.1/src/include/ipv4/lwip -I../../../middleware/lwip_1.4.1/src/include/ipv6 -I../../../middleware/lwip_1.4.1/src/include/ipv6/lwip -I../../../middleware/lwip_1.4.1/src/include/lwip -I../../../middleware/lwip_1.4.1/src/include/netif -I../../../middleware/lwip_1.4.1/src/include/posix -I../../../middleware/lwip_1.4.1/src/netif/ppp -I../../../http_client -I../../../modem_gprs -I../../../modem_flash -I../../../modem_plc -I../../../modem_plc/drv_st7580 -I../../../protocol -I../../../../protocol -I../../../json -I../../../mqtt -I../../../uplink -I../../../../crypto -std=gnu99 -mapcs $(CFLAGS) -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" -c -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

lwip_1.4.1/src/core/lwip_timers.o: ../../../../project_2g/middleware/lwip_1.4.1/src/core/lwip_timers.c
	@echo 'Building file: $<'
	@echo 'Invoking: Cross ARM C Compiler'
	arm-none-eabi-gcc -mcpu=cortex-m4 -mthumb -mfloat-abi=hard -mfpu=fpv4-sp-d16 -Os -fmessage-length=0 -fsigned-char -ffunction-sections -fdata-sections -fno-common -ffreestanding -fno-builtin -Wall -Wredundant-decls -DNDEBUG -DUSE_RTOS=1 -DCPU_MK24FN256VDC12 -DFSL_RTOS_FREE_RTOS -DTWR_K24F120M -DTOWER -DAUDIO_PLAY -I../../../../rtos/freertos_8.2.3_k24/Source/portable/GCC/ARM_CM4F -I../../../mqtt/MQTTpacket -I../../../../rtos/freertos_8.2.3_k24/Source/include -I../../../../rtos/freertos_8.2.3_k24/Source -I../../../CMSIS/Include -I../../../../devices -I../../../../devices/MK24F25612 -I../../../../devices/MK24F25612/drivers -I../../../../devices/MK24F25612/utilities -I../.. -I../../../7139 -I../../../middleware/lwip_1.4.1 -I../../../middleware/lwip_1.4.1/port -I../../../middleware/lwip_1.4.1/port/arch -I../../../middleware/lwip_1.4.1/src -I../../../middleware/lwip_1.4.1/src/include -I../../../middleware/lwip_1.4.1/src/include/ipv4 -I../../../middleware/lwip_1.4.1/src/include/ipv4/lwip -I../../../middleware/lwip_1.4.1/src/include/ipv6 -I../../../middleware/lwip_1.4.1/src/include/ipv6/lwip -I../../../middleware/lwip_1.4.1/src/include/lwip -I../../../middleware/lwip_1.4.1/src/include/netif -I../../../middleware/lwip_1.4.1/src/include/posix -I../../../middleware/lwip_1.4.1/src/netif/ppp -I../../../http_client -I../../../modem_gprs -I../../../modem_flash -I../../../modem_plc -I../../../modem_plc/drv_st7580 -I../../../protocol -I../../../../protocol -I../../../json -I../../../mqtt -I../../../uplink -I../../../../crypto -std=gnu99 -mapcs $(CFLAGS) -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" -c -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

lwip_1.4.1/src/core/mem.o: ../../../../project_2g/middleware/lwip_1.4.1/src/core/mem.c
	@echo 'Building file: $<'
	@echo 'Invoking: Cross ARM C Compiler'
	arm-none-eabi-gcc -mcpu=cortex-m4 -mthumb -mfloat-abi=hard -mfpu=fpv4-sp-d16 -Os -fmessage-length=0 -fsigned-char -ffunction-sections -fdata-sections -fno-common -ffreestanding -fno-builtin -Wall -Wredundant-decls -DNDEBUG -DUSE_RTOS=1 -DCPU_MK24FN256VDC12 -DFSL_RTOS_FREE_RTOS -DTWR_K24F120M -DTOWER -DAUDIO_PLAY -I../../../../rtos/freertos_8.2.3_k24/Source/portable/GCC/ARM_CM4F -I../../../mqtt/MQTTpacket -I../../../../rtos/freertos_8.2.3_k24/Source/include -I../../../../rtos/freertos_8.2.3_k24/Source -I../../../CMSIS/Include -I../../../../devices -I../../../../devices/MK24F25612 -I../../../../devices/MK24F25612/drivers -I../../../../devices/MK24F25612/utilities -I../.. -I../../../7139 -I../../../middleware/lwip_1.4.1 -I../../../middleware/lwip_1.4.1/port -I../../../middleware/lwip_1.4.1/port/arch -I../../../middleware/lwip_1.4.1/src -I../../../middleware/lwip_1.4.1/src/include -I../../../middleware/lwip_1.4.1/src/include/ipv4 -I../../../middleware/lwip_1.4.1/src/include/ipv4/lwip -I../../../middleware/lwip_1.4.1/src/include/ipv6 -I../../../middleware/lwip_1.4.1/src/include/ipv6/lwip -I../../../middleware/lwip_1.4.1/src/include/lwip -I../../../middleware/lwip_1.4.1/src/include/netif -I../../../middleware/lwip_1.4.1/src/include/posix -I../../../middleware/lwip_1.4.1/src/netif/ppp -I../../../http_client -I../../../modem_gprs -I../../../modem_flash -I../../../modem_plc -I../../../modem_plc/drv_st7580 -I../../../protocol -I../../../../protocol -I../../../json -I../../../mqtt -I../../../uplink -I../../../../crypto -std=gnu99 -mapcs $(CFLAGS) -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" -c -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

lwip_1.4.1/src/core/memp.o: ../../../../project_2g/middleware/lwip_1.4.1/src/core/memp.c
	@echo 'Building file: $<'
	@echo 'Invoking: Cross ARM C Compiler'
	arm-none-eabi-gcc -mcpu=cortex-m4 -mthumb -mfloat-abi=hard -mfpu=fpv4-sp-d16 -Os -fmessage-length=0 -fsigned-char -ffunction-sections -fdata-sections -fno-common -ffreestanding -fno-builtin -Wall -Wredundant-decls -DNDEBUG -DUSE_RTOS=1 -DCPU_MK24FN256VDC12 -DFSL_RTOS_FREE_RTOS -DTWR_K24F120M -DTOWER -DAUDIO_PLAY -I../../../../rtos/freertos_8.2.3_k24/Source/portable/GCC/ARM_CM4F -I../../../mqtt/MQTTpacket -I../../../../rtos/freertos_8.2.3_k24/Source/include -I../../../../rtos/freertos_8.2.3_k24/Source -I../../../CMSIS/Include -I../../../../devices -I../../../../devices/MK24F25612 -I../../../../devices/MK24F25612/drivers -I../../../../devices/MK24F25612/utilities -I../.. -I../../../7139 -I../../../middleware/lwip_1.4.1 -I../../../middleware/lwip_1.4.1/port -I../../../middleware/lwip_1.4.1/port/arch -I../../../middleware/lwip_1.4.1/src -I../../../middleware/lwip_1.4.1/src/include -I../../../middleware/lwip_1.4.1/src/include/ipv4 -I../../../middleware/lwip_1.4.1/src/include/ipv4/lwip -I../../../middleware/lwip_1.4.1/src/include/ipv6 -I../../../middleware/lwip_1.4.1/src/include/ipv6/lwip -I../../../middleware/lwip_1.4.1/src/include/lwip -I../../../middleware/lwip_1.4.1/src/include/netif -I../../../middleware/lwip_1.4.1/src/include/posix -I../../../middleware/lwip_1.4.1/src/netif/ppp -I../../../http_client -I../../../modem_gprs -I../../../modem_flash -I../../../modem_plc -I../../../modem_plc/drv_st7580 -I../../../protocol -I../../../../protocol -I../../../json -I../../../mqtt -I../../../uplink -I../../../../crypto -std=gnu99 -mapcs $(CFLAGS) -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" -c -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

lwip_1.4.1/src/core/netif.o: ../../../../project_2g/middleware/lwip_1.4.1/src/core/netif.c
	@echo 'Building file: $<'
	@echo 'Invoking: Cross ARM C Compiler'
	arm-none-eabi-gcc -mcpu=cortex-m4 -mthumb -mfloat-abi=hard -mfpu=fpv4-sp-d16 -Os -fmessage-length=0 -fsigned-char -ffunction-sections -fdata-sections -fno-common -ffreestanding -fno-builtin -Wall -Wredundant-decls -DNDEBUG -DUSE_RTOS=1 -DCPU_MK24FN256VDC12 -DFSL_RTOS_FREE_RTOS -DTWR_K24F120M -DTOWER -DAUDIO_PLAY -I../../../../rtos/freertos_8.2.3_k24/Source/portable/GCC/ARM_CM4F -I../../../mqtt/MQTTpacket -I../../../../rtos/freertos_8.2.3_k24/Source/include -I../../../../rtos/freertos_8.2.3_k24/Source -I../../../CMSIS/Include -I../../../../devices -I../../../../devices/MK24F25612 -I../../../../devices/MK24F25612/drivers -I../../../../devices/MK24F25612/utilities -I../.. -I../../../7139 -I../../../middleware/lwip_1.4.1 -I../../../middleware/lwip_1.4.1/port -I../../../middleware/lwip_1.4.1/port/arch -I../../../middleware/lwip_1.4.1/src -I../../../middleware/lwip_1.4.1/src/include -I../../../middleware/lwip_1.4.1/src/include/ipv4 -I../../../middleware/lwip_1.4.1/src/include/ipv4/lwip -I../../../middleware/lwip_1.4.1/src/include/ipv6 -I../../../middleware/lwip_1.4.1/src/include/ipv6/lwip -I../../../middleware/lwip_1.4.1/src/include/lwip -I../../../middleware/lwip_1.4.1/src/include/netif -I../../../middleware/lwip_1.4.1/src/include/posix -I../../../middleware/lwip_1.4.1/src/netif/ppp -I../../../http_client -I../../../modem_gprs -I../../../modem_flash -I../../../modem_plc -I../../../modem_plc/drv_st7580 -I../../../protocol -I../../../../protocol -I../../../json -I../../../mqtt -I../../../uplink -I../../../../crypto -std=gnu99 -mapcs $(CFLAGS) -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" -c -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

lwip_1.4.1/src/core/pbuf.o: ../../../../project_2g/middleware/lwip_1.4.1/src/core/pbuf.c
	@echo 'Building file: $<'
	@echo 'Invoking: Cross ARM C Compiler'
	arm-none-eabi-gcc -mcpu=cortex-m4 -mthumb -mfloat-abi=hard -mfpu=fpv4-sp-d16 -Os -fmessage-length=0 -fsigned-char -ffunction-sections -fdata-sections -fno-common -ffreestanding -fno-builtin -Wall -Wredundant-decls -DNDEBUG -DUSE_RTOS=1 -DCPU_MK24FN256VDC12 -DFSL_RTOS_FREE_RTOS -DTWR_K24F120M -DTOWER -DAUDIO_PLAY -I../../../../rtos/freertos_8.2.3_k24/Source/portable/GCC/ARM_CM4F -I../../../mqtt/MQTTpacket -I../../../../rtos/freertos_8.2.3_k24/Source/include -I../../../../rtos/freertos_8.2.3_k24/Source -I../../../CMSIS/Include -I../../../../devices -I../../../../devices/MK24F25612 -I../../../../devices/MK24F25612/drivers -I../../../../devices/MK24F25612/utilities -I../.. -I../../../7139 -I../../../middleware/lwip_1.4.1 -I../../../middleware/lwip_1.4.1/port -I../../../middleware/lwip_1.4.1/port/arch -I../../../middleware/lwip_1.4.1/src -I../../../middleware/lwip_1.4.1/src/include -I../../../middleware/lwip_1.4.1/src/include/ipv4 -I../../../middleware/lwip_1.4.1/src/include/ipv4/lwip -I../../../middleware/lwip_1.4.1/src/include/ipv6 -I../../../middleware/lwip_1.4.1/src/include/ipv6/lwip -I../../../middleware/lwip_1.4.1/src/include/lwip -I../../../middleware/lwip_1.4.1/src/include/netif -I../../../middleware/lwip_1.4.1/src/include/posix -I../../../middleware/lwip_1.4.1/src/netif/ppp -I../../../http_client -I../../../modem_gprs -I../../../modem_flash -I../../../modem_plc -I../../../modem_plc/drv_st7580 -I../../../protocol -I../../../../protocol -I../../../json -I../../../mqtt -I../../../uplink -I../../../../crypto -std=gnu99 -mapcs $(CFLAGS) -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" -c -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

lwip_1.4.1/src/core/raw.o: ../../../../project_2g/middleware/lwip_1.4.1/src/core/raw.c
	@echo 'Building file: $<'
	@echo 'Invoking: Cross ARM C Compiler'
	arm-none-eabi-gcc -mcpu=cortex-m4 -mthumb -mfloat-abi=hard -mfpu=fpv4-sp-d16 -Os -fmessage-length=0 -fsigned-char -ffunction-sections -fdata-sections -fno-common -ffreestanding -fno-builtin -Wall -Wredundant-decls -DNDEBUG -DUSE_RTOS=1 -DCPU_MK24FN256VDC12 -DFSL_RTOS_FREE_RTOS -DTWR_K24F120M -DTOWER -DAUDIO_PLAY -I../../../../rtos/freertos_8.2.3_k24/Source/portable/GCC/ARM_CM4F -I../../../mqtt/MQTTpacket -I../../../../rtos/freertos_8.2.3_k24/Source/include -I../../../../rtos/freertos_8.2.3_k24/Source -I../../../CMSIS/Include -I../../../../devices -I../../../../devices/MK24F25612 -I../../../../devices/MK24F25612/drivers -I../../../../devices/MK24F25612/utilities -I../.. -I../../../7139 -I../../../middleware/lwip_1.4.1 -I../../../middleware/lwip_1.4.1/port -I../../../middleware/lwip_1.4.1/port/arch -I../../../middleware/lwip_1.4.1/src -I../../../middleware/lwip_1.4.1/src/include -I../../../middleware/lwip_1.4.1/src/include/ipv4 -I../../../middleware/lwip_1.4.1/src/include/ipv4/lwip -I../../../middleware/lwip_1.4.1/src/include/ipv6 -I../../../middleware/lwip_1.4.1/src/include/ipv6/lwip -I../../../middleware/lwip_1.4.1/src/include/lwip -I../../../middleware/lwip_1.4.1/src/include/netif -I../../../middleware/lwip_1.4.1/src/include/posix -I../../../middleware/lwip_1.4.1/src/netif/ppp -I../../../http_client -I../../../modem_gprs -I../../../modem_flash -I../../../modem_plc -I../../../modem_plc/drv_st7580 -I../../../protocol -I../../../../protocol -I../../../json -I../../../mqtt -I../../../uplink -I../../../../crypto -std=gnu99 -mapcs $(CFLAGS) -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" -c -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

lwip_1.4.1/src/core/stats.o: ../../../../project_2g/middleware/lwip_1.4.1/src/core/stats.c
	@echo 'Building file: $<'
	@echo 'Invoking: Cross ARM C Compiler'
	arm-none-eabi-gcc -mcpu=cortex-m4 -mthumb -mfloat-abi=hard -mfpu=fpv4-sp-d16 -Os -fmessage-length=0 -fsigned-char -ffunction-sections -fdata-sections -fno-common -ffreestanding -fno-builtin -Wall -Wredundant-decls -DNDEBUG -DUSE_RTOS=1 -DCPU_MK24FN256VDC12 -DFSL_RTOS_FREE_RTOS -DTWR_K24F120M -DTOWER -DAUDIO_PLAY -I../../../../rtos/freertos_8.2.3_k24/Source/portable/GCC/ARM_CM4F -I../../../mqtt/MQTTpacket -I../../../../rtos/freertos_8.2.3_k24/Source/include -I../../../../rtos/freertos_8.2.3_k24/Source -I../../../CMSIS/Include -I../../../../devices -I../../../../devices/MK24F25612 -I../../../../devices/MK24F25612/drivers -I../../../../devices/MK24F25612/utilities -I../.. -I../../../7139 -I../../../middleware/lwip_1.4.1 -I../../../middleware/lwip_1.4.1/port -I../../../middleware/lwip_1.4.1/port/arch -I../../../middleware/lwip_1.4.1/src -I../../../middleware/lwip_1.4.1/src/include -I../../../middleware/lwip_1.4.1/src/include/ipv4 -I../../../middleware/lwip_1.4.1/src/include/ipv4/lwip -I../../../middleware/lwip_1.4.1/src/include/ipv6 -I../../../middleware/lwip_1.4.1/src/include/ipv6/lwip -I../../../middleware/lwip_1.4.1/src/include/lwip -I../../../middleware/lwip_1.4.1/src/include/netif -I../../../middleware/lwip_1.4.1/src/include/posix -I../../../middleware/lwip_1.4.1/src/netif/ppp -I../../../http_client -I../../../modem_gprs -I../../../modem_flash -I../../../modem_plc -I../../../modem_plc/drv_st7580 -I../../../protocol -I../../../../protocol -I../../../json -I../../../mqtt -I../../../uplink -I../../../../crypto -std=gnu99 -mapcs $(CFLAGS) -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" -c -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

lwip_1.4.1/src/core/sys.o: ../../../../project_2g/middleware/lwip_1.4.1/src/core/sys.c
	@echo 'Building file: $<'
	@echo 'Invoking: Cross ARM C Compiler'
	arm-none-eabi-gcc -mcpu=cortex-m4 -mthumb -mfloat-abi=hard -mfpu=fpv4-sp-d16 -Os -fmessage-length=0 -fsigned-char -ffunction-sections -fdata-sections -fno-common -ffreestanding -fno-builtin -Wall -Wredundant-decls -DNDEBUG -DUSE_RTOS=1 -DCPU_MK24FN256VDC12 -DFSL_RTOS_FREE_RTOS -DTWR_K24F120M -DTOWER -DAUDIO_PLAY -I../../../../rtos/freertos_8.2.3_k24/Source/portable/GCC/ARM_CM4F -I../../../mqtt/MQTTpacket -I../../../../rtos/freertos_8.2.3_k24/Source/include -I../../../../rtos/freertos_8.2.3_k24/Source -I../../../CMSIS/Include -I../../../../devices -I../../../../devices/MK24F25612 -I../../../../devices/MK24F25612/drivers -I../../../../devices/MK24F25612/utilities -I../.. -I../../../7139 -I../../../middleware/lwip_1.4.1 -I../../../middleware/lwip_1.4.1/port -I../../../middleware/lwip_1.4.1/port/arch -I../../../middleware/lwip_1.4.1/src -I../../../middleware/lwip_1.4.1/src/include -I../../../middleware/lwip_1.4.1/src/include/ipv4 -I../../../middleware/lwip_1.4.1/src/include/ipv4/lwip -I../../../middleware/lwip_1.4.1/src/include/ipv6 -I../../../middleware/lwip_1.4.1/src/include/ipv6/lwip -I../../../middleware/lwip_1.4.1/src/include/lwip -I../../../middleware/lwip_1.4.1/src/include/netif -I../../../middleware/lwip_1.4.1/src/include/posix -I../../../middleware/lwip_1.4.1/src/netif/ppp -I../../../http_client -I../../../modem_gprs -I../../../modem_flash -I../../../modem_plc -I../../../modem_plc/drv_st7580 -I../../../protocol -I../../../../protocol -I../../../json -I../../../mqtt -I../../../uplink -I../../../../crypto -std=gnu99 -mapcs $(CFLAGS) -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" -c -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

lwip_1.4.1/src/core/tcp.o: ../../../../project_2g/middleware/lwip_1.4.1/src/core/tcp.c
	@echo 'Building file: $<'
	@echo 'Invoking: Cross ARM C Compiler'
	arm-none-eabi-gcc -mcpu=cortex-m4 -mthumb -mfloat-abi=hard -mfpu=fpv4-sp-d16 -Os -fmessage-length=0 -fsigned-char -ffunction-sections -fdata-sections -fno-common -ffreestanding -fno-builtin -Wall -Wredundant-decls -DNDEBUG -DUSE_RTOS=1 -DCPU_MK24FN256VDC12 -DFSL_RTOS_FREE_RTOS -DTWR_K24F120M -DTOWER -DAUDIO_PLAY -I../../../../rtos/freertos_8.2.3_k24/Source/portable/GCC/ARM_CM4F -I../../../mqtt/MQTTpacket -I../../../../rtos/freertos_8.2.3_k24/Source/include -I../../../../rtos/freertos_8.2.3_k24/Source -I../../../CMSIS/Include -I../../../../devices -I../../../../devices/MK24F25612 -I../../../../devices/MK24F25612/drivers -I../../../../devices/MK24F25612/utilities -I../.. -I../../../7139 -I../../../middleware/lwip_1.4.1 -I../../../middleware/lwip_1.4.1/port -I../../../middleware/lwip_1.4.1/port/arch -I../../../middleware/lwip_1.4.1/src -I../../../middleware/lwip_1.4.1/src/include -I../../../middleware/lwip_1.4.1/src/include/ipv4 -I../../../middleware/lwip_1.4.1/src/include/ipv4/lwip -I../../../middleware/lwip_1.4.1/src/include/ipv6 -I../../../middleware/lwip_1.4.1/src/include/ipv6/lwip -I../../../middleware/lwip_1.4.1/src/include/lwip -I../../../middleware/lwip_1.4.1/src/include/netif -I../../../middleware/lwip_1.4.1/src/include/posix -I../../../middleware/lwip_1.4.1/src/netif/ppp -I../../../http_client -I../../../modem_gprs -I../../../modem_flash -I../../../modem_plc -I../../../modem_plc/drv_st7580 -I../../../protocol -I../../../../protocol -I../../../json -I../../../mqtt -I../../../uplink -I../../../../crypto -std=gnu99 -mapcs $(CFLAGS) -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" -c -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

lwip_1.4.1/src/core/tcp_in.o: ../../../../project_2g/middleware/lwip_1.4.1/src/core/tcp_in.c
	@echo 'Building file: $<'
	@echo 'Invoking: Cross ARM C Compiler'
	arm-none-eabi-gcc -mcpu=cortex-m4 -mthumb -mfloat-abi=hard -mfpu=fpv4-sp-d16 -Os -fmessage-length=0 -fsigned-char -ffunction-sections -fdata-sections -fno-common -ffreestanding -fno-builtin -Wall -Wredundant-decls -DNDEBUG -DUSE_RTOS=1 -DCPU_MK24FN256VDC12 -DFSL_RTOS_FREE_RTOS -DTWR_K24F120M -DTOWER -DAUDIO_PLAY -I../../../../rtos/freertos_8.2.3_k24/Source/portable/GCC/ARM_CM4F -I../../../mqtt/MQTTpacket -I../../../../rtos/freertos_8.2.3_k24/Source/include -I../../../../rtos/freertos_8.2.3_k24/Source -I../../../CMSIS/Include -I../../../../devices -I../../../../devices/MK24F25612 -I../../../../devices/MK24F25612/drivers -I../../../../devices/MK24F25612/utilities -I../.. -I../../../7139 -I../../../middleware/lwip_1.4.1 -I../../../middleware/lwip_1.4.1/port -I../../../middleware/lwip_1.4.1/port/arch -I../../../middleware/lwip_1.4.1/src -I../../../middleware/lwip_1.4.1/src/include -I../../../middleware/lwip_1.4.1/src/include/ipv4 -I../../../middleware/lwip_1.4.1/src/include/ipv4/lwip -I../../../middleware/lwip_1.4.1/src/include/ipv6 -I../../../middleware/lwip_1.4.1/src/include/ipv6/lwip -I../../../middleware/lwip_1.4.1/src/include/lwip -I../../../middleware/lwip_1.4.1/src/include/netif -I../../../middleware/lwip_1.4.1/src/include/posix -I../../../middleware/lwip_1.4.1/src/netif/ppp -I../../../http_client -I../../../modem_gprs -I../../../modem_flash -I../../../modem_plc -I../../../modem_plc/drv_st7580 -I../../../protocol -I../../../../protocol -I../../../json -I../../../mqtt -I../../../uplink -I../../../../crypto -std=gnu99 -mapcs $(CFLAGS) -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" -c -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

lwip_1.4.1/src/core/tcp_out.o: ../../../../project_2g/middleware/lwip_1.4.1/src/core/tcp_out.c
	@echo 'Building file: $<'
	@echo 'Invoking: Cross ARM C Compiler'
	arm-none-eabi-gcc -mcpu=cortex-m4 -mthumb -mfloat-abi=hard -mfpu=fpv4-sp-d16 -Os -fmessage-length=0 -fsigned-char -ffunction-sections -fdata-sections -fno-common -ffreestanding -fno-builtin -Wall -Wredundant-decls -DNDEBUG -DUSE_RTOS=1 -DCPU_MK24FN256VDC12 -DFSL_RTOS_FREE_RTOS -DTWR_K24F120M -DTOWER -DAUDIO_PLAY -I../../../../rtos/freertos_8.2.3_k24/Source/portable/GCC/ARM_CM4F -I../../../mqtt/MQTTpacket -I../../../../rtos/freertos_8.2.3_k24/Source/include -I../../../../rtos/freertos_8.2.3_k24/Source -I../../../CMSIS/Include -I../../../../devices -I../../../../devices/MK24F25612 -I../../../../devices/MK24F25612/drivers -I../../../../devices/MK24F25612/utilities -I../.. -I../../../7139 -I../../../middleware/lwip_1.4.1 -I../../../middleware/lwip_1.4.1/port -I../../../middleware/lwip_1.4.1/port/arch -I../../../middleware/lwip_1.4.1/src -I../../../middleware/lwip_1.4.1/src/include -I../../../middleware/lwip_1.4.1/src/include/ipv4 -I../../../middleware/lwip_1.4.1/src/include/ipv4/lwip -I../../../middleware/lwip_1.4.1/src/include/ipv6 -I../../../middleware/lwip_1.4.1/src/include/ipv6/lwip -I../../../middleware/lwip_1.4.1/src/include/lwip -I../../../middleware/lwip_1.4.1/src/include/netif -I../../../middleware/lwip_1.4.1/src/include/posix -I../../../middleware/lwip_1.4.1/src/netif/ppp -I../../../http_client -I../../../modem_gprs -I../../../modem_flash -I../../../modem_plc -I../../../modem_plc/drv_st7580 -I../../../protocol -I../../../../protocol -I../../../json -I../../../mqtt -I../../../uplink -I../../../../crypto -std=gnu99 -mapcs $(CFLAGS) -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" -c -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

lwip_1.4.1/src/core/udp.o: ../../../../project_2g/middleware/lwip_1.4.1/src/core/udp.c
	@echo 'Building file: $<'
	@echo 'Invoking: Cross ARM C Compiler'
	arm-none-eabi-gcc -mcpu=cortex-m4 -mthumb -mfloat-abi=hard -mfpu=fpv4-sp-d16 -Os -fmessage-length=0 -fsigned-char -ffunction-sections -fdata-sections -fno-common -ffreestanding -fno-builtin -Wall -Wredundant-decls -DNDEBUG -DUSE_RTOS=1 -DCPU_MK24FN256VDC12 -DFSL_RTOS_FREE_RTOS -DTWR_K24F120M -DTOWER -DAUDIO_PLAY -I../../../../rtos/freertos_8.2.3_k24/Source/portable/GCC/ARM_CM4F -I../../../mqtt/MQTTpacket -I../../../../rtos/freertos_8.2.3_k24/Source/include -I../../../../rtos/freertos_8.2.3_k24/Source -I../../../CMSIS/Include -I../../../../devices -I../../../../devices/MK24F25612 -I../../../../devices/MK24F25612/drivers -I../../../../devices/MK24F25612/utilities -I../.. -I../../../7139 -I../../../middleware/lwip_1.4.1 -I../../../middleware/lwip_1.4.1/port -I../../../middleware/lwip_1.4.1/port/arch -I../../../middleware/lwip_1.4.1/src -I../../../middleware/lwip_1.4.1/src/include -I../../../middleware/lwip_1.4.1/src/include/ipv4 -I../../../middleware/lwip_1.4.1/src/include/ipv4/lwip -I../../../middleware/lwip_1.4.1/src/include/ipv6 -I../../../middleware/lwip_1.4.1/src/include/ipv6/lwip -I../../../middleware/lwip_1.4.1/src/include/lwip -I../../../middleware/lwip_1.4.1/src/include/netif -I../../../middleware/lwip_1.4.1/src/include/posix -I../../../middleware/lwip_1.4.1/src/netif/ppp -I../../../http_client -I../../../modem_gprs -I../../../modem_flash -I../../../modem_plc -I../../../modem_plc/drv_st7580 -I../../../protocol -I../../../../protocol -I../../../json -I../../../mqtt -I../../../uplink -I../../../../crypto -std=gnu99 -mapcs $(CFLAGS) -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" -c -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


