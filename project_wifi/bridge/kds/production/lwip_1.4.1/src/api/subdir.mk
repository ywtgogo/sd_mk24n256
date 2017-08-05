################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../../../middleware/lwip_1.4.1/src/api/api_lib.c \
../../../middleware/lwip_1.4.1/src/api/api_msg.c \
../../../middleware/lwip_1.4.1/src/api/err.c \
../../../middleware/lwip_1.4.1/src/api/netbuf.c \
../../../middleware/lwip_1.4.1/src/api/netdb.c \
../../../middleware/lwip_1.4.1/src/api/netifapi.c \
../../../middleware/lwip_1.4.1/src/api/sockets.c \
../../../middleware/lwip_1.4.1/src/api/tcpip.c 

OBJS += \
./lwip_1.4.1/src/api/api_lib.o \
./lwip_1.4.1/src/api/api_msg.o \
./lwip_1.4.1/src/api/err.o \
./lwip_1.4.1/src/api/netbuf.o \
./lwip_1.4.1/src/api/netdb.o \
./lwip_1.4.1/src/api/netifapi.o \
./lwip_1.4.1/src/api/sockets.o \
./lwip_1.4.1/src/api/tcpip.o 

C_DEPS += \
./lwip_1.4.1/src/api/api_lib.d \
./lwip_1.4.1/src/api/api_msg.d \
./lwip_1.4.1/src/api/err.d \
./lwip_1.4.1/src/api/netbuf.d \
./lwip_1.4.1/src/api/netdb.d \
./lwip_1.4.1/src/api/netifapi.d \
./lwip_1.4.1/src/api/sockets.d \
./lwip_1.4.1/src/api/tcpip.d 


# Each subdirectory must supply rules for building sources it contributes
lwip_1.4.1/src/api/api_lib.o: ../../../middleware/lwip_1.4.1/src/api/api_lib.c
	@echo 'Building file: $<'
	@echo 'Invoking: Cross ARM C Compiler'
	arm-none-eabi-gcc -mcpu=cortex-m4 -mthumb -mfloat-abi=hard -mfpu=fpv4-sp-d16 -Os -fmessage-length=0 -fsigned-char -ffunction-sections -fdata-sections -fno-common -ffreestanding -fno-builtin -Wall -DNDEBUG -DPLC -DBRIDGE -DUSE_RTOS=1 -DCPU_MK24FN256VDC12 -DFSL_RTOS_FREE_RTOS -DTWR_K24F120M -DTOWER -I../../.. -I../../../homeboxk24 -I../../../rtos/freertos_8.2.3_k24/Source/portable/GCC/ARM_CM4F -I../../../rtos/freertos_8.2.3_k24/Source/include -I../../../rtos/freertos_8.2.3_k24/Source -I../../../CMSIS/Include -I../../../devices -I../../../devices/MK24F25612 -I../../../devices/MK24F25612/drivers -I../../../devices/MK24F25612/utilities -I../../../7139 -I../../../mqtt -I../../../mqtt/MQTTpacket -I../../../json -I../../../uplink -I../../../middleware/lwip_1.4.1 -I../../../middleware/lwip_1.4.1/port -I../../../middleware/lwip_1.4.1/port/arch -I../../../middleware/lwip_1.4.1/src -I../../../middleware/lwip_1.4.1/src/include -I../../../middleware/lwip_1.4.1/src/include/ipv4 -I../../../middleware/lwip_1.4.1/src/include/ipv4/lwip -I../../../middleware/lwip_1.4.1/src/include/ipv6 -I../../../middleware/lwip_1.4.1/src/include/ipv6/lwip -I../../../middleware/lwip_1.4.1/src/include/lwip -I../../../middleware/lwip_1.4.1/src/include/netif -I../../../middleware/lwip_1.4.1/src/include/posix -I../../../middleware/lwip_1.4.1/src/netif/ppp -I../../../http_client -I../../../modem_gprs -I../../../modem_flash -I../../../protocol -I../../../modem_plc -I../../../modem_plc/drv_st7580 -I../../../crypto -std=gnu99 -mapcs -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" -c -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

lwip_1.4.1/src/api/api_msg.o: ../../../middleware/lwip_1.4.1/src/api/api_msg.c
	@echo 'Building file: $<'
	@echo 'Invoking: Cross ARM C Compiler'
	arm-none-eabi-gcc -mcpu=cortex-m4 -mthumb -mfloat-abi=hard -mfpu=fpv4-sp-d16 -Os -fmessage-length=0 -fsigned-char -ffunction-sections -fdata-sections -fno-common -ffreestanding -fno-builtin -Wall -DNDEBUG -DPLC -DBRIDGE -DUSE_RTOS=1 -DCPU_MK24FN256VDC12 -DFSL_RTOS_FREE_RTOS -DTWR_K24F120M -DTOWER -I../../.. -I../../../homeboxk24 -I../../../rtos/freertos_8.2.3_k24/Source/portable/GCC/ARM_CM4F -I../../../rtos/freertos_8.2.3_k24/Source/include -I../../../rtos/freertos_8.2.3_k24/Source -I../../../CMSIS/Include -I../../../devices -I../../../devices/MK24F25612 -I../../../devices/MK24F25612/drivers -I../../../devices/MK24F25612/utilities -I../../../7139 -I../../../mqtt -I../../../mqtt/MQTTpacket -I../../../json -I../../../uplink -I../../../middleware/lwip_1.4.1 -I../../../middleware/lwip_1.4.1/port -I../../../middleware/lwip_1.4.1/port/arch -I../../../middleware/lwip_1.4.1/src -I../../../middleware/lwip_1.4.1/src/include -I../../../middleware/lwip_1.4.1/src/include/ipv4 -I../../../middleware/lwip_1.4.1/src/include/ipv4/lwip -I../../../middleware/lwip_1.4.1/src/include/ipv6 -I../../../middleware/lwip_1.4.1/src/include/ipv6/lwip -I../../../middleware/lwip_1.4.1/src/include/lwip -I../../../middleware/lwip_1.4.1/src/include/netif -I../../../middleware/lwip_1.4.1/src/include/posix -I../../../middleware/lwip_1.4.1/src/netif/ppp -I../../../http_client -I../../../modem_gprs -I../../../modem_flash -I../../../protocol -I../../../modem_plc -I../../../modem_plc/drv_st7580 -I../../../crypto -std=gnu99 -mapcs -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" -c -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

lwip_1.4.1/src/api/err.o: ../../../middleware/lwip_1.4.1/src/api/err.c
	@echo 'Building file: $<'
	@echo 'Invoking: Cross ARM C Compiler'
	arm-none-eabi-gcc -mcpu=cortex-m4 -mthumb -mfloat-abi=hard -mfpu=fpv4-sp-d16 -Os -fmessage-length=0 -fsigned-char -ffunction-sections -fdata-sections -fno-common -ffreestanding -fno-builtin -Wall -DNDEBUG -DPLC -DBRIDGE -DUSE_RTOS=1 -DCPU_MK24FN256VDC12 -DFSL_RTOS_FREE_RTOS -DTWR_K24F120M -DTOWER -I../../.. -I../../../homeboxk24 -I../../../rtos/freertos_8.2.3_k24/Source/portable/GCC/ARM_CM4F -I../../../rtos/freertos_8.2.3_k24/Source/include -I../../../rtos/freertos_8.2.3_k24/Source -I../../../CMSIS/Include -I../../../devices -I../../../devices/MK24F25612 -I../../../devices/MK24F25612/drivers -I../../../devices/MK24F25612/utilities -I../../../7139 -I../../../mqtt -I../../../mqtt/MQTTpacket -I../../../json -I../../../uplink -I../../../middleware/lwip_1.4.1 -I../../../middleware/lwip_1.4.1/port -I../../../middleware/lwip_1.4.1/port/arch -I../../../middleware/lwip_1.4.1/src -I../../../middleware/lwip_1.4.1/src/include -I../../../middleware/lwip_1.4.1/src/include/ipv4 -I../../../middleware/lwip_1.4.1/src/include/ipv4/lwip -I../../../middleware/lwip_1.4.1/src/include/ipv6 -I../../../middleware/lwip_1.4.1/src/include/ipv6/lwip -I../../../middleware/lwip_1.4.1/src/include/lwip -I../../../middleware/lwip_1.4.1/src/include/netif -I../../../middleware/lwip_1.4.1/src/include/posix -I../../../middleware/lwip_1.4.1/src/netif/ppp -I../../../http_client -I../../../modem_gprs -I../../../modem_flash -I../../../protocol -I../../../modem_plc -I../../../modem_plc/drv_st7580 -I../../../crypto -std=gnu99 -mapcs -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" -c -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

lwip_1.4.1/src/api/netbuf.o: ../../../middleware/lwip_1.4.1/src/api/netbuf.c
	@echo 'Building file: $<'
	@echo 'Invoking: Cross ARM C Compiler'
	arm-none-eabi-gcc -mcpu=cortex-m4 -mthumb -mfloat-abi=hard -mfpu=fpv4-sp-d16 -Os -fmessage-length=0 -fsigned-char -ffunction-sections -fdata-sections -fno-common -ffreestanding -fno-builtin -Wall -DNDEBUG -DPLC -DBRIDGE -DUSE_RTOS=1 -DCPU_MK24FN256VDC12 -DFSL_RTOS_FREE_RTOS -DTWR_K24F120M -DTOWER -I../../.. -I../../../homeboxk24 -I../../../rtos/freertos_8.2.3_k24/Source/portable/GCC/ARM_CM4F -I../../../rtos/freertos_8.2.3_k24/Source/include -I../../../rtos/freertos_8.2.3_k24/Source -I../../../CMSIS/Include -I../../../devices -I../../../devices/MK24F25612 -I../../../devices/MK24F25612/drivers -I../../../devices/MK24F25612/utilities -I../../../7139 -I../../../mqtt -I../../../mqtt/MQTTpacket -I../../../json -I../../../uplink -I../../../middleware/lwip_1.4.1 -I../../../middleware/lwip_1.4.1/port -I../../../middleware/lwip_1.4.1/port/arch -I../../../middleware/lwip_1.4.1/src -I../../../middleware/lwip_1.4.1/src/include -I../../../middleware/lwip_1.4.1/src/include/ipv4 -I../../../middleware/lwip_1.4.1/src/include/ipv4/lwip -I../../../middleware/lwip_1.4.1/src/include/ipv6 -I../../../middleware/lwip_1.4.1/src/include/ipv6/lwip -I../../../middleware/lwip_1.4.1/src/include/lwip -I../../../middleware/lwip_1.4.1/src/include/netif -I../../../middleware/lwip_1.4.1/src/include/posix -I../../../middleware/lwip_1.4.1/src/netif/ppp -I../../../http_client -I../../../modem_gprs -I../../../modem_flash -I../../../protocol -I../../../modem_plc -I../../../modem_plc/drv_st7580 -I../../../crypto -std=gnu99 -mapcs -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" -c -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

lwip_1.4.1/src/api/netdb.o: ../../../middleware/lwip_1.4.1/src/api/netdb.c
	@echo 'Building file: $<'
	@echo 'Invoking: Cross ARM C Compiler'
	arm-none-eabi-gcc -mcpu=cortex-m4 -mthumb -mfloat-abi=hard -mfpu=fpv4-sp-d16 -Os -fmessage-length=0 -fsigned-char -ffunction-sections -fdata-sections -fno-common -ffreestanding -fno-builtin -Wall -DNDEBUG -DPLC -DBRIDGE -DUSE_RTOS=1 -DCPU_MK24FN256VDC12 -DFSL_RTOS_FREE_RTOS -DTWR_K24F120M -DTOWER -I../../.. -I../../../homeboxk24 -I../../../rtos/freertos_8.2.3_k24/Source/portable/GCC/ARM_CM4F -I../../../rtos/freertos_8.2.3_k24/Source/include -I../../../rtos/freertos_8.2.3_k24/Source -I../../../CMSIS/Include -I../../../devices -I../../../devices/MK24F25612 -I../../../devices/MK24F25612/drivers -I../../../devices/MK24F25612/utilities -I../../../7139 -I../../../mqtt -I../../../mqtt/MQTTpacket -I../../../json -I../../../uplink -I../../../middleware/lwip_1.4.1 -I../../../middleware/lwip_1.4.1/port -I../../../middleware/lwip_1.4.1/port/arch -I../../../middleware/lwip_1.4.1/src -I../../../middleware/lwip_1.4.1/src/include -I../../../middleware/lwip_1.4.1/src/include/ipv4 -I../../../middleware/lwip_1.4.1/src/include/ipv4/lwip -I../../../middleware/lwip_1.4.1/src/include/ipv6 -I../../../middleware/lwip_1.4.1/src/include/ipv6/lwip -I../../../middleware/lwip_1.4.1/src/include/lwip -I../../../middleware/lwip_1.4.1/src/include/netif -I../../../middleware/lwip_1.4.1/src/include/posix -I../../../middleware/lwip_1.4.1/src/netif/ppp -I../../../http_client -I../../../modem_gprs -I../../../modem_flash -I../../../protocol -I../../../modem_plc -I../../../modem_plc/drv_st7580 -I../../../crypto -std=gnu99 -mapcs -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" -c -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

lwip_1.4.1/src/api/netifapi.o: ../../../middleware/lwip_1.4.1/src/api/netifapi.c
	@echo 'Building file: $<'
	@echo 'Invoking: Cross ARM C Compiler'
	arm-none-eabi-gcc -mcpu=cortex-m4 -mthumb -mfloat-abi=hard -mfpu=fpv4-sp-d16 -Os -fmessage-length=0 -fsigned-char -ffunction-sections -fdata-sections -fno-common -ffreestanding -fno-builtin -Wall -DNDEBUG -DPLC -DBRIDGE -DUSE_RTOS=1 -DCPU_MK24FN256VDC12 -DFSL_RTOS_FREE_RTOS -DTWR_K24F120M -DTOWER -I../../.. -I../../../homeboxk24 -I../../../rtos/freertos_8.2.3_k24/Source/portable/GCC/ARM_CM4F -I../../../rtos/freertos_8.2.3_k24/Source/include -I../../../rtos/freertos_8.2.3_k24/Source -I../../../CMSIS/Include -I../../../devices -I../../../devices/MK24F25612 -I../../../devices/MK24F25612/drivers -I../../../devices/MK24F25612/utilities -I../../../7139 -I../../../mqtt -I../../../mqtt/MQTTpacket -I../../../json -I../../../uplink -I../../../middleware/lwip_1.4.1 -I../../../middleware/lwip_1.4.1/port -I../../../middleware/lwip_1.4.1/port/arch -I../../../middleware/lwip_1.4.1/src -I../../../middleware/lwip_1.4.1/src/include -I../../../middleware/lwip_1.4.1/src/include/ipv4 -I../../../middleware/lwip_1.4.1/src/include/ipv4/lwip -I../../../middleware/lwip_1.4.1/src/include/ipv6 -I../../../middleware/lwip_1.4.1/src/include/ipv6/lwip -I../../../middleware/lwip_1.4.1/src/include/lwip -I../../../middleware/lwip_1.4.1/src/include/netif -I../../../middleware/lwip_1.4.1/src/include/posix -I../../../middleware/lwip_1.4.1/src/netif/ppp -I../../../http_client -I../../../modem_gprs -I../../../modem_flash -I../../../protocol -I../../../modem_plc -I../../../modem_plc/drv_st7580 -I../../../crypto -std=gnu99 -mapcs -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" -c -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

lwip_1.4.1/src/api/sockets.o: ../../../middleware/lwip_1.4.1/src/api/sockets.c
	@echo 'Building file: $<'
	@echo 'Invoking: Cross ARM C Compiler'
	arm-none-eabi-gcc -mcpu=cortex-m4 -mthumb -mfloat-abi=hard -mfpu=fpv4-sp-d16 -Os -fmessage-length=0 -fsigned-char -ffunction-sections -fdata-sections -fno-common -ffreestanding -fno-builtin -Wall -DNDEBUG -DPLC -DBRIDGE -DUSE_RTOS=1 -DCPU_MK24FN256VDC12 -DFSL_RTOS_FREE_RTOS -DTWR_K24F120M -DTOWER -I../../.. -I../../../homeboxk24 -I../../../rtos/freertos_8.2.3_k24/Source/portable/GCC/ARM_CM4F -I../../../rtos/freertos_8.2.3_k24/Source/include -I../../../rtos/freertos_8.2.3_k24/Source -I../../../CMSIS/Include -I../../../devices -I../../../devices/MK24F25612 -I../../../devices/MK24F25612/drivers -I../../../devices/MK24F25612/utilities -I../../../7139 -I../../../mqtt -I../../../mqtt/MQTTpacket -I../../../json -I../../../uplink -I../../../middleware/lwip_1.4.1 -I../../../middleware/lwip_1.4.1/port -I../../../middleware/lwip_1.4.1/port/arch -I../../../middleware/lwip_1.4.1/src -I../../../middleware/lwip_1.4.1/src/include -I../../../middleware/lwip_1.4.1/src/include/ipv4 -I../../../middleware/lwip_1.4.1/src/include/ipv4/lwip -I../../../middleware/lwip_1.4.1/src/include/ipv6 -I../../../middleware/lwip_1.4.1/src/include/ipv6/lwip -I../../../middleware/lwip_1.4.1/src/include/lwip -I../../../middleware/lwip_1.4.1/src/include/netif -I../../../middleware/lwip_1.4.1/src/include/posix -I../../../middleware/lwip_1.4.1/src/netif/ppp -I../../../http_client -I../../../modem_gprs -I../../../modem_flash -I../../../protocol -I../../../modem_plc -I../../../modem_plc/drv_st7580 -I../../../crypto -std=gnu99 -mapcs -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" -c -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

lwip_1.4.1/src/api/tcpip.o: ../../../middleware/lwip_1.4.1/src/api/tcpip.c
	@echo 'Building file: $<'
	@echo 'Invoking: Cross ARM C Compiler'
	arm-none-eabi-gcc -mcpu=cortex-m4 -mthumb -mfloat-abi=hard -mfpu=fpv4-sp-d16 -Os -fmessage-length=0 -fsigned-char -ffunction-sections -fdata-sections -fno-common -ffreestanding -fno-builtin -Wall -DNDEBUG -DPLC -DBRIDGE -DUSE_RTOS=1 -DCPU_MK24FN256VDC12 -DFSL_RTOS_FREE_RTOS -DTWR_K24F120M -DTOWER -I../../.. -I../../../homeboxk24 -I../../../rtos/freertos_8.2.3_k24/Source/portable/GCC/ARM_CM4F -I../../../rtos/freertos_8.2.3_k24/Source/include -I../../../rtos/freertos_8.2.3_k24/Source -I../../../CMSIS/Include -I../../../devices -I../../../devices/MK24F25612 -I../../../devices/MK24F25612/drivers -I../../../devices/MK24F25612/utilities -I../../../7139 -I../../../mqtt -I../../../mqtt/MQTTpacket -I../../../json -I../../../uplink -I../../../middleware/lwip_1.4.1 -I../../../middleware/lwip_1.4.1/port -I../../../middleware/lwip_1.4.1/port/arch -I../../../middleware/lwip_1.4.1/src -I../../../middleware/lwip_1.4.1/src/include -I../../../middleware/lwip_1.4.1/src/include/ipv4 -I../../../middleware/lwip_1.4.1/src/include/ipv4/lwip -I../../../middleware/lwip_1.4.1/src/include/ipv6 -I../../../middleware/lwip_1.4.1/src/include/ipv6/lwip -I../../../middleware/lwip_1.4.1/src/include/lwip -I../../../middleware/lwip_1.4.1/src/include/netif -I../../../middleware/lwip_1.4.1/src/include/posix -I../../../middleware/lwip_1.4.1/src/netif/ppp -I../../../http_client -I../../../modem_gprs -I../../../modem_flash -I../../../protocol -I../../../modem_plc -I../../../modem_plc/drv_st7580 -I../../../crypto -std=gnu99 -mapcs -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" -c -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


