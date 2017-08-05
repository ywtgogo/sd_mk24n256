/*
* Copyright (c) 2013 - 2015, Freescale Semiconductor, Inc.
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

/* FreeRTOS kernel includes. */
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "timers.h"
#include "event_groups.h"

/* Freescale includes. */
#include "fsl_device_registers.h"
#include "fsl_debug_console.h"
#include "fsl_dspi.h"
#include "fsl_dspi_freertos.h"
#include "board.h"

#include "fsl_common.h"
#include "pin_mux.h"
#include "clock_config.h"

#include "MCU_Interface.h"
//#include "A7139.h"
//#include "SPI_A7139.h"

#include "hb_protocol.h"
/*******************************************************************************
* Definitions
******************************************************************************/

#if (EXAMPLE_DSPI_MASTER_BASE == SPI0_BASE)
#define DSPI_MASTER_CLK_SRC (DSPI0_CLK_SRC)
#elif(EXAMPLE_DSPI_MASTER_BASE == SPI1_BASE)
#define DSPI_MASTER_CLK_SRC (DSPI1_CLK_SRC)
#elif(EXAMPLE_DSPI_MASTER_BASE == SPI2_BASE)
#define DSPI_MASTER_CLK_SRC (DSPI2_CLK_SRC)
#elif(EXAMPLE_DSPI_MASTER_BASE == SPI3_BASE)
#define DSPI_MASTER_CLK_SRC (DSPI3_CLK_SRC)
#elif(EXAMPLE_DSPI_MASTER_BASE == SPI4_BASE)
#define DSPI_MASTER_CLK_SRC (DSPI4_CLK_SRC)
#else
#error Should define the DSPI_MASTER_CLK_SRC!
#endif

/*******************************************************************************
* Variables
******************************************************************************/
SemaphoreHandle_t dspi_sem;
extern dspi_rtos_handle_t master_rtos_handle;

extern void SPI_A7139_INIT(void);
void status_spirit_task(void *p);
/*******************************************************************************
 * Definitions
 ******************************************************************************/

/*******************************************************************************
 * Prototypes
 ******************************************************************************/

/*******************************************************************************
 * Code
 ******************************************************************************/

static void DelayBlinkRfCallback(TimerHandle_t xTimer)
{	
    GPIO_ClearPinsOutput(GPIOD, 1U << 14);
    vTaskDelay(10);
    GPIO_SetPinsOutput(GPIOD, 1U << 14);
}


void rf_get_message_task(void *pvParameters)
{
    spirit1_main();
    while(1);

}

/*!
 * @brief Application entry point.
 */

int main(void)
{
    /* Init board hardware. */
    BOARD_InitPins();
    BOARD_BootClockRUN();
    BOARD_InitDebugConsole(); 

//    PRINTF("Bronze_HW_test RF Send/Recv start.\r\n");
//    PRINTF("It will Recv two pakgs, and then Send two pakg, cycle...\r\n");
//    PRINTF("Please make sure you make the correct line connection. Basically, the connection is: \r\n");
//    PRINTF("DSPI_master -- A7139_slave   \r\n");
    
    if (xTaskCreate(status_spirit_task, "Status_spirit_task", configMINIMAL_STACK_SIZE*2, NULL, configMAX_PRIORITIES - 1, NULL) !=
        pdPASS)
    {
        PRINTF("Failed to create master task");
        vTaskSuspend(NULL);
    }
    
    if (xTaskCreate(rf_get_message_task, "Rf_get_message_task", configMINIMAL_STACK_SIZE*10, NULL, configMAX_PRIORITIES - 2, NULL) !=
        pdPASS)
    {
        PRINTF("Failed to create master task");
        vTaskSuspend(NULL);
    }
    
    vTaskStartScheduler();
    for (;;)
        ;
}

