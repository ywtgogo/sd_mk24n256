/*
 * Copyright (c) 2015, Freescale Semiconductor, Inc.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *
 * o Redistributions of source code must retain the above copyright notice, this list
 *   of conditions and the following disclaimer.
 *
 * o Redistributions in binary form must reproduce the above copyright notice, this
 *   list of conditions and the following disclaimer in the documentation and/or
 *   other materials provided with the distribution.
 *
 * o Neither the name of Freescale Semiconductor, Inc. nor the names of its
 *   contributors may be used to endorse or promote products derived from this
 *   software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
 * ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "fsl_device_registers.h"
#include "fsl_port.h"
#include "pin_mux.h"
#include "fsl_common.h"
#include "board.h"

/*******************************************************************************
 * Code
 ******************************************************************************/
/*!
 * @brief Initialize all pins used in this example
 *
 * @param disablePortClockAfterInit disable port clock after pin
 * initialization or not.
 */
void BOARD_InitPins(void)
{
    CLOCK_EnableClock(kCLOCK_PortA);
    CLOCK_EnableClock(kCLOCK_PortC);
    CLOCK_EnableClock(kCLOCK_PortD);
    CLOCK_EnableClock(kCLOCK_PortE);
    CLOCK_EnableClock(kCLOCK_PortB);
    
#if (BOARD_NAME == HW_V01)
    /* Initialize UART0 pins below */
    PORT_SetPinMux(PORTA, 14U, kPORT_MuxAlt3);
    PORT_SetPinMux(PORTA, 15U, kPORT_MuxAlt3);

    /* Initialize UART1 pins below */
    PORT_SetPinMux(PORTC, 4u, kPORT_MuxAlt3);
    PORT_SetPinMux(PORTC, 3u, kPORT_MuxAlt3);

    /*Initialize UART2 pins below*/
    PORT_SetPinMux(PORTD, 2u, kPORT_MuxAlt3);
    PORT_SetPinMux(PORTD, 3u, kPORT_MuxAlt3);

    /*Initialize UART3 pins below*/
    PORT_SetPinMux(PORTB, 10u, kPORT_MuxAlt3);
    PORT_SetPinMux(PORTB, 11u, kPORT_MuxAlt3);

    /* spi0 for rf */
    PORT_SetPinMux(PORTD,  0u, kPORT_MuxAlt2);
    PORT_SetPinMux(PORTC,  5u, kPORT_MuxAlt2);
    PORT_SetPinMux(PORTC,  6u, kPORT_MuxAlt2);
    PORT_SetPinMux(PORTC,  7u, kPORT_MuxAlt2);
    /* for rf GIO2*/
    port_pin_config_t config_gio2 = {0};
    config_gio2.pullSelect = kPORT_PullDown;
    config_gio2.mux = kPORT_MuxAsGpio;
    PORT_SetPinConfig(PORTC, 19u, &config_gio2);

    /* spi1 for spi_flash */
    PORT_SetPinMux(PORTD, 4u, kPORT_MuxAlt7);
    PORT_SetPinMux(PORTD, 5u, kPORT_MuxAlt7);
    PORT_SetPinMux(PORTD, 6u, kPORT_MuxAlt7);
    PORT_SetPinMux(PORTD, 7u, kPORT_MuxAlt7);

    /* for SW */
    port_pin_config_t config_sw = {0};
    config_sw.pullSelect = kPORT_PullUp;
    config_sw.mux = kPORT_MuxAsGpio;
    PORT_SetPinConfig(PORTE, 2u, &config_sw);

    /* Led pin mux Configuration */
    PORT_SetPinMux(PORTD, 14U, kPORT_MuxAsGpio);
    PORT_SetPinMux(PORTD, 15U, kPORT_MuxAsGpio);

    /* GSM_ON_OFF */
    PORT_SetPinMux(PORTD,  8U, kPORT_MuxAsGpio);

    /* speaker switch */
    PORT_SetPinMux(PORTB,  8U, kPORT_MuxAsGpio);
    /* for 2G DCD */
    port_pin_config_t config_dcd = {0};
    config_dcd.pullSelect = kPORT_PullUp;
    config_dcd.mux = kPORT_MuxAsGpio;
    PORT_SetPinConfig(PORTB, 12u, &config_dcd);
    /* 2G DCD */
    //PORT_SetPinMux(PORTB,  12U, kPORT_MuxAsGpio);

    /* for 2G ring */
    port_pin_config_t config_ring = {0};
    config_ring.pullSelect = kPORT_PullUp;
    config_ring.mux = kPORT_MuxAsGpio;
    PORT_SetPinConfig(PORTB, 13u, &config_ring);
    /* 2G ring */
    //PORT_SetPinMux(PORTB,  13U, kPORT_MuxAsGpio);
    /* V-detect ADC */
    PORT_SetPinMux(PORTB,  0U, kPORT_PinDisabledOrAnalog);

    /* HIGHV 258V input */
    PORT_SetPinMux(PORTA,  18U, kPORT_MuxAsGpio);

    /* NORV 195V LPTMR counter */
    PORT_SetPinMux(PORTA,  19U, kPORT_MuxAlt6);

#elif  (BOARD_NAME >= HW_V02)
    /* Initialize UART0 pins below -> KM*/
    PORT_SetPinMux(PORTA, 14u, kPORT_MuxAlt3);
    PORT_SetPinMux(PORTA, 15u, kPORT_MuxAlt3);

    /* Initialize UART1 pins below -> WIFI*/
    PORT_SetPinMux(PORTC, 3U, kPORT_MuxAlt3);
    PORT_SetPinMux(PORTC, 4U, kPORT_MuxAlt3);

    /* Initialize UART2 pins below -> SILVER*/
    PORT_SetPinMux(PORTD, 2U, kPORT_MuxAlt3);
    PORT_SetPinMux(PORTD, 3U, kPORT_MuxAlt3);

    /* Initialize UART3 pins below -> PLC*/
    PORT_SetPinMux(PORTB, 10U, kPORT_MuxAlt3);
    PORT_SetPinMux(PORTB, 11U, kPORT_MuxAlt3);

    /* Initialize UART4 pins below -> ZIGBEE*/
    PORT_SetPinMux(PORTE, 24U, kPORT_MuxAlt3);
    PORT_SetPinMux(PORTE, 25U, kPORT_MuxAlt3);

    /* Initialize UART5 pins below -> DEBUG*/
    PORT_SetPinMux(PORTD,  8U, kPORT_MuxAlt3);
    PORT_SetPinMux(PORTD,  9U, kPORT_MuxAlt3);

    /* spi0 for rf */
    PORT_SetPinMux(PORTD,  0u, kPORT_MuxAlt2);
    PORT_SetPinMux(PORTC,  5u, kPORT_MuxAlt2);
    PORT_SetPinMux(PORTC,  6u, kPORT_MuxAlt2);
    PORT_SetPinMux(PORTC,  7u, kPORT_MuxAlt2);
    /* for rf GIO2*/
    port_pin_config_t config_gio2 = {0};
    config_gio2.pullSelect = kPORT_PullDown;
    config_gio2.mux = kPORT_MuxAsGpio;
    PORT_SetPinConfig(PORTC, 19u, &config_gio2);

    /* spi1 for spi_flash */
    PORT_SetPinConfig(PORTD, 4U, &(port_pin_config_t){
        kPORT_PullUp,
        kPORT_SlowSlewRate,
        kPORT_PassiveFilterDisable,
        kPORT_OpenDrainDisable,
        kPORT_LowDriveStrength,
        kPORT_MuxAlt7,
        kPORT_UnlockRegister,
    });
    PORT_SetPinConfig(PORTD, 5U, &(port_pin_config_t){
        kPORT_PullUp,
        kPORT_SlowSlewRate,
        kPORT_PassiveFilterDisable,
        kPORT_OpenDrainDisable,
        kPORT_LowDriveStrength,
        kPORT_MuxAlt7,
        kPORT_UnlockRegister,
    });
    PORT_SetPinConfig(PORTD, 6U, &(port_pin_config_t){
        kPORT_PullUp,
        kPORT_SlowSlewRate,
        kPORT_PassiveFilterDisable,
        kPORT_OpenDrainDisable,
        kPORT_LowDriveStrength,
        kPORT_MuxAlt7,
        kPORT_UnlockRegister,
    });
    PORT_SetPinConfig(PORTD, 7U, &(port_pin_config_t){
        kPORT_PullUp,
        kPORT_SlowSlewRate,
        kPORT_PassiveFilterDisable,
        kPORT_OpenDrainDisable,
        kPORT_LowDriveStrength,
        kPORT_MuxAlt7,
        kPORT_UnlockRegister,
    });
//    PORT_SetPinMux(PORTD, 4u, kPORT_MuxAlt7);
//    PORT_SetPinMux(PORTD, 5u, kPORT_MuxAlt7);
//    PORT_SetPinMux(PORTD, 6u, kPORT_MuxAlt7);
//    PORT_SetPinMux(PORTD, 7u, kPORT_MuxAlt7);

    /* Led pin mux Configuration, low-on*/
    PORT_SetPinMux(PORTA,  1U, kPORT_MuxAsGpio);
    PORT_SetPinMux(PORTA,  2U, kPORT_MuxAsGpio);
    PORT_SetPinMux(PORTA,  5U, kPORT_MuxAsGpio);
    GPIO_PinInit(GPIOA, 5U, &(gpio_pin_config_t){kGPIO_DigitalOutput, (0)});
    PORT_SetPinMux(PORTA, 10U, kPORT_MuxAsGpio);
    PORT_SetPinMux(PORTA, 11U, kPORT_MuxAsGpio);
    PORT_SetPinMux(PORTA, 13U, kPORT_MuxAsGpio);
    PORT_SetPinMux(PORTA, 16U, kPORT_MuxAsGpio);
    PORT_SetPinMux(PORTA, 17U, kPORT_MuxAsGpio);

    /* Key pin mux Configuration */
    port_pin_config_t config_sos = {0};
    config_sos.pullSelect = kPORT_PullUp;
    config_sos.mux = kPORT_MuxAsGpio;
    PORT_SetPinConfig(PORTA, 29u, &config_sos);			//按键检测信号；默认配置为INPUT,PULL-HIGH；当摁键事件发生时，输入信号会有高变低
    port_pin_config_t config_k24reset = {0};
    config_k24reset.pullSelect = kPORT_PullDown;
    config_k24reset.mux = kPORT_MuxAsGpio;
    PORT_SetPinConfig(PORTE, 2U, &config_k24reset);     //系统reset,按下为高
    port_pin_config_t config_poweroff = {0};
    config_poweroff.pullSelect = kPORT_PullDisable;
    config_poweroff.mux = kPORT_MuxAsGpio;
    PORT_SetPinConfig(PORTE, 6U, &config_poweroff);     //8051检测到关机按键后通知K24

    PORT_SetPinMux(PORTE,  4U, kPORT_MuxAsGpio);        //New board PTE4 ctrl POWER DOWN, K24输出关机信号给8051

    /* for zigbee ctrl pin*/
    PORT_SetPinMux(PORTD, 14U, kPORT_MuxAsGpio);
    PORT_SetPinMux(PORTD, 15U, kPORT_MuxAsGpio);		//New board PTE4 ctrl POWER DOWN
    POWER_OFF_GPIO_INIT();
    
    /* for silver */
    PORT_SetPinMux(PORTD, 12U, kPORT_MuxAsGpio);        //SILVER模块插入检测信号；默认配置为INPUT,PULL-HIGH；有模块插入时，信号由高变低，并持续保持为低时。
    PORT_SetPinMux(PORTD, 13U, kPORT_MuxAsGpio);        //SILVER模块电源开关；默认配置为OUTPUT LOW，关闭电源；OUTPUT HIGH时，打开电源。

    /* for plc ctrl pin */
    PORT_SetPinMux(PORTB, 21U, kPORT_MuxAsGpio);        //PLC模块接口T_REQ信号，根据ST7580芯片规格书进行配置

    /* for wifi ctrl pin */
    PORT_SetPinMux(PORTC,  0U, kPORT_MuxAsGpio);        //WIFI模块使能控制信号；默认状态为OUTPUT LOW，WIFI关闭；当OUTPUT HIGH时，WIFI打开

    /* for battery */
    port_pin_config_t config_nPG = {0};
    config_nPG.pullSelect = kPORT_PullDisable;
    config_nPG.mux = kPORT_MuxAsGpio;
    PORT_SetPinConfig(PORTB, 20u, &config_nPG);        	//电池管理模块 nPG信号
    PORT_SetPinMux(PORTB,  6U, kPORT_MuxAsGpio);        //电池管理模块 nCHG信号
    PORT_SetPinMux(PORTB,  0U, kPORT_PinDisabledOrAnalog);  //电池电压检测
    
    /* for MODEM_2G */
	#if 0
    port_pin_config_t config_emerg_rst = {0};
    config_emerg_rst.pullSelect = kPORT_PullUp;
    config_emerg_rst.mux = kPORT_MuxAsGpio;
    PORT_SetPinConfig(PORTA, 12u, &config_emerg_rst);        	//2G_emerg_rst
	#endif
    PORT_SetPinMux(PORTA, 12U, kPORT_MuxAsGpio);

	
    port_pin_config_t config_dcd = {0};
    config_dcd.pullSelect = kPORT_PullUp;
    config_dcd.mux = kPORT_MuxAsGpio;
    PORT_SetPinConfig(PORTB, 12u, &config_dcd);        	//2G_DCD
    port_pin_config_t config_ring = {0};
    config_ring.pullSelect = kPORT_PullUp;
    config_ring.mux = kPORT_MuxAsGpio;
    PORT_SetPinConfig(PORTB, 13u, &config_ring);        //2G_RING
    PORT_SetPinMux(PORTB, 16U, kPORT_MuxAsGpio);        //2G_DCR
    PORT_SetPinMux(PORTB,  9U, kPORT_MuxAsGpio);        //2G_GSM_ON_OFF, KM 2G模块，GSM_ON_OFF信号；低有效，拉低3.5s以上会启动ON/OFF动作
    PORT_SetPinMux(PORTB,  7U, kPORT_MuxAsGpio);        //2G_PWR_MON, KM 2G模块，2G_PWR_MON信号；默认配置为INPUT；当系统正常起来的时候会输出一个信号表示系统已经起来

    /* for speaker en */
    port_pin_config_t config_spk = {0};
    config_spk.pullSelect = kPORT_PullDown;
    config_spk.mux = kPORT_MuxAsGpio;
    PORT_SetPinConfig(PORTB,  8U, &config_spk);        //SPEAKER 使能信号;默认状态为OUTPUT LOW；OUTPUT HIGH时，打开扬声器。

    /* for PEAK_SENSOR*/
    port_pin_config_t config_overvoltage = {0};
    config_overvoltage.pullSelect = kPORT_PullDown;
    config_overvoltage.mux = kPORT_MuxAsGpio;
    PORT_SetPinConfig(PORTA, 18U, &config_overvoltage); //PEAK SENSOR 过压触发中断输入信号；默认配置为INPUT, PULL-LOW；当过压发生时，输入信号由低变高。
    PORT_SetPinMux(PORTA, 19U, kPORT_MuxAlt6);          //PEAK SENSOR 欠压计数检测信号；默认配置为INPUT，计数；当欠压发生时，脉冲计数数量小于100/秒。

    /* RTC_CLKOUT */
    PORT_SetPinMux(PORTE,  0U, kPORT_MuxAlt7);


//#ifdef  WIFI_MODULE
    /*enable wifi module*/
    WIFI_EN_GPIO_INIT();
//#endif

#endif
}
