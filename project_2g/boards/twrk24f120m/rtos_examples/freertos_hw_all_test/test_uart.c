#include <stdio.h>
#include <string.h>
#include <stdlib.h>

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
#include "fsl_shell.h"

#include "drv_st7580.h"

#include "pin_mux.h"
#include "clock_config.h"

extern uart_rtos_handle_t console_handle;
extern uart_rtos_handle_t plc_handle;
uart_rtos_handle_t km_handle;
struct _uart_handle km_t_handle;



const char *to_send = "AT\r\n";
const char *send_ring_overrun = "\r\nRing buffer overrun!\r\n";
const char *send_hardware_overrun = "\r\nHardware buffer overrun!\r\n";

static int32_t UartControl(p_shell_context_t context, int32_t argc, char **argv);

const shell_command_context_t   xUartCommand = {"uart",
                                "\r\n\"uart arg1 arg2\":\r\n"
                                " Usage:\r\n"
                                "    arg1: 0|1|2|3|4                            Uart index\r\n"
                                /*"    arg2: on|off                               Uart do\r\n"*/,
                                UartControl, 1};

static void km_uart_test(void)
{
    int error;
    size_t n;
    uint8_t background_buffer[512];
    uint8_t recv_buffer[1];
    
    struct rtos_uart_config uart_config = {
        .baudrate = 115200,
        .parity = kUART_ParityDisabled,
        .stopbits = kUART_OneStopBit,
        .buffer = background_buffer,
        .buffer_size = sizeof(background_buffer),
    };
   
    uart_config.srcclk = CLOCK_GetFreq(KM_UART_CLKSRC);
    uart_config.base = KM_UART;

    if (0 > UART_RTOS_Init(&km_handle, &km_t_handle, &uart_config))
    {
        PRINTF("Error during UART initialization.\r\n");
        vTaskSuspend(NULL);
    }

    /* Send some data */
    if (0 > UART_RTOS_Send(&km_handle, (uint8_t *)"AT\r\n", strlen("AT\r\n")))
    {
        PRINTF("Error during UART send.\r\n");
        vTaskSuspend(NULL);
    }
    
    KM_POWER_INIT();
    KM_POWER_ON();//À­µÍ¿ªGSM
	vTaskDelay(6000);
	KM_POWER_OFF();
    
    while(1)
    {
        error = UART_RTOS_Receive(&km_handle, recv_buffer, sizeof(recv_buffer), &n);    
        if (n > 0)
        {         
            PRINTF("%c",recv_buffer[0]);
            if (0 == strncmp("K", (char *)recv_buffer, 1))
            {
                PRINTF("\r\n");
                break;
            }
        }
    } 
    vTaskDelay(1000);
    if (0 > UART_RTOS_Send(&km_handle, (uint8_t *)"ATD18121135161\r\n", strlen("ATD18121135161\r\n")))
    {
        PRINTF("Error during UART send.\r\n");
        vTaskSuspend(NULL);
    }
    while(1)
    {
        error = UART_RTOS_Receive(&km_handle, recv_buffer, sizeof(recv_buffer), &n);    
        if (n > 0)
        {         
            PRINTF("%c",recv_buffer[0]);
            if (0 == strncmp("K", (char *)recv_buffer, 1))
            {
                PRINTF("\r\n");
                break;
            }
        }
    } 
    
    UART_RTOS_Deinit(&km_handle);
}

#include "plc_k24_api.h"
#include "uartplc_k24.h"
#include "test_ping.h"

xTaskHandle pvCreatedTask_PlcSend_task;

static void plc_uart_test_rx(void)
{
    TEST_PING_SN(NULL);
}

static void plc_uart_test_tx(void)
{
    TEST_PING_MN(NULL);
}

uint32_t test_s=1;
extern   void plc_send_poling_task(void *pvParameters);
static int32_t UartControl(p_shell_context_t context, int32_t argc, char **argv)
{
    int32_t kUartIndex = ((int32_t)atoi(argv[1]));
    //char *kUartCommand = argv[2];

    /* Check second argument to control led */
    switch (kUartIndex)
    {
        case 0:
            PRINTF("Test KM\r\n");
            km_uart_test();
            break;
        case 1:
            PRINTF("Test WIFI\r\n");
            rtc_test();        
            break;

        case 2:
            PRINTF("Test SILVER\r\n");
            
            break;

        case 3:
            PRINTF("Test PLC RX\r\n");
            if (test_s==1)
            {
                plc_uart_test_tx();
                xTaskCreate(plc_send_poling_task, "Plc_send_poling_task", configMINIMAL_STACK_SIZE * 2, NULL, configMAX_PRIORITIES-4, &pvCreatedTask_PlcSend_task);
                test_s=2;
            }else if (test_s==2)
            {
                vTaskResume(pvCreatedTask_PlcSend_task);
                test_s=3;
            }else if (test_s==3)
            {
                vTaskSuspend(pvCreatedTask_PlcSend_task);       
                test_s=2;
            }
            
            break;

        case 4:
            PRINTF("Send data to Nor\r\n");
            /* DeInit DebugConsole */
            
            break;

        default:
            SHELL_Printf("Uart index is wrong\r\n");
            break;
    }
    return 0;
}

void DataToNor(uint8_t *buf)
{
    
}
