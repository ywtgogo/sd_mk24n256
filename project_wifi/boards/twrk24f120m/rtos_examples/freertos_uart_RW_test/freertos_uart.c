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

/* FreeRTOS kernel includes. */
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "timers.h"

/* Freescale includes. */
#include "fsl_device_registers.h"
#include "fsl_debug_console.h"
#include "board.h"

#include "fsl_uart_freertos.h"
#include "fsl_uart.h"

#include "pin_mux.h"
#include "clock_config.h"
#include "uplink_KM.h"
/*******************************************************************************
 * Definitions
 ******************************************************************************/
/* PPP-UART instance and clock */
#define PPP_UART UART1
#define PPP_UART_CLKSRC UART1_CLK_SRC
#define PPP_UART_RX_TX_IRQn UART1_RX_TX_IRQn
/* PLC-UART instance and clock */
#define PLC_UART UART2
#define PLC_UART_CLKSRC UART2_CLK_SRC
#define PLC_UART_RX_TX_IRQn UART2_RX_TX_IRQn
/* MM-UART instance and clock */
#define MM_UART UART3
#define MM_UART_CLKSRC UART3_CLK_SRC
#define MM_UART_RX_TX_IRQn UART3_RX_TX_IRQn

/* Task priorities. */
#define ppp_uart_task_PRIORITY (configMAX_PRIORITIES - 1)
#define plc_uart_task_PRIORITY (configMAX_PRIORITIES - 2)
#define  mm_uart_task_PRIORITY (configMAX_PRIORITIES - 3)
/*******************************************************************************
 * Prototypes
 ******************************************************************************/

/*******************************************************************************
 * Variables
 ******************************************************************************/
const char *to_send = "Hello, world!\r\n";
const char *send_ring_overrun = "\r\nRing buffer overrun!\r\n";
const char *send_hardware_overrun = "\r\nHardware buffer overrun!\r\n";

uart_rtos_handle_t ppp_handle;
struct _uart_handle ppp_t_handle;
uart_rtos_handle_t plc_handle;
struct _uart_handle plc_t_handle;
uart_rtos_handle_t mm_handle;
struct _uart_handle mm_t_handle;

/*******************************************************************************
 * Code
 ******************************************************************************/

/*!
 * @brief Task responsible for printing of "Hello world." message.
 */
static void ppp_uart_task(void *pvParameters)
{
    int error;
    size_t n;
    uint8_t background_buffer[32];
    uint8_t recv_buffer[1];
    
    struct rtos_uart_config uart_config = {
        .baudrate = 115200,
        .parity = kUART_ParityDisabled,
        .stopbits = kUART_OneStopBit,
        .buffer = background_buffer,
        .buffer_size = sizeof(background_buffer),
    };    
   
    uart_config.srcclk = CLOCK_GetFreq(PPP_UART_CLKSRC);
    uart_config.base = PPP_UART;

    PRINTF("Test PPP\r\n");

    if (0 > UART_RTOS_Init(&ppp_handle, &ppp_t_handle, &uart_config))
    {
        PRINTF("Error during UART initialization.\r\n");
        vTaskSuspend(NULL);
    }

    /* Send some data */
    if (0 > UART_RTOS_Send(&ppp_handle, (uint8_t *)to_send, strlen(to_send)))
    {
        PRINTF("Error during UART send.\r\n");
        vTaskSuspend(NULL);
    }
    
    /* Send data */
    do
    {
        error = UART_RTOS_Receive(&ppp_handle, recv_buffer, sizeof(recv_buffer), &n);
        if (error == kStatus_UART_RxHardwareOverrun)
        {
            /* Notify about hardware buffer overrun */
            if (kStatus_Success != UART_RTOS_Send(&ppp_handle, (uint8_t *)send_hardware_overrun, strlen(send_hardware_overrun)))
            {
                vTaskSuspend(NULL);
            }
        }
        if (error == kStatus_UART_RxRingBufferOverrun)
        {
            /* Notify about ring buffer overrun */
            if (kStatus_Success != UART_RTOS_Send(&ppp_handle, (uint8_t *)send_ring_overrun, strlen(send_ring_overrun)))
            {
                vTaskSuspend(NULL);
            }
        }
        if (n > 0)
        {
            /* send back the received data */
            UART_RTOS_Send(&ppp_handle, (uint8_t *)recv_buffer, n);
        }
    } while (kStatus_Success == error);

    UART_RTOS_Deinit(&ppp_handle);
    
    vTaskSuspend(NULL);
}

static void plc_uart_task(void *pvParameters)
{
    int error;
    size_t n;
    uint8_t background_buffer[32];
    uint8_t recv_buffer[1];
    
    struct rtos_uart_config uart_config = {
        .baudrate = 115200,
        .parity = kUART_ParityDisabled,
        .stopbits = kUART_OneStopBit,
        .buffer = background_buffer,
        .buffer_size = sizeof(background_buffer),
    };
   
    uart_config.srcclk = CLOCK_GetFreq(PLC_UART_CLKSRC);
    uart_config.base = PLC_UART;

    PRINTF("Test PLC\r\n");

    if (0 > UART_RTOS_Init(&plc_handle, &plc_t_handle, &uart_config))
    {
        PRINTF("Error during UART initialization.\r\n");
        vTaskSuspend(NULL);
    }

    /* Send some data */
    if (0 > UART_RTOS_Send(&plc_handle, (uint8_t *)to_send, strlen(to_send)))
    {
        PRINTF("Error during UART send.\r\n");
        vTaskSuspend(NULL);
    }
    
    /* Send data */
    do
    {
        error = UART_RTOS_Receive(&plc_handle, recv_buffer, sizeof(recv_buffer), &n);
        if (error == kStatus_UART_RxHardwareOverrun)
        {
            /* Notify about hardware buffer overrun */
            if (kStatus_Success != UART_RTOS_Send(&plc_handle, (uint8_t *)send_hardware_overrun, strlen(send_hardware_overrun)))
            {
                vTaskSuspend(NULL);
            }
        }
        if (error == kStatus_UART_RxRingBufferOverrun)
        {
            /* Notify about ring buffer overrun */
            if (kStatus_Success != UART_RTOS_Send(&plc_handle, (uint8_t *)send_ring_overrun, strlen(send_ring_overrun)))
            {
                vTaskSuspend(NULL);
            }
        }
        if (n > 0)
        {
            /* send back the received data */
            UART_RTOS_Send(&plc_handle, (uint8_t *)recv_buffer, n);
        }
    } while (kStatus_Success == error);

    UART_RTOS_Deinit(&plc_handle);
    
    vTaskSuspend(NULL);
}

static void mm_uart_task(void *pvParameters)
{
    int error;
    size_t n;
    uint8_t background_buffer[32];
    uint8_t recv_buffer[1];
    
    struct rtos_uart_config uart_config = {
        .baudrate = 115200,
        .parity = kUART_ParityDisabled,
        .stopbits = kUART_OneStopBit,
        .buffer = background_buffer,
        .buffer_size = sizeof(background_buffer),
    };
   
    uart_config.srcclk = CLOCK_GetFreq(MM_UART_CLKSRC);
    uart_config.base = MM_UART;

    PRINTF("Test MM\r\n");

    if (0 > UART_RTOS_Init(&mm_handle, &mm_t_handle, &uart_config))
    {
        PRINTF("Error during UART initialization.\r\n");
        vTaskSuspend(NULL);
    }

    /* Send some data */
    if (0 > UART_RTOS_Send(&mm_handle, (uint8_t *)to_send, strlen(to_send)))
    {
        PRINTF("Error during UART send.\r\n");
        vTaskSuspend(NULL);
    }
    
    /* Send data */
    do
    {
        error = UART_RTOS_Receive(&mm_handle, recv_buffer, sizeof(recv_buffer), &n);
        if (error == kStatus_UART_RxHardwareOverrun)
        {
            /* Notify about hardware buffer overrun */
            if (kStatus_Success != UART_RTOS_Send(&mm_handle, (uint8_t *)send_hardware_overrun, strlen(send_hardware_overrun)))
            {
                vTaskSuspend(NULL);
            }
        }
        if (error == kStatus_UART_RxRingBufferOverrun)
        {
            /* Notify about ring buffer overrun */
            if (kStatus_Success != UART_RTOS_Send(&mm_handle, (uint8_t *)send_ring_overrun, strlen(send_ring_overrun)))
            {
                vTaskSuspend(NULL);
            }
        }
        if (n > 0)
        {
            /* send back the received data */
            UART_RTOS_Send(&mm_handle, (uint8_t *)recv_buffer, n);
        }
    } while (kStatus_Success == error);

    UART_RTOS_Deinit(&mm_handle);
    
    vTaskSuspend(NULL);
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
    NVIC_SetPriority(PPP_UART_RX_TX_IRQn, 6);
    NVIC_SetPriority(PLC_UART_RX_TX_IRQn, 6);
    NVIC_SetPriority( MM_UART_RX_TX_IRQn, 6);

    xTaskCreate(ppp_uart_task, "Ppp_uart_task", configMINIMAL_STACK_SIZE, NULL, ppp_uart_task_PRIORITY, NULL);

    xTaskCreate(plc_uart_task, "Plc_uart_task", configMINIMAL_STACK_SIZE, NULL, plc_uart_task_PRIORITY, NULL);
    
    xTaskCreate( mm_uart_task,  "Mm_uart_task", configMINIMAL_STACK_SIZE, NULL,  mm_uart_task_PRIORITY, NULL);
    
    vTaskStartScheduler();
    for (;;)
        ;
}