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
#include "spiflash_config.h"
#include "wav_record.h"
/*******************************************************************************
 * Definitions
 ******************************************************************************/
#define DEMO_UART               UART5
#define DEMO_UART_CLKSRC        kCLOCK_BusClk
#define DEMO_UART_RX_TX_IRQn    UART5_RX_TX_IRQn
/* Task priorities. */
#define uart_task_PRIORITY (configMAX_PRIORITIES - 1)
/*******************************************************************************
 * Prototypes
 ******************************************************************************/
static void uart_task(void *pvParameters);

/*******************************************************************************
 * Variables
 ******************************************************************************/
const char *to_send = "FreeRTOS UART driver example!\r\n";
const char *send_ring_overrun = "\r\nRing buffer overrun!\r\n";
const char *send_hardware_overrun = "\r\nHardware buffer overrun!\r\n";
uint8_t background_buffer[32768];
uint8_t recv_buffer[4096];
uint8_t read_back[4096];
uint32_t sector_num;

uart_rtos_handle_t handle;
struct _uart_handle t_handle;

struct rtos_uart_config uart_config = {
    .baudrate = BOARD_DEBUG_UART_BAUDRATE,
    .parity = kUART_ParityDisabled,
    .stopbits = kUART_OneStopBit,
    .buffer = background_buffer,
    .buffer_size = sizeof(background_buffer),
};

/*******************************************************************************
 * Code
 ******************************************************************************/
/*!
 * @brief Application entry point.
 */
//__attribute(section(name))
 int main(void)
{
    /* Init board hardware. */
    BOARD_InitPins();
    BOARD_BootClockRUN();
    BOARD_InitDebugConsole();
    PRINTF("\r\n!!!\r\n");
    
    spi_flash_init();
    
    NVIC_SetPriority(DEMO_UART_RX_TX_IRQn, 5);

    xTaskCreate(uart_task, "Uart_task", configMINIMAL_STACK_SIZE*20, NULL, uart_task_PRIORITY, NULL);

    vTaskStartScheduler();
    for (;;)
        ;
}

/*!
 * @brief Task responsible for printing of "Hello world." message.
 */

uint8_t Audio_Source_Temp[1024];
static void uart_task(void *pvParameters)
{
    int error;
    size_t n;

    uart_config.srcclk = CLOCK_GetFreq(DEMO_UART_CLKSRC);
    uart_config.base = DEMO_UART;

    if (0 > UART_RTOS_Init(&handle, &t_handle, &uart_config))
    {
        vTaskSuspend(NULL);
    }
    
//    while(1)
//    {
//        spi_flash_test();
//        vTaskDelay(1000);
//    }
    
    PRINTF("\r\n!!!\r\n");
    /* Send some data */
    if (0 > UART_RTOS_Send(&handle, (uint8_t *)to_send, strlen(to_send)))
    {
        vTaskSuspend(NULL);
    }
    
    /* copy big_ben 8bit 8Khz to nor */
    norflash_chip_erase_ll(&flash_master_rtos_handle);
    norflash_read_data_ll (&flash_master_rtos_handle, 0x700000, 1024, Audio_Source_Temp);
    for (uint32_t j=0; j<1024; j++) {
        printf("0x%02x ", Audio_Source_Temp[j] );
    }
    uint8_t *p =(uint8_t *)wav_record_file;
    norflash_write_data_ll(&flash_master_rtos_handle, 0x700000, sizeof(wav_record_file), p);

    norflash_read_data_ll (&flash_master_rtos_handle, 0x700000, 1024, Audio_Source_Temp);
    for (uint32_t j=0; j<1024; j++) {
        printf("0x%02x ", Audio_Source_Temp[j] );
    }
    
    /* Send data */
    do
    {
        error = UART_RTOS_Receive(&handle, recv_buffer, sizeof(recv_buffer), &n);
        if (error == kStatus_UART_RxHardwareOverrun)
        {
            /* Notify about hardware buffer overrun */
            if (kStatus_Success != UART_RTOS_Send(&handle, (uint8_t *)send_hardware_overrun, strlen(send_hardware_overrun)))
            {
                //vTaskSuspend(NULL);
            }
        }
        if (error == kStatus_UART_RxRingBufferOverrun)
        {
            /* Notify about ring buffer overrun */
            if (kStatus_Success != UART_RTOS_Send(&handle, (uint8_t *)send_ring_overrun, strlen(send_ring_overrun)))
            {
                //vTaskSuspend(NULL);
            }
        }
        if (n > 0)
        {
            /* send back the received data */
            //UART_RTOS_Send(&handle, (uint8_t *)recv_buffer, n);
            
            norflash_sector_erase_ll(&flash_master_rtos_handle, 0x700000+0x1000*sector_num);
            norflash_write_data_ll(&flash_master_rtos_handle, 0x700000+0x1000*sector_num, n, recv_buffer);
            norflash_read_data_ll (&flash_master_rtos_handle, 0x700000+0x1000*sector_num, n, read_back);
            for (uint32_t i=0; i<n; i++)
            {
                if (recv_buffer[i] != read_back[i]) PRINTF("\r\n!error occurred n=%d!\r\n", n);
            }
        }
        PRINTF("\r\n!!!\r\n");
    } while (1);//kStatus_Success == error);

    UART_RTOS_Deinit(&handle);

    vTaskSuspend(NULL);
}
