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

/*******************************************************************************
 * Includes
 ******************************************************************************/
#include "FreeRTOS.h"
#include "fsl_debug_console.h"
#include "board.h"
#include "task.h"
#include "fsl_wdog.h"
#include "fsl_port.h"
#include "fsl_rcm.h"
#include "pin_mux.h"
#include "clock_config.h"
#include "hb_protocol.h"
#include "spiflash_config.h"
#include "supervisor.h"
#include "uart_ppp.h"
#include "log_task.h"
#ifdef WIFI_MODULE
#include "wifi_module.h"
#endif
/*******************************************************************************
 * Definitions
 ******************************************************************************/
#define WDOG_WCT_INSTRUCITON_COUNT (256U)

POST_STATUS_t post_status = {
	.a7139_check	= false,
	.nor_flash_check= true,
	.plc_check		= false,
	.wifi_check		= false,
	.zigbee_check	= false,
};

PERIPHERALS_COUNT_t	PeripheralsCount = {
	.Plc 	= 0,
	.Wifi 	= 0,
	.Km		= 0,
	.NorFlash	= 0,
	.A7139	= 0,
	.Zigbee	= 0,
};

uint32_t	reset_reason = 0;


void Increase_PeripheralsCntr(uint32_t Index)
{
    switch (Index)
    {
    case Peripherals_Plc:
    	PeripheralsCount.Plc++;
    	break;
    case Peripherals_Wifi:
    	PeripheralsCount.Wifi++;
    	break;
    case Peripherals_Km:
    	PeripheralsCount.Km++;
    	break;
    case Peripherals_NorFlash:
    	PeripheralsCount.NorFlash++;
    	break;
    case Peripherals_A7139:
    	PeripheralsCount.A7139++;
    	break;
    case Peripherals_Zigbee:
    	PeripheralsCount.Zigbee++;
    	break;
    default:
    	break;
    };
}

uint32_t Get_PeripheralsIncreasedCntr(uint32_t Index)
{
    switch (Index)
    {
    case Peripherals_Plc:
    	return PeripheralsCount.Plc;
    case Peripherals_Wifi:
    	return PeripheralsCount.Wifi;
    case Peripherals_Km:
    	return PeripheralsCount.Km;
    case Peripherals_NorFlash:
    	return PeripheralsCount.NorFlash;
    case Peripherals_A7139:
    	return PeripheralsCount.A7139;
    case Peripherals_Zigbee:
    	return PeripheralsCount.Zigbee;
    default:
    	return 0;
    };
}

static void Increase_StatusCheck()
{
	PERIPHERALS_COUNT_t		pre_peripherals_count = {0};

	if (pre_peripherals_count.A7139	== PeripheralsCount.A7139) {
		PRINTF("A7139 NO DATA STREAM\r\n");
		hbb_send_uplink_data(hbb_info.config.id, 30,  6, "%d", RF_ERROR);
	}
	if (pre_peripherals_count.NorFlash == PeripheralsCount.NorFlash) {
		PRINTF("NorFlash NO DATA STREAM\r\n");
		hbb_send_uplink_data(hbb_info.config.id, 30,  6, "%d", SPI_FLASH_ERROR);
	}
	if (pre_peripherals_count.Plc == PeripheralsCount.Plc) {
		PRINTF("Plc NO DATA STREAM\r\n");
		hbb_send_uplink_data(hbb_info.config.id, 30,  6, "%d", PLC_ERROR);
	}
	if (pre_peripherals_count.Wifi == PeripheralsCount.Wifi) {
		PRINTF("Wifi NO DATA STREAM\r\n");
		hbb_send_uplink_data(hbb_info.config.id, 30,  6, "%d", WIFI_ERROR);
	}
	if (pre_peripherals_count.Km == PeripheralsCount.Km) {
		PRINTF("Km NO DATA STREAM\r\n");
		//hbb_send_uplink_data(hbb_info.config.id, 30,  6, "%d", SPI_FLASH_ERROR);
	}
	if (pre_peripherals_count.Zigbee == PeripheralsCount.Zigbee) {
		PRINTF("Zigbee NO DATA STREAM\r\n");
		hbb_send_uplink_data(hbb_info.config.id, 30,  6, "%d", ZIGBEE_ERROR);
	}
	memcpy(&pre_peripherals_count, &PeripheralsCount, sizeof(PERIPHERALS_COUNT_t));
}

void InitCheckPeripherals(void)
{
	uint32_t    id;
	/* spi flash error check */
    if (!read_ID(&flash_master_rtos_handle, &id))
    {
    	print_log("NorFlash Read ID Fialed\r\n");
    	post_status.nor_flash_check = false;
    }
    /* rf check in func init_A7139() */
    /* wifi check in func  */
    /* PLC check in func */
    vTaskDelay(3000);

    for (uint32_t i=0; i<2; i++) {
    	//PRINTF("\r\npost_status:%d", *((uint8_t*)&post_status+i));
    	if (false == *((uint8_t*)&post_status+i)) {
    		/* detected error ph, send to server */
    		hbb_send_uplink_data(hbb_info.config.id, 30,  6, "%d", 30002+i);
    	}
    }
}

static void HbbResetReasonCheck(WDOG_Type *wdog_base)
{
    RCM_Type 	*rcm_base = RCM;
    uint32_t 	reset_source = 0;
    reset_source = RCM_GetPreviousResetSources(rcm_base);
    if (reset_source& kRCM_SourceWdog) {
    	PRINTF("WDOG RESET!!! %d\r\n", WDOG_GetResetCount(wdog_base));
        WDOG_ClearResetCount(wdog_base);
        reset_reason = 10103;
    } else if (reset_source & kRCM_SourcePor) {
    	PRINTF("Power on RESET!!! \r\n");
    	reset_reason = 10106;
    } else if (reset_source & kRCM_SourcePin) {
    	PRINTF("External pin RESET!!! \r\n");
    	reset_reason = 10104;
    } else if (reset_source & kRCM_SourceSw) {
    	PRINTF("Software RESET!!! \r\n");
    	reset_reason = 10105;
    	/* if fota, 10101 */
    	if ((0xffffffff != READ32((uint32_t*)(FOTA_PRAM_ADDR +(sizeof(hbb_config)|0x07)+1))) && (0x0 != READ32((uint32_t*)(FOTA_PRAM_ADDR +(sizeof(hbb_config)|0x07)+1))))
    		reset_reason = 10101;
    } else if (reset_source & kRCM_SourceLvd) {
    	PRINTF("Low-voltage RESET!!! \r\n");
    	reset_reason = 10102;
    } else {
    	PRINTF("RESET!!! reson 0x%x\r\n", reset_source);
    }
}

static void WaitWctClose(WDOG_Type *base)
{
    /* Accessing register by bus clock */
    for (uint32_t i = 0; i < WDOG_WCT_INSTRUCITON_COUNT; i++)
    {
        (void)base->RSTCNT;
    }
}

/*!
 * @brief Gets the Watchdog timer output.
 *
 * @param base WDOG peripheral base address
 * @return Current value of watchdog timer counter.
 */
static inline uint32_t GetTimerOutputValue(WDOG_Type *base)
{
    return (uint32_t)((((uint32_t)base->TMROUTH) << 16U) | (base->TMROUTL));
}

int check_last_tick(TickType_t last, TickType_t now, uint32_t ms)
{
	TickType_t		elapse;

	elapse = (now - last) * portTICK_PERIOD_MS;

	if (elapse <= ms)
		return 1;
	else
		return 0;
}

void WatchdogTask( void *pvParameters )
{
	TickType_t	xLastWakeTime;
    wdog_config_t config;
    WDOG_Type 	*wdog_base = WDOG;

    WDOG_GetDefaultConfig(&config);
    config.timeoutValue = WDOG_RESET_INTERVAL * 2;
    WDOG_Init(wdog_base, &config);
    WaitWctClose(wdog_base);

    HbbResetReasonCheck(wdog_base);

 	/* Get currrent tick count */
	xLastWakeTime = xTaskGetTickCount();

	for( ;; )
	{
		/* Get currrent tick count */
		xLastWakeTime = xTaskGetTickCount();

		if (!check_last_tick(hbb_info.hb_tick, xLastWakeTime, hbb_info.config.heartbeat_interval * 2))
		{
			PRINTF("heartbeat missed %u %u %u\r\n", hbb_info.hb_tick, xLastWakeTime, hbb_info.config.heartbeat_interval * 2);
			continue;
		}

		if (ppp_gprs_info.ppp_gprs_current_status == K24_STATUS_DEAD)
			continue;
		if (ppp_gprs_info.ppp_uart_current_status == K24_UART_STATUS_DEAD)
			continue;
		#ifdef WIFI_MODULE
		if(wifi_uart_current_status == K24_UART_STATUS_DEAD)
			continue;
		#endif

        WDOG_Refresh(wdog_base);

		/* Block the task for WTC_CLR_PER ticks (1s) at watchdog overflow period of WTC_PER_2_23 CLKMC cycles */
		vTaskDelayUntil( &xLastWakeTime, WDOG_RESET_INTERVAL );

		//Increase_StatusCheck();
	}
}

enum { r0, r1, r2, r3, r12, lr, pc, psr};

void HardFault_Handler(void)
{
	uint32_t *stack = (uint32_t *)__get_PSP();

	PRINTF("r0  = 0x%x\r\n", stack[r0]);
	PRINTF("r1  = 0x%x\r\n", stack[r1]);
	PRINTF("r2  = 0x%x\r\n", stack[r2]);
	PRINTF("r3  = 0x%x\r\n", stack[r3]);
	PRINTF("r12 = 0x%x\r\n", stack[r12]);
	PRINTF("lr  = 0x%x\r\n", stack[lr]);
	PRINTF("pc  = 0x%x\r\n", stack[pc]);
	PRINTF("psr = 0x%x\r\n", stack[psr]);

	while(1);
}

void power_key_task(void *pvParameters)
{
	PORT_SetPinInterruptConfig(POWER_KEY_PORT, POWER_KEY_GPIO_PIN, kPORT_InterruptFallingEdge);
	NVIC_SetPriority(POWER_KEY_IRQ, 7);
	EnableIRQ(POWER_KEY_IRQ);
	POWER_KEY_INIT();
	while(1)
	{
		/* wait event */
    	xEventGroupWaitBits(PowerKeyEventGroup,    	/* The event group handle. */
    						B_POWER_KEY,			/* The bit pattern the event group is waiting for. */
							pdTRUE,         		/* BIT will be cleared automatically. */
							pdTRUE,         		/* Don't wait for both bits, either bit unblock task. */
							portMAX_DELAY); 		/* Block indefinitely to wait for the condition to be met. */
		/* save log */
    	PR_INFO("power off detect\r\n");

		vTaskDelay(4000);
		if (POWER_KEY_READ()){
			/* erase log */
			PR_INFO("power off no effect\r\n");
		}
		vTaskDelay(1000);
		//GPIO_ClearPinsInterruptFlags(POWER_KEY_GPIO, 1<<POWER_KEY_GPIO_PIN);
		//EnableIRQ(POWER_KEY_IRQ);
	}
}


#ifdef WDOG_TEST
int wdog_test(void)
{
    uint16_t wdog_reset_count = 0;
    wdog_test_config_t test_config;
    wdog_config_t config;

    /*If not wdog reset*/
    if (!(RCM_GetPreviousResetSources(rcm_base) & kRCM_SourceWdog))
    {
        WDOG_ClearResetCount(wdog_base);
    }
    wdog_reset_count = WDOG_GetResetCount(wdog_base);
    if (wdog_reset_count == 0)
    {
        /*quick test*/
        test_config.testMode = kWDOG_QuickTest;
        test_config.timeoutValue = 0xfffffu;
        /*Not necessary to configure tested byte for quick test, just to get rid of using uninitialized value check*/
        test_config.testedByte = kWDOG_TestByte0;
        PRINTF("\r\n--- Quick test ---\r\n");
        WDOG_SetTestModeConfig(wdog_base, &test_config);
        WaitWctClose(wdog_base);

        /*wait for timeout reset*/
        while (1)
        {
        }
    }
    else if (wdog_reset_count == 1)
    {
        PRINTF("--- Quick test done ---\r\n");
    /*
     * config.enableWdog = true;
     * config.clockSource = kWDOG_LpoClockSource;
     * config.prescaler = kWDOG_ClockPrescalerDivide1;
     * config.enableUpdate = true;
     * config.enableInterrupt = false;
     * config.enableWindowMode = false;
     * config.windowValue = 0U;
     * config.timeoutValue = 0xFFFFU;
     */
        WDOG_GetDefaultConfig(&config);
        config.timeoutValue = 0x7ffU;
        /* wdog refresh test in none-window mode */
        PRINTF("\r\n--- None-window mode refresh test start---\r\n");
        WDOG_Init(wdog_base, &config);
        WaitWctClose(wdog_base);
        for (uint32_t i = 0; i < 10; i++)
        {
            WDOG_Refresh(wdog_base);
            PRINTF("--- Refresh wdog %d time ---\r\n", i + 1);

            while (GetTimerOutputValue(wdog_base) < (config.timeoutValue >> 3U))
            {
            }
        }
        /* wait for wdog timeout reset */
        while (1)
        {
        }
    }
    else if (wdog_reset_count == 2)
    {
        PRINTF("--- None-window mode refresh test done ---\r\n");
    /*
     * config.enableWdog = true;
     * config.clockSource = kWDOG_LpoClockSource;
     * config.prescaler = kWDOG_ClockPrescalerDivide1;
     * config.enableUpdate = true;
     * config.enableInterrupt = false;
     * config.enableWindowMode = false;
     * config.windowValue = 0U;
     * config.timeoutValue = 0xFFFFU;
     */
        WDOG_GetDefaultConfig(&config);
        config.timeoutValue = 0x7ffU;
        config.enableWindowMode = true;
        config.windowValue = 0x1ffU;
        /* wdog refresh test in window mode */
        PRINTF("\r\n--- Window mode refresh test start---\r\n");
        WDOG_Init(wdog_base, &config);
        WaitWctClose(wdog_base);

        while (GetTimerOutputValue(wdog_base) < config.windowValue)
        {
        }

        for (uint32_t i = 0; i < 10; i++)
        {
            WDOG_Refresh(wdog_base);
            PRINTF("--- Refresh wdog %d time ---\r\n", i + 1);
            while (GetTimerOutputValue(wdog_base) < config.windowValue)
            {
            }
        }
        WDOG_Refresh(wdog_base);
        /*Wait until refresh squence takes effect*/
        while (GetTimerOutputValue(wdog_base) >= config.windowValue)
        {
        }
        /*Refresh before window value to trigger reset*/
        WDOG_Refresh(wdog_base);
        /*If reset is not generated, you will see this log*/
        PRINTF("\r\n--- Reset failed---\r\n");
    }
    else
    {
        PRINTF("--- Window mode refresh test done---\r\n");
        PRINTF("\r\nEnd of Wdog example!\r\n");
    }
    while (1)
    {
    }
}
#endif
