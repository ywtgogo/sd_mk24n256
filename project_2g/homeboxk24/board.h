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

#ifndef _BOARD_H_
#define _BOARD_H_

#include "clock_config.h"
#include "fsl_gpio.h"

#define HW_V01	1
#define HW_V02	2
#define HW_V03  3


#if (BOARD_NAME == HW_V01)
	/* The UART to use for debug messages. */
	#define BOARD_DEBUG_UART_TYPE DEBUG_CONSOLE_DEVICE_TYPE_UART
	#define BOARD_DEBUG_UART_BASEADDR (uint32_t) UART0
	#define BOARD_DEBUG_UART_CLKSRC SYS_CLK
	#define BOARD_DEBUG_UART_CLK_FREQ CLOCK_GetCoreSysClkFreq()
	#define BOARD_UART_IRQ UART0_RX_TX_IRQn
	#define BOARD_UART_IRQ_HANDLER UART0_RX_TX_IRQHandler

	#ifndef BOARD_DEBUG_UART_BAUDRATE
		#define BOARD_DEBUG_UART_BAUDRATE 921600
	#endif /* BOARD_DEBUG_UART_BAUDRATE */

	/* KM-UART instance and clock */
	#define MODEM_UART UART1
	#define MODEM_UART_CLKSRC UART1_CLK_SRC
	#define MODEM_UART_RX_TX_IRQn UART1_RX_TX_IRQn
	/* WIFI-UART instance and clock */
	#define WIFI_UART UART2
	#define WIFI_UART_CLKSRC UART2_CLK_SRC
	#define WIFI_UART_RX_TX_IRQn UART2_RX_TX_IRQn
	/* PLC-UART instance and clock */
	#define PLC_UART UART2
	#define PLC_UART_CLKSRC UART2_CLK_SRC
	#define PLC_UART_RX_TX_IRQn UART2_RX_TX_IRQn
	/* Console-UART instance and clock */
	#define CONSOLE_UART UART0
	#define CONSOLE_UART_CLKSRC UART0_CLK_SRC
	#define CONSOLE_UART_RX_TX_IRQn UART0_RX_TX_IRQn

	/* Board led color mapping */
	#define LOGIC_HIGH                  1U
	#define LOGIC_LOW                   0U
	#define LOGIC_LED_ON                0U
	#define LOGIC_LED_OFF               1U
	#define LEDx_COLOR_GPIO             GPIOA
	#define LED1_YELLOW_GPIO            GPIOD
	#define LED1_YELLOW_PIN             15U
	#define LED1_GREEN_PIN              1U
	#define LED1_BLUE_PIN               2U
	#define LED1_RED_PIN                5U

	#define LED1_RED_INIT(output)       GPIO_PinInit(LEDx_COLOR_GPIO, LED1_RED_PIN, &(gpio_pin_config_t){kGPIO_DigitalOutput, (output)}) /*!< Enable target LED_RED */
	#define LED1_RED_ON()               GPIO_ClearPinsOutput(LEDx_COLOR_GPIO, 1U << LED1_RED_PIN) /*!< Turn on target LED_RED */
	#define LED1_RED_OFF()              GPIO_SetPinsOutput(LEDx_COLOR_GPIO, 1U << LED1_RED_PIN) /*!< Turn off target LED_RED */
	#define LED1_RED_TOGGLE()           GPIO_TogglePinsOutput(LEDx_COLOR_GPIO, 1U << LED1_RED_PIN) /*!< Toggle on target LED_RED */

	#define LED1_YELLOW_INIT(output)    GPIO_PinInit(LED1_YELLOW_GPIO, LED1_YELLOW_PIN, &(gpio_pin_config_t){kGPIO_DigitalOutput, (output)}) /*!< Enable target LED_RED */
	#define LED1_YELLOW_ON()            GPIO_ClearPinsOutput(LED1_YELLOW_GPIO, 1U << LED1_YELLOW_PIN) /*!< Turn on target LED_RED */
	#define LED1_YELLOW_OFF()           GPIO_SetPinsOutput(LED1_YELLOW_GPIO, 1U << LED1_YELLOW_PIN) /*!< Turn off target LED_RED */
	#define LED1_YELLOW_TOGGLE()        GPIO_TogglePinsOutput(LED1_YELLOW_GPIO, 1U << LED1_YELLOW_PIN) /*!< Toggle on target LED_RED */

	#define LED1_BLUE_INIT(output)      GPIO_PinInit(LEDx_COLOR_GPIO, LED1_BLUE_PIN, &(gpio_pin_config_t){kGPIO_DigitalOutput, (output)}) /*!< Enable target LED_RED */
	#define LED1_BLUE_ON()              GPIO_ClearPinsOutput(LEDx_COLOR_GPIO, 1U << LED1_BLUE_PIN) /*!< Turn on target LED_RED */
	#define LED1_BLUE_OFF()             GPIO_SetPinsOutput(LEDx_COLOR_GPIO, 1U << LED1_BLUE_PIN) /*!< Turn off target LED_RED */
	#define LED1_BLUE_TOGGLE()          GPIO_TogglePinsOutput(LEDx_COLOR_GPIO, 1U << LED1_BLUE_PIN) /*!< Toggle on target LED_RED */

	#define LED1_GREEN_INIT(output)     GPIO_PinInit(LEDx_COLOR_GPIO, LED1_GREEN_PIN, &(gpio_pin_config_t){kGPIO_DigitalOutput, (output)}) /*!< Enable target LED_RED */
	#define LED1_GREEN_ON()             GPIO_ClearPinsOutput(LEDx_COLOR_GPIO, 1U << LED1_GREEN_PIN) /*!< Turn on target LED_RED */
	#define LED1_GREEN_OFF()            GPIO_SetPinsOutput(LEDx_COLOR_GPIO, 1U << LED1_GREEN_PIN) /*!< Turn off target LED_RED */
	#define LED1_GREEN_TOGGLE()         GPIO_TogglePinsOutput(LEDx_COLOR_GPIO, 1U << LED1_GREEN_PIN) /*!< Toggle on target LED_RED */

	#define RF_LED_INIT()  				GPIO_PinInit(GPIOD, 14U, &(gpio_pin_config_t){kGPIO_DigitalOutput, (0U)});
	#define RF_LED_OFF()           		GPIO_SetPinsOutput(GPIOD, 1U << 14);
	#define RF_LED_ON()            		GPIO_ClearPinsOutput(GPIOD, 1U << 14)
	#define RF_LED_TOGGLE()        		GPIO_TogglePinsOutput(GPIOD, 1U << 14)

	#define PPP_LED_INIT()				GPIO_PinInit(GPIOD, 15U, &(gpio_pin_config_t){kGPIO_DigitalOutput, (0U)});
	#define PPP_LED_OFF() 				GPIO_SetPinsOutput(GPIOD, 1U << 15);
	#define PPP_LED_ON()  				GPIO_ClearPinsOutput(GPIOD, 1U << 15)
	#define PPP_LED_TOGGLE()     		GPIO_TogglePinsOutput(GPIOD, 1U << 15)

	#define TP51_INIT()     			LED3_RED_INIT(LOGIC_HIGH)
	#define TP51_ON()           		LED3_RED_ON()
	/* end led */

	#define K24_POWER_LOCK_INIT()       GPIO_PinInit(GPIOE, 6U, &(gpio_pin_config_t){kGPIO_DigitalOutput, (1U)})
	#define K24_POWER_LOCK_ON()         GPIO_SetPinsOutput(GPIOE, 1U << 6)
	#define K24_POWER_LOCK_OFF()        GPIO_ClearPinsOutput(GPIOE, 1U << 6)

	#define SPKR_GPIO_INIT()            GPIO_PinInit(GPIOB, 8U,  &(gpio_pin_config_t){kGPIO_DigitalOutput, (1U)})
	#define SPKR_ON()                   GPIO_SetPinsOutput(GPIOB, 1U << 8)
	#define SPKR_OFF()                  GPIO_ClearPinsOutput(GPIOB, 1U << 8)
	#define SPKR_TOGGLE()               GPIO_TogglePinsOutput(GPIOB, 1U << 8)

	#define A7139_GIO2_GPIO             GPIOC
	#define A7139_GIO2_PORT             PORTC
	#define A7139_GIO2_GPIO_PIN         19U
	#define A7139_GIO2_IRQ              PORTC_IRQn
	#define A7139_GIO2_IRQ_HANDLER      PORTC_IRQHandler
	#define A7139_GIO2_INIT()           GPIO_PinInit(A7139_GIO2_GPIO, A7139_GIO2_GPIO_PIN, &(gpio_pin_config_t){kGPIO_DigitalInput, (1U)})

	#define MODEM_POWER_INIT()            	GPIO_PinInit(GPIOD, 8U, &(gpio_pin_config_t){kGPIO_DigitalOutput, (1U)})
	#define MODEM_POWER_OFF()             	GPIO_SetPinsOutput(GPIOD, 1U << 8)
	#define MODEM_POWER_ON()              	GPIO_ClearPinsOutput(GPIOD, 1U << 8)
	#define MODEM_POWER_TOGGLE()          	GPIO_TogglePinsOutput(GPIOD, 1U << 8)

	#define MODEM_DCD_GPIO                GPIOB
	#define MODEM_DCD_PORT                PORTB
	#define MODEM_DCD_GPIO_PIN            12U
	#define MODEM_DCD_IRQ                 PORTB_IRQn
	#define MODEM_DCD_IRQ_HANDLER         PORTB_IRQHandler
	#define MODEM_DCD_INIT()              GPIO_PinInit(MODEM_DCD_GPIO, MODEM_DCD_GPIO_PIN, &(gpio_pin_config_t){kGPIO_DigitalInput, (1U)})
	#define MODEM_DCD_READ() 		       GPIO_ReadPinInput(MODEM_DCD_GPIO, MODEM_DCD_GPIO_PIN)

	#define MODEM_RING_SMS_GPIO           	GPIOB
	#define MODEM_RING_SMS_PORT           	PORTB
	#define MODEM_RING_SMS_GPIO_PIN       	13U
	#define MODEM_RING_SMS_IRQ            	PORTB_IRQn
	#define MODEM_RING_SMS_IRQ_HANDLER    	PORTB_IRQHandler
	#define MODEM_RING_SMS_INIT()         	GPIO_PinInit(MODEM_RING_SMS_GPIO, MODEM_RING_SMS_GPIO_PIN, &(gpio_pin_config_t){kGPIO_DigitalInput, (1U)})
	#define MODEM_RING_SMS_READ() 	    	GPIO_ReadPinInput(MODEM_RING_SMS_GPIO, MODEM_RING_SMS_GPIO_PIN)


	#define MODEM_SOS_GPIO                	GPIOE
	#define MODEM_SOS_PORT                	PORTE
	#define MODEM_SOS_GPIO_PIN            	2U
	#define MODEM_SOS_IRQ                 	PORTE_IRQn
	//#define MODEM_SOS_IRQ_HANDLER         	PORTE_IRQHandler
	#define MODEM_SOS_INIT()              	GPIO_PinInit(MODEM_SOS_GPIO, MODEM_SOS_GPIO_PIN, &(gpio_pin_config_t){kGPIO_DigitalInput, (1U)})
	#define MODEM_SOS_READ() 		       	GPIO_ReadPinInput(MODEM_SOS_GPIO, MODEM_SOS_GPIO_PIN)

	#define BAT_nPG_GPIO                GPIOB
	#define BAT_nPG_PORT                PORTB
	#define BAT_nPG_GPIO_PIN            20U
	#define BAT_nPG_IRQ                 PORTA_IRQn
	#define BAT_nPG_IRQ_HANDLER         PORTA_IRQHandler
	#define BAT_nPG_INIT()              GPIO_PinInit(BAT_nPG_GPIO, BAT_nPG_GPIO_PIN, &(gpio_pin_config_t){kGPIO_DigitalInput, (1U)})
	#define BAT_nPG_READ() 		        GPIO_ReadPinInput(BAT_nPG_GPIO, BAT_nPG_GPIO_PIN)

	#define BAT_nCHG_GPIO               GPIOB
	#define BAT_nCHG_PORT               PORTB
	#define BAT_nCHG_GPIO_PIN           6U
	#define BAT_nCHG_IRQ                PORTA_IRQn
	#define BAT_nCHG_IRQ_HANDLER        PORTA_IRQHandler
	#define BAT_nCHG_INIT()             GPIO_PinInit(BAT_nCHG_GPIO, BAT_nCHG_GPIO_PIN, &(gpio_pin_config_t){kGPIO_DigitalInput, (1U)})
	#define BAT_nCHG_READ() 		    GPIO_ReadPinInput(BAT_nCHG_GPIO, BAT_nCHG_GPIO_PIN)

	#define OVERVOLTAGE_GPIO            GPIOA
	#define OVERVOLTAGE_PORT            PORTA
	#define OVERVOLTAGE_GPIO_PIN        18U
	#define OVERVOLTAGE_IRQ             PORTA_IRQn
	#define OVERVOLTAGE_IRQ_HANDLER     PORTA_IRQHandler
	#define OVERVOLTAGE_INIT()    		GPIO_PinInit(OVERVOLTAGE_GPIO, OVERVOLTAGE_GPIO_PIN, &(gpio_pin_config_t){kGPIO_DigitalInput, (1U)})

	#define BAT_ADC16_BASE              ADC0
	#define BAT_ADC16_CHANNEL_GROUP     0U
	#define BAT_ADC16_USER_CHANNEL      8U
	#define BAT_ADC16_IRQn              ADC0_IRQn
	#define BAT_ADC16_IRQ_HANDLER_FUNC  ADC0_IRQHandler


#elif (BOARD_NAME >= HW_V02)
	/*******************************************************************************
	 * Definitions
	 ******************************************************************************/
	#define SHELL_Printf       PRINTF
	//#define BOARD_NAME "FRDM-K64F"

	/* The UART to use for debug messages. */
	#define BOARD_DEBUG_UART_TYPE DEBUG_CONSOLE_DEVICE_TYPE_UART
	#define BOARD_DEBUG_UART_BASEADDR (uint32_t) UART5
	#define BOARD_DEBUG_UART_CLKSRC UART5_CLK_SRC
	#define BOARD_DEBUG_UART_CLK_FREQ CLOCK_GetBusClkFreq()
	#define BOARD_UART_IRQ UART5_RX_TX_IRQn
	#define BOARD_UART_IRQ_HANDLER UART5_RX_TX_IRQHandler

	#ifndef BOARD_DEBUG_UART_BAUDRATE
		#define BOARD_DEBUG_UART_BAUDRATE 	921600  //115200
	#endif /* BOARD_DEBUG_UART_BAUDRATE */

	/* KM-UART instance and clock */
	#define MODEM_UART UART0
	#define MODEM_UART_CLKSRC UART0_CLK_SRC
	#define MODEM_UART_RX_TX_IRQn UART0_RX_TX_IRQn
	/* WIFI-UART instance and clock */
	#define WIFI_UART UART1
	#define WIFI_UART_CLKSRC UART1_CLK_SRC
	#define WIFI_UART_RX_TX_IRQn UART1_RX_TX_IRQn
	/* SILVER-UART instance and clock */
	#define SILVER_UART UART2
	#define SILVER_UART_CLKSRC UART2_CLK_SRC
	#define SILVER_UART_RX_TX_IRQn UART2_RX_TX_IRQn
	/* PLC-UART instance and clock */
	#define PLC_UART UART3
	#define PLC_UART_CLKSRC UART3_CLK_SRC
	#define PLC_UART_RX_TX_IRQn UART3_RX_TX_IRQn
	/* ZIGBEE-UART instance and clock */
	#define ZIGBEE_UART UART4
	#define ZIGBEE_UART_CLKSRC UART4_CLK_SRC
	#define ZIGBEE_UART_RX_TX_IRQn UART4_RX_TX_IRQn
	/* Console-UART instance and clock */
	#define CONSOLE_UART UART5
	#define CONSOLE_UART_CLKSRC UART5_CLK_SRC
	#define CONSOLE_UART_RX_TX_IRQn UART5_RX_TX_IRQn

	/* BQ25050 */
	#define LOGIC_HIGH                  1U
	#define LOGIC_LOW                   0U

	#define CHG_CTRL_PORT               PORTA
	#define CHG_CTRL_GPIO               GPIOA
	#define CHG_CTRL_GPIO_PIN           5U

	#define CHG_CTRL_INIT(output)       GPIO_PinInit(CHG_CTRL_GPIO, CHG_CTRL_GPIO_PIN, &(gpio_pin_config_t){kGPIO_DigitalOutput, (output)}) /*!< Enable target LED_RED */
	#define CHG_CTRL_LOW()              GPIO_ClearPinsOutput(CHG_CTRL_GPIO, 1U << CHG_CTRL_GPIO_PIN) /*!< Turn on target LED_RED */
	#define CHG_CTRL_HIGH()             GPIO_SetPinsOutput(CHG_CTRL_GPIO, 1U << CHG_CTRL_GPIO_PIN) /*!< Turn off target LED_RED */

	#define AUDIO_MSG_PORT              PORTA
	#define AUDIO_MSG_GPIO              GPIOA
	#define AUDIO_MSG_GPIO_PIN          2U

	#define AUDIO_MSG_INIT()            GPIO_PinInit(AUDIO_MSG_GPIO, AUDIO_MSG_GPIO_PIN, &(gpio_pin_config_t){kGPIO_DigitalOutput, (1)})
	#define AUDIO_MSG_DAC()             GPIO_SetPinsOutput(AUDIO_MSG_GPIO, 1U << AUDIO_MSG_GPIO_PIN)
	#define AUDIO_MSG_2G()              GPIO_ClearPinsOutput(AUDIO_MSG_GPIO, 1U << AUDIO_MSG_GPIO_PIN)

	/* Board led color mapping */
	#define LOGIC_HIGH                  1U
	#define LOGIC_LOW                   0U
	#define LOGIC_LED_ON                0U
	#define LOGIC_LED_OFF               1U
	#define LEDx_COLOR_GPIO             GPIOA
	#define LED1_YELLOW_GPIO            GPIOD
	#define LED1_YELLOW_PIN             15U
	#define LED1_GREEN_PIN              1U
	#define LED1_BLUE_PIN               2U
	#define LED1_RED_PIN                5U
	#define LED2_GREEN_PIN              10U
	#define LED2_BLUE_PIN               11U
	#define LED2_RED_PIN                12U
	#define LED3_GREEN_PIN              13U
	#define LED3_BLUE_PIN               16U
	#define LED3_RED_PIN                17U

	#define LED1_RED_INIT(output)       GPIO_PinInit(LEDx_COLOR_GPIO, LED1_RED_PIN, &(gpio_pin_config_t){kGPIO_DigitalOutput, (output)}) /*!< Enable target LED_RED */
	#define LED1_RED_ON()               GPIO_ClearPinsOutput(LEDx_COLOR_GPIO, 1U << LED1_RED_PIN) /*!< Turn on target LED_RED */
	#define LED1_RED_OFF()              GPIO_SetPinsOutput(LEDx_COLOR_GPIO, 1U << LED1_RED_PIN) /*!< Turn off target LED_RED */
	#define LED1_RED_TOGGLE()           GPIO_TogglePinsOutput(LEDx_COLOR_GPIO, 1U << LED1_RED_PIN) /*!< Toggle on target LED_RED */

	#define LED1_YELLOW_INIT(output)    GPIO_PinInit(LED1_YELLOW_GPIO, LED1_YELLOW_PIN, &(gpio_pin_config_t){kGPIO_DigitalOutput, (output)}) /*!< Enable target LED_RED */
	#define LED1_YELLOW_ON()            GPIO_ClearPinsOutput(LED1_YELLOW_GPIO, 1U << LED1_YELLOW_PIN) /*!< Turn on target LED_RED */
	#define LED1_YELLOW_OFF()           GPIO_SetPinsOutput(LED1_YELLOW_GPIO, 1U << LED1_YELLOW_PIN) /*!< Turn off target LED_RED */
	#define LED1_YELLOW_TOGGLE()        GPIO_TogglePinsOutput(LED1_YELLOW_GPIO, 1U << LED1_YELLOW_PIN) /*!< Toggle on target LED_RED */

	#define LED1_BLUE_INIT(output)      GPIO_PinInit(LEDx_COLOR_GPIO, LED1_BLUE_PIN, &(gpio_pin_config_t){kGPIO_DigitalOutput, (output)}) /*!< Enable target LED_RED */
	#define LED1_BLUE_ON()              GPIO_ClearPinsOutput(LEDx_COLOR_GPIO, 1U << LED1_BLUE_PIN) /*!< Turn on target LED_RED */
	#define LED1_BLUE_OFF()             GPIO_SetPinsOutput(LEDx_COLOR_GPIO, 1U << LED1_BLUE_PIN) /*!< Turn off target LED_RED */
	#define LED1_BLUE_TOGGLE()          GPIO_TogglePinsOutput(LEDx_COLOR_GPIO, 1U << LED1_BLUE_PIN) /*!< Toggle on target LED_RED */

	#define LED1_GREEN_INIT(output)     GPIO_PinInit(LEDx_COLOR_GPIO, LED1_GREEN_PIN, &(gpio_pin_config_t){kGPIO_DigitalOutput, (output)}) /*!< Enable target LED_RED */
	#define LED1_GREEN_ON()             GPIO_ClearPinsOutput(LEDx_COLOR_GPIO, 1U << LED1_GREEN_PIN) /*!< Turn on target LED_RED */
	#define LED1_GREEN_OFF()            GPIO_SetPinsOutput(LEDx_COLOR_GPIO, 1U << LED1_GREEN_PIN) /*!< Turn off target LED_RED */
	#define LED1_GREEN_TOGGLE()         GPIO_TogglePinsOutput(LEDx_COLOR_GPIO, 1U << LED1_GREEN_PIN) /*!< Toggle on target LED_RED */

	#define LED2_RED_INIT(output)       GPIO_PinInit(LEDx_COLOR_GPIO, LED2_RED_PIN, &(gpio_pin_config_t){kGPIO_DigitalOutput, (output)})
	#define LED2_RED_ON()               GPIO_ClearPinsOutput(LEDx_COLOR_GPIO, 1U << LED2_RED_PIN)
	#define LED2_RED_OFF()              GPIO_SetPinsOutput(LEDx_COLOR_GPIO, 1U << LED2_RED_PIN)
	#define LED2_RED_TOGGLE()           GPIO_TogglePinsOutput(LEDx_COLOR_GPIO, 1U << LED2_RED_PIN)

	#define LED2_BLUE_INIT(output)      GPIO_PinInit(LEDx_COLOR_GPIO, LED2_BLUE_PIN, &(gpio_pin_config_t){kGPIO_DigitalOutput, (output)})
	#define LED2_BLUE_ON()              GPIO_ClearPinsOutput(LEDx_COLOR_GPIO, 1U << LED2_BLUE_PIN)
	#define LED2_BLUE_OFF()             GPIO_SetPinsOutput(LEDx_COLOR_GPIO, 1U << LED2_BLUE_PIN)
	#define LED2_BLUE_TOGGLE()          GPIO_TogglePinsOutput(LEDx_COLOR_GPIO, 1U << LED2_BLUE_PIN)

	#define LED2_GREEN_INIT(output)     GPIO_PinInit(LEDx_COLOR_GPIO, LED2_GREEN_PIN, &(gpio_pin_config_t){kGPIO_DigitalOutput, (output)})
	#define LED2_GREEN_ON()             GPIO_ClearPinsOutput(LEDx_COLOR_GPIO, 1U << LED2_GREEN_PIN)
	#define LED2_GREEN_OFF()            GPIO_SetPinsOutput(LEDx_COLOR_GPIO, 1U << LED2_GREEN_PIN)
	#define LED2_GREEN_TOGGLE()         GPIO_TogglePinsOutput(LEDx_COLOR_GPIO, 1U << LED2_GREEN_PIN)

	#define LED3_RED_INIT(output)       GPIO_PinInit(LEDx_COLOR_GPIO, LED3_RED_PIN, &(gpio_pin_config_t){kGPIO_DigitalOutput, (output)})
	#define LED3_RED_ON()               GPIO_ClearPinsOutput(LEDx_COLOR_GPIO, 1U << LED3_RED_PIN)
	#define LED3_RED_OFF()              GPIO_SetPinsOutput(LEDx_COLOR_GPIO, 1U << LED3_RED_PIN)
	#define LED3_RED_TOGGLE()           GPIO_TogglePinsOutput(LEDx_COLOR_GPIO, 1U << LED3_RED_PIN)

	#define LED3_BLUE_INIT(output)      GPIO_PinInit(LEDx_COLOR_GPIO, LED3_BLUE_PIN, &(gpio_pin_config_t){kGPIO_DigitalOutput, (output)})
	#define LED3_BLUE_ON()              GPIO_ClearPinsOutput(LEDx_COLOR_GPIO, 1U << LED3_BLUE_PIN)
	#define LED3_BLUE_OFF()             GPIO_SetPinsOutput(LEDx_COLOR_GPIO, 1U << LED3_BLUE_PIN)
	#define LED3_BLUE_TOGGLE()          GPIO_TogglePinsOutput(LEDx_COLOR_GPIO, 1U << LED3_BLUE_PIN)

	#define LED3_GREEN_INIT(output)     GPIO_PinInit(LEDx_COLOR_GPIO, LED3_GREEN_PIN, &(gpio_pin_config_t){kGPIO_DigitalOutput, (output)})
	#define LED3_GREEN_ON()             GPIO_ClearPinsOutput(LEDx_COLOR_GPIO, 1U << LED3_GREEN_PIN)
	#define LED3_GREEN_OFF()            GPIO_SetPinsOutput(LEDx_COLOR_GPIO, 1U << LED3_GREEN_PIN)
	#define LED3_GREEN_TOGGLE()         GPIO_TogglePinsOutput(LEDx_COLOR_GPIO, 1U << LED3_GREEN_PIN)

	#ifdef WIFI_MODULE
		#define WIFI_GREEN_LED_INIT()		LED2_GREEN_INIT(LOGIC_LOW)
		#define WIFI_GREEN_LED_OFF() 		LED2_GREEN_OFF()
		#define WIFI_GREEN_LED_ON()  		LED2_GREEN_ON()
		#define WIFI_GREEN_LED_TOGGLE()     	LED2_GREEN_TOGGLE()
	#else
		#define PPP_GREEN_LED_INIT()		LED2_GREEN_INIT(LOGIC_LOW)
		#define PPP_GREEN_LED_OFF() 		LED2_GREEN_OFF()
		#define PPP_GREEN_LED_ON()  		LED2_GREEN_ON()
		#define PPP_GREEN_LED_TOGGLE()     	LED2_GREEN_TOGGLE()
	#endif

	#ifdef WIFI_MODULE
		#define WIFI_RED_LED_INIT()			LED3_RED_INIT(LOGIC_LOW)
		#define WIFI_RED_LED_OFF() 			LED3_RED_OFF()
		#define WIFI_RED_LED_ON()  			LED3_RED_ON()
		#define WIFI_RED_LED_TOGGLE()     	LED3_RED_TOGGLE()
	#else
		#define PPP_RED_LED_INIT()			LED3_RED_INIT(LOGIC_LOW)
		#define PPP_RED_LED_OFF() 			LED3_RED_OFF()
		#define PPP_RED_LED_ON()  			LED3_RED_ON()
		#define PPP_RED_LED_TOGGLE()     	LED3_RED_TOGGLE()
	#endif
	#if 1
		#define RF_LED_INIT()  				//LED2_GREEN_INIT(LOGIC_HIGH)
		#define RF_LED_OFF()           		//LED2_GREEN_ON()
		#define RF_LED_ON()            		//LED2_GREEN_OFF()
		#define RF_LED_TOGGLE()        		//LED2_GREEN_TOGGLE()
	#endif

	#define K24_POWER_LED_INIT()     	LED3_RED_INIT(LOGIC_LOW)
	#define K24_POWER_LED_OFF()         LED3_RED_OFF()
	#define K24_POWER_LED_ON() 		    LED3_RED_ON()
	/* end led */

	#define POWER_KEY_GPIO           	GPIOE
	#define POWER_KEY_PORT           	PORTE
	#define POWER_KEY_GPIO_PIN       	6U
	#define POWER_KEY_IRQ            	PORTE_IRQn
	//#define POWER_KEY_IRQ_HANDLER    	PORTE_IRQHandler
	#define POWER_KEY_INIT()         	GPIO_PinInit(POWER_KEY_GPIO, POWER_KEY_GPIO_PIN, &(gpio_pin_config_t){kGPIO_DigitalInput, (1U)})
	#define POWER_KEY_READ() 	    	GPIO_ReadPinInput(POWER_KEY_GPIO, POWER_KEY_GPIO_PIN)

	#if (BOARD_NAME == HW_V03)
		#define POWER_OFF_GPIO_INIT()       GPIO_PinInit(GPIOE, 4U,  &(gpio_pin_config_t){kGPIO_DigitalOutput, (LOGIC_HIGH)})	//new board PTE4
		#define POWER_OFF()               	GPIO_ClearPinsOutput(GPIOE, 1U << 4)
	#else
		#define POWER_OFF_GPIO_INIT()     	GPIO_PinInit(GPIOD, 15U,  &(gpio_pin_config_t){kGPIO_DigitalOutput, (LOGIC_HIGH)})	//new board PTE4
		#define POWER_OFF()               	GPIO_ClearPinsOutput(GPIOD, 1U << 15)
	#endif

	#define WIFI_EN_GPIO_INIT()        	GPIO_PinInit(GPIOC, 0U,  &(gpio_pin_config_t){kGPIO_DigitalOutput, (LOGIC_HIGH)})
	#define WIFI_EN_ON()                GPIO_SetPinsOutput(GPIOC, 1U << 0)
	#define WIFI_EN_OFF()               GPIO_ClearPinsOutput(GPIOC, 1U << 0)

	#define SPKR_GPIO_INIT()            GPIO_PinInit(GPIOB, 8U,  &(gpio_pin_config_t){kGPIO_DigitalOutput, (1U)})
	#define SPKR_ON()                   GPIO_SetPinsOutput(GPIOB, 1U << 8)
	#define SPKR_OFF()                  GPIO_ClearPinsOutput(GPIOB, 1U << 8)
	#define SPKR_TOGGLE()               GPIO_TogglePinsOutput(GPIOB, 1U << 8)

	#define SPKR_SWT_GPIO_INIT()        GPIO_PinInit(GPIOA, 2U,  &(gpio_pin_config_t){kGPIO_DigitalOutput, (1U)})
	#define SPKR_SWT_OFF()              GPIO_SetPinsOutput(GPIOA, 1U << 2)
	#define SPKR_SWT_ON()              	GPIO_ClearPinsOutput(GPIOA, 1U << 2)
	#define SPKR_SWT_TOGGLE()           GPIO_TogglePinsOutput(GPIOA, 1U << 2)

	#define PLC_TREQ_GPIO               GPIOB
	#define PLC_TREQ_PIN                21U
	#define PLC_TREQ_GPIO_INIT()        GPIO_PinInit(GPIOB, PLC_TREQ_PIN,  &(gpio_pin_config_t){kGPIO_DigitalOutput, (0U)})
	#define PLC_TREQ_ON()               GPIO_SetPinsOutput(GPIOB, 1U << PLC_TREQ_PIN)
	#define PLC_TREQ_OFF()              GPIO_ClearPinsOutput(GPIOB, 1U << PLC_TREQ_PIN)

	#define A7139_GIO2_GPIO             GPIOC
	#define A7139_GIO2_PORT             PORTC
	#define A7139_GIO2_GPIO_PIN         19U
	#define A7139_GIO2_IRQ              PORTC_IRQn
	//#define A7139_GIO2_IRQ_HANDLER    PORTC_IRQHandler
	#define A7139_GIO2_INIT()           GPIO_PinInit(A7139_GIO2_GPIO, A7139_GIO2_GPIO_PIN, &(gpio_pin_config_t){kGPIO_DigitalInput, (1U)})

	#define MODEM_POWER_INIT()            	GPIO_PinInit(GPIOB, 9U, &(gpio_pin_config_t){kGPIO_DigitalOutput, (1U)})
	#define MODEM_POWER_OFF()             	GPIO_SetPinsOutput(GPIOB, 1U << 9)
	#define MODEM_POWER_ON()              	GPIO_ClearPinsOutput(GPIOB, 1U << 9)
	#define MODEM_POWER_TOGGLE()          	GPIO_TogglePinsOutput(GPIOB, 1U << 9)

	#define MODEM_EMERG_RST_INIT()          GPIO_PinInit(GPIOA, 12U, &(gpio_pin_config_t){kGPIO_DigitalOutput, (1U)})
	#define MODEM_EMERG_RST_OFF()           GPIO_SetPinsOutput(GPIOA, 1U << 12)
	#define MODEM_EMERG_RST_ON()            GPIO_ClearPinsOutput(GPIOA, 1U << 12)

	#ifndef BOOTLOADER
		#define MODEM_DCD_GPIO                	GPIOB
		#define MODEM_DCD_PORT                	PORTB
		#define MODEM_DCD_GPIO_PIN            	12U
		#define MODEM_DCD_IRQ                 	PORTB_IRQn
		//#define MODEM_DCD_IRQ_HANDLER        PORTB_IRQHandler
		#define MODEM_DCD_INIT()              	GPIO_PinInit(MODEM_DCD_GPIO, MODEM_DCD_GPIO_PIN, &(gpio_pin_config_t){kGPIO_DigitalInput, (1U)})
		#define MODEM_DCD_READ() 		       	GPIO_ReadPinInput(MODEM_DCD_GPIO, MODEM_DCD_GPIO_PIN)

		#define MODEM_RING_SMS_GPIO           	GPIOB
		#define MODEM_RING_SMS_PORT           	PORTB
		#define MODEM_RING_SMS_GPIO_PIN       	13U
		#define MODEM_RING_SMS_IRQ            	PORTB_IRQn
		//#define MODEM_RING_SMS_IRQ_HANDLER   PORTB_IRQHandler
		#define MODEM_RING_SMS_INIT()         	GPIO_PinInit(MODEM_RING_SMS_GPIO, MODEM_RING_SMS_GPIO_PIN, &(gpio_pin_config_t){kGPIO_DigitalInput, (1U)})
		#define MODEM_RING_SMS_READ() 	    	GPIO_ReadPinInput(MODEM_RING_SMS_GPIO, MODEM_RING_SMS_GPIO_PIN)


		#define MODEM_SOS_GPIO                	GPIOA
		#define MODEM_SOS_PORT                	PORTA
		#define MODEM_SOS_GPIO_PIN            	29U
		#define MODEM_SOS_IRQ                 	PORTA_IRQn
		//#define MODEM_SOS_IRQ_HANDLER        PORTA_IRQHandler
		#define MODEM_SOS_INIT()              	GPIO_PinInit(MODEM_SOS_GPIO, MODEM_SOS_GPIO_PIN, &(gpio_pin_config_t){kGPIO_DigitalInput, (1U)})
		#define MODEM_SOS_READ() 		       	GPIO_ReadPinInput(MODEM_SOS_GPIO, MODEM_SOS_GPIO_PIN)
	#endif
	#define BAT_nPG_GPIO                GPIOB
	#define BAT_nPG_PORT                PORTB
	#define BAT_nPG_GPIO_PIN            20U
	#define BAT_nPG_IRQ                 PORTA_IRQn
	//#define BAT_nPG_IRQ_HANDLER       PORTA_IRQHandler
	#define BAT_nPG_INIT()              GPIO_PinInit(BAT_nPG_GPIO, BAT_nPG_GPIO_PIN, &(gpio_pin_config_t){kGPIO_DigitalInput, (1U)})
	#define BAT_nPG_READ() 		        GPIO_ReadPinInput(BAT_nPG_GPIO, BAT_nPG_GPIO_PIN)

	#define BAT_nCHG_GPIO               GPIOB
	#define BAT_nCHG_PORT               PORTB
	#define BAT_nCHG_GPIO_PIN           6U
	#define BAT_nCHG_IRQ                PORTA_IRQn
	//#define BAT_nCHG_IRQ_HANDLER      PORTA_IRQHandler
	#define BAT_nCHG_INIT()             GPIO_PinInit(BAT_nCHG_GPIO, BAT_nCHG_GPIO_PIN, &(gpio_pin_config_t){kGPIO_DigitalInput, (1U)})
	#define BAT_nCHG_READ() 		    GPIO_ReadPinInput(BAT_nCHG_GPIO, BAT_nCHG_GPIO_PIN)

	#define OVERVOLTAGE_GPIO            GPIOA
	#define OVERVOLTAGE_PORT            PORTA
	#define OVERVOLTAGE_GPIO_PIN        18U
	#define OVERVOLTAGE_IRQ             PORTA_IRQn
	//#define OVERVOLTAGE_IRQ_HANDLER   PORTA_IRQHandler
	#define OVERVOLTAGE_INIT()    		GPIO_PinInit(OVERVOLTAGE_GPIO, OVERVOLTAGE_GPIO_PIN, &(gpio_pin_config_t){kGPIO_DigitalInput, (1U)})

	#define BAT_ADC16_BASE              ADC0
	#define BAT_ADC16_CHANNEL_GROUP     0U
	#define BAT_ADC16_USER_CHANNEL      8U
	#define BAT_ADC16_IRQn              ADC0_IRQn
	#define BAT_ADC16_IRQ_HANDLER_FUNC  ADC0_IRQHandler

#else
	#error please config your BOARD_NAME
#endif

/* Get source clock for LPTMR driver */
#define LPTMR_SOURCE_CLOCK          CLOCK_GetFreq(kCLOCK_LpoClk)
/* Define LPTMR microseconds counts value */
#define LPTMR_USEC_COUNT            5000000U

/* @brief FreeRTOS tickless timer configuration. */
#define vPortLptmrIsr LPTMR0_IRQHandler /*!< Timer IRQ handler. */
#define TICKLESS_LPTMR_BASE_PTR LPTMR0  /*!< Tickless timer base address. */
#define TICKLESS_LPTMR_IRQn LPTMR0_IRQn /*!< Tickless timer IRQ number. */

/* @brief pit IRQ configuration for lwip demo */
#define LWIP_TIME_ISR PIT0_IRQHandler
#define LWIP_PIT_IRQ_ID PIT0_IRQn

#if defined(__cplusplus)
extern "C" {
#endif /* __cplusplus */

/*******************************************************************************
 * API
 ******************************************************************************/
void BOARD_InitDebugConsole(void);
//void Modem_Ring_Sms_Irq_Init();
//void Modem_SOS_Irq_Init();
//void Power_HighV_Irq_Init();
//void Power_LowV_Init();
//uint16_t Power_LowV_Read();
#if defined(__cplusplus)
}
#endif /* __cplusplus */

#endif /* _BOARD_H_ */
