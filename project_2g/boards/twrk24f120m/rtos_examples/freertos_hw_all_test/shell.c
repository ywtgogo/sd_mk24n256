/* Copyright (c) 2016, Freescale Semiconductor, Inc.
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

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "board.h"
#include "fsl_shell.h"
#include "fsl_debug_console.h"
#include "fsl_uart_freertos.h"
#include "shell_cmd.h"

#include "fsl_common.h"
#include "pin_mux.h"
#include "clock_config.h"
/*******************************************************************************
 * Definitions
 ******************************************************************************/

/*******************************************************************************
 * Variables
 ******************************************************************************/
uart_rtos_handle_t console_handle;
struct _uart_handle console_t_handle;     
/*******************************************************************************
 * Prototypes
 ******************************************************************************/


/* SHELL user send data callback */
static void SHELL_SendDataCallback(uint8_t *buf, uint32_t len);

/* SHELL user receive data callback */
static void SHELL_ReceiveDataCallback(uint8_t *buf, uint32_t len);



static void SHELL_SendDataCallback(uint8_t *buf, uint32_t len)
{
    while (len--)
    {
        UART_RTOS_Send(&console_handle, buf++, 1);
    }
}

static void SHELL_ReceiveDataCallback(uint8_t *buf, uint32_t len)
{
    while (len--)
    {
        UART_RTOS_Receive(&console_handle, buf++, 1, NULL);        
    }
}



void shell_task(void *pvParameters)
{
    shell_context_struct user_context;
    
    uint8_t background_buffer[32];
    struct rtos_uart_config uart_config = {
        .baudrate = 115200,
        .parity = kUART_ParityDisabled,
        .stopbits = kUART_OneStopBit,
        .buffer = background_buffer,
        .buffer_size = sizeof(background_buffer),
    };    
    uart_config.srcclk = CLOCK_GetFreq(CONSOLE_UART_CLKSRC);
    uart_config.base = CONSOLE_UART;
    
    if (0 > UART_RTOS_Init(&console_handle, &console_t_handle, &uart_config))
    {
        PRINTF("Error during UART initialization.\r\n");
        vTaskSuspend(NULL);
    }
    
    /* Init SHELL */
    SHELL_Init(&user_context, SHELL_SendDataCallback, SHELL_ReceiveDataCallback, SHELL_Printf, ">> ");
    /* Add new command to commands list */
    SHELL_RegisterCommand(&xLedCommand);
    SHELL_RegisterCommand(&xKeyCommand);
    SHELL_RegisterCommand(&xUartCommand);
    SHELL_RegisterCommand(&xSpiCommand);    
    SHELL_RegisterCommand(&xBatCommand);
    SHELL_RegisterCommand(&xPeakSensorCommand);
    SHELL_RegisterCommand(&xSpkrCommand);
    SHELL_RegisterCommand(&xProfilingCommand);
    SHELL_Main(&user_context);
}
