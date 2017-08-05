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

#include <stdint.h>
#include <stdbool.h>
#include "fsl_common.h"
#include "fsl_debug_console.h"
#include "fsl_port.h"
#ifndef BOOTLOADER
#include "fsl_dspi_freertos.h"
#endif
#include "fsl_dspi.h"
#include "fsl_lptmr.h"
//#include "event_groups.h"
#include "board.h"

#if (!defined DRIVER_RF_PROJECT)&&(!defined BOOTLOADER)
#include "uart_ppp.h"
#include "uplink_protocol.h"
#include "fota.h"
#endif

#ifndef BOOTLOADER
#ifdef WIFI_MODULE
#include "wifi_module.h"

extern uint32_t wifi_key_press_flag;
#endif
#endif

/* Initialize debug console. */
void BOARD_InitDebugConsole(void)
{
    uint32_t uartClkSrcFreq = BOARD_DEBUG_UART_CLK_FREQ;
    DbgConsole_Init(BOARD_DEBUG_UART_BASEADDR, BOARD_DEBUG_UART_BAUDRATE, BOARD_DEBUG_UART_TYPE, uartClkSrcFreq);
}

void ms_delay(uint32_t msec)
{
    volatile uint32_t i = 0;
    for (i = 0; i < 10000*msec; ++i)
    {
        __asm("NOP"); /* delay */
    }
}

void us_delay(uint32_t usec)
{
    volatile uint32_t i = 0;
    for (i = 0; i < 10*usec; ++i)
    {
        __asm("NOP"); /* delay */
    }
}

#if (!defined DRIVER_RF_PROJECT)&&(!defined BOOTLOADER)
extern u32_t send_bat_status(uint32_t status,int p,char *data_str);


void BQ25050_MaxOutputSet(void)
{
    CHG_CTRL_INIT(LOGIC_LOW);
    ms_delay(5);    //tCTRL-DGL
    ms_delay(32);   //tWAIT

    for (uint32_t i=0; i<11; i++)
    {
        us_delay(100);
        CHG_CTRL_HIGH();
        us_delay(100);
        CHG_CTRL_LOW();
    }

    ms_delay(2);    //tCTRL-LATCH
}

/*
*weitao.yang
*2017.02.04
*irq for external power supply
*/
void BQ25050_nPG_Irq_Init()
{
	PORT_SetPinInterruptConfig(BAT_nPG_PORT, BAT_nPG_GPIO_PIN, kPORT_InterruptRisingEdge);
	NVIC_SetPriority(BAT_nPG_IRQ, 7);
}

/*
*hao.yunran
*2016.11.29
*irq for apn update by sms
*/
void Modem_Ring_Sms_Irq_Init()
{
	PORT_SetPinInterruptConfig(MODEM_RING_SMS_PORT, MODEM_RING_SMS_GPIO_PIN, kPORT_InterruptFallingEdge);
	NVIC_SetPriority(MODEM_RING_SMS_IRQ, 7);
	//EnableIRQ(MODEM_RING_SMS_IRQ);
}

/*
*hao.yunran
*2016.11.29
*irq for apn update by sms
*/
void Power_HighV_Irq_Init()
{
	PORT_SetPinInterruptConfig(OVERVOLTAGE_PORT, OVERVOLTAGE_GPIO_PIN, kPORT_InterruptRisingEdge);
	NVIC_SetPriority(OVERVOLTAGE_IRQ, 7);
	EnableIRQ(OVERVOLTAGE_IRQ);
}

/*
*hao.yunran
*2016.11.29
*irq for apn update by sms
*/
void Modem_SOS_Irq_Init()
{
	PORT_SetPinInterruptConfig(MODEM_SOS_PORT, MODEM_SOS_GPIO_PIN, kPORT_InterruptFallingEdge);
	NVIC_SetPriority(MODEM_SOS_IRQ, 7);
	//EnableIRQ(MODEM_SOS_IRQ);
}

/*
*hao.yunran
*2016.11.29
*irq for apn update by sms
*/
void Power_LowV_Init()
{
    lptmr_config_t lptmrConfig;
	
    /* Configure LPTMR */
    /*
     * lptmrConfig.timerMode = kLPTMR_TimerModeTimeCounter;
     * lptmrConfig.pinSelect = kLPTMR_PinSelectInput_0;
     * lptmrConfig.pinPolarity = kLPTMR_PinPolarityActiveHigh;
     * lptmrConfig.enableFreeRunning = false;
     * lptmrConfig.bypassPrescaler = true;
     * lptmrConfig.prescalerClockSource = kLPTMR_PrescalerClock_1;
     * lptmrConfig.value = kLPTMR_Prescale_Glitch_0;
     */
    LPTMR_GetDefaultConfig(&lptmrConfig);

    /* Initialize the LPTMR */
    LPTMR_Init(LPTMR0, &lptmrConfig);

    /* Set timer period */
    LPTMR_SetTimerPeriod(LPTMR0, USEC_TO_COUNT(LPTMR_USEC_COUNT, LPTMR_SOURCE_CLOCK));

    /* Enable timer interrupt */
    LPTMR_EnableInterrupts(LPTMR0, kLPTMR_TimerInterruptEnable);

    /* Enable at the NVIC */
    EnableIRQ(LPTMR0_IRQn);

    PRINTF("..Low Power Timer Init..\r\n");
	
}

/*
*hao.yunran
*2016.11.29
*irq for apn update by sms
*/
uint16_t Power_LowV_Read()
{
	uint16_t currentCNR = 0U;
	LPTMR_StartTimer(LPTMR0);

	currentCNR = LPTMR_GetCurrentTimerCount(LPTMR0);
	
//	PRINTF("...........LPTMR currentCNR Num%d..CMR=0x%x.. TCF=0x%x\r\n", currentCNR, LPTMR0->CMR, (LPTMR0->CSR&LPTMR_CSR_TCF_MASK));

	return currentCNR;
}

#define B_POWER_KEY 		(1 << 3)
extern EventGroupHandle_t PowerKeyEventGroup;
void PORTE_IRQHandler(void)
{
	BaseType_t xHigherPriorityTaskWoken, xResult;
	xHigherPriorityTaskWoken = pdFALSE;
	PRINTF("0x%x, %d\r\n", GPIO_GetPinsInterruptFlags(POWER_KEY_GPIO), POWER_KEY_READ());
	if(GPIO_GetPinsInterruptFlags(POWER_KEY_GPIO) == (1U << POWER_KEY_GPIO_PIN))
	{
		GPIO_ClearPinsInterruptFlags(POWER_KEY_GPIO, (1U <<POWER_KEY_GPIO_PIN));
    	xResult = xEventGroupSetBitsFromISR(PowerKeyEventGroup, B_POWER_KEY, &xHigherPriorityTaskWoken );
    	if( xResult != pdFAIL )
    	{
    		portYIELD_FROM_ISR( xHigherPriorityTaskWoken );
    	}
	}
}


/*
*hao.yunran
*2016.11.15
*irq for apn update by sms
*/
void PORTB_IRQHandler(void)
{
	uint32_t irq_flag=0;
	uint32_t nPG_value=0;
	ppp_gprs_rq_msg_conf ppp_gprs_rq_msg;
	BaseType_t xHigherPriorityTaskWoken = pdFALSE;

	irq_flag = GPIO_GetPinsInterruptFlags(MODEM_RING_SMS_GPIO);
	GPIO_ClearPinsInterruptFlags(MODEM_RING_SMS_GPIO, 1U << MODEM_RING_SMS_GPIO_PIN);
	PRINTF(".irq.SMS.Ring.0x%x.%d.%d.%d.\r\n",irq_flag,ppp_gprs_info.ppp_modem_init_flag,ppp_gprs_info.irq_for_sms_or_incoming_call,ppp_gprs_info.irq_for_sos_call );

	if(irq_flag == 0x2000)
	{	
		if(ppp_gprs_info.ppp_modem_init_flag == 1)
		{

		}
		else
		{	
			GPIO_ClearPinsInterruptFlags(MODEM_RING_SMS_GPIO, 1U << MODEM_RING_SMS_GPIO_PIN);
			DisableIRQ(MODEM_RING_SMS_IRQ);
			GPIO_ClearPinsInterruptFlags(MODEM_SOS_GPIO, 1U << MODEM_SOS_GPIO_PIN);
			DisableIRQ(MODEM_SOS_IRQ);
			
			if(ppp_gprs_info.irq_for_sms_or_incoming_call == NO_SMS_OR_INCOMING_CALL)
			{
				ppp_gprs_info.irq_for_sms_or_incoming_call = SMS_OR_INCOMING_CALL;
				PRINTF(".SMS.or.incoming_call.irq.\r\n");
			}
		}
	}

	if (irq_flag == 0x100000)
	{
		GPIO_ClearPinsInterruptFlags(BAT_nPG_GPIO, 1U << BAT_nPG_GPIO_PIN);
		nPG_value = BAT_nPG_READ();
		if(nPG_value == 1)
		{
			PRINTF("External power supply has been inserted!\r\n");
		}
		else if(nPG_value == 0)
		{
			PRINTF("External power supply has been removed!\r\n");
		}
		BQ25050_MaxOutputSet();
	}
}


#ifdef WIFI_MODULE
/*
*hao.yunran
*2017.3.20
*irq for sos call by sos key
*/
void PORTA_IRQHandler(void)
{
	uint32_t irq_flag=0;
	char ac_low_v_data[12]="s:1/t:200";
	irq_flag = GPIO_GetPinsInterruptFlags(MODEM_SOS_GPIO);

	GPIO_ClearPinsInterruptFlags(MODEM_SOS_GPIO, 1U << MODEM_SOS_GPIO_PIN);
	PRINTF(".irq.SOS.0x%x.\r\n",irq_flag);
	if(irq_flag == 0x20000000)
	{
		wifi_key_press_flag = WIFI_KEY_PRESS;

	}
}
#else
/*
*hao.yunran
*2016.11.15A
*irq for sos call by sos key
*/
//extern TaskHandle_t ppp_uart_task_handle;
void PORTA_IRQHandler(void)
{
	uint32_t irq_flag=0;
	char ac_low_v_data[12]="s:1/t:200";
	irq_flag = GPIO_GetPinsInterruptFlags(MODEM_SOS_GPIO);

	GPIO_ClearPinsInterruptFlags(MODEM_SOS_GPIO, 1U << MODEM_SOS_GPIO_PIN);
	PRINTF(".irq.SOS.0x%x.%d.%d.%d.\r\n",irq_flag,ppp_gprs_info.ppp_modem_init_flag,ppp_gprs_info.irq_for_sms_or_incoming_call,ppp_gprs_info.irq_for_sos_call);
	if(irq_flag == 0x20000000)
	{
		//GPIO_ClearPinsInterruptFlags(MODEM_RING_SMS_GPIO, 1U << MODEM_RING_SMS_GPIO_PIN);
		DisableIRQ(MODEM_RING_SMS_IRQ);
		//GPIO_ClearPinsInterruptFlags(MODEM_SOS_GPIO, 1U << MODEM_SOS_GPIO_PIN);
		//DisableIRQ(MODEM_SOS_IRQ);
		switch(ppp_gprs_info.irq_for_sos_call)
		{
			case SOS_NO_CALL:
				PPP_RED_LED_ON();
				PPP_GREEN_LED_OFF();
				ppp_gprs_info.irq_for_sos_call = SOS_CALL_IRQ;
				
				PRINTF(".sos_call.irq.\r\n");
				//GPIO_ClearPinsInterruptFlags(MODEM_SOS_GPIO, 1U << MODEM_SOS_GPIO_PIN);
				//EnableIRQ(MODEM_SOS_IRQ);
				break;
			case SOS_CALL_HAND_UP:
				PPP_RED_LED_ON();
				PPP_GREEN_LED_OFF();
				ppp_gprs_info.irq_for_sos_call = SOS_CALL_IRQ;
				PRINTF(".sos_call.irq..01\r\n");
				//GPIO_ClearPinsInterruptFlags(MODEM_SOS_GPIO, 1U << MODEM_SOS_GPIO_PIN);
				//EnableIRQ(MODEM_SOS_IRQ);
				break;
			case SOS_CALL_IRQ:
				PPP_RED_LED_OFF();
				PPP_GREEN_LED_ON();
				ppp_gprs_info.irq_for_sos_call = SOS_CALL_HAND_UP;
				PRINTF(".sos_call_hand_up.irq.\r\n");
				//GPIO_ClearPinsInterruptFlags(MODEM_SOS_GPIO, 1U << MODEM_SOS_GPIO_PIN);
				//EnableIRQ(MODEM_SOS_IRQ);
				break;
		}
		//GPIO_ClearPinsInterruptFlags(MODEM_SOS_GPIO, 1U << MODEM_SOS_GPIO_PIN);
	}
	if(irq_flag == 0x40000)
	{
		GPIO_ClearPinsInterruptFlags(OVERVOLTAGE_GPIO, 1U << OVERVOLTAGE_GPIO_PIN);
		send_bat_status(WARNING,6,ac_low_v_data);
	}


}
#endif
/*
*hao.yunran
*2016.11.29
*irq for apn update by sms
*/
void LPTMR0_IRQHandler(void)
{
    LPTMR_ClearStatusFlags(LPTMR0, kLPTMR_TimerCompareFlag);
	//PRINTF("................LPTMR0_IRQHandler..\r\n");
}




#endif

