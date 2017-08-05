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

#include "pin_mux.h"
#include "clock_config.h"


static int32_t LedControl(p_shell_context_t context, int32_t argc, char **argv);

const shell_command_context_t   xLedCommand = {"led",
                                "\r\n\"led arg1 arg2\":\r\n"
                                " Usage:\r\n"
                                "    arg1: 1|2|3|4                              Led index\r\n"
                                "    arg2: on|off                               Led status\r\n",
                                LedControl, 2};

/*******************************************************************************
 * Code
 ******************************************************************************/
void Led_Init(void)
{
    LED3_RED_INIT(LOGIC_LED_ON);
    //LED1_YELLOW_INIT(LOGIC_LED_OFF);
    //LED1_BLUE_INIT(LOGIC_LED_OFF);
    LED2_GREEN_INIT(LOGIC_LED_OFF);
}

static int32_t LedControl(p_shell_context_t context, int32_t argc, char **argv)
{
    int32_t kLedIndex = ((int32_t)atoi(argv[1]));
    char *kLedCommand = argv[2];

    /* Check second argument to control led */
    switch (kLedIndex)
    {
        case 1:
            if (strcmp(kLedCommand, "on") == 0)
            {
                LED1_RED_ON();
                LED1_YELLOW_ON();
                LED1_BLUE_ON();
                LED1_GREEN_ON();
            }
            else if (strcmp(kLedCommand, "off") == 0)
            {
                LED1_RED_OFF();
                LED1_YELLOW_OFF();
                LED1_BLUE_OFF();
                LED1_GREEN_OFF();
            }
            else
            {
                SHELL_Printf("Control conmmand is wrong!\r\n");
            }
            break;
        case 2:
            if (strcmp(kLedCommand, "on") == 0)
            {
                LED2_RED_ON();
                LED2_BLUE_ON();
                LED2_GREEN_ON();
            }
            else if (strcmp(kLedCommand, "off") == 0)
            {
                LED2_RED_OFF();
                LED2_BLUE_OFF();
                LED2_GREEN_OFF();
            }
            else
            {
                SHELL_Printf("Control conmmand is wrong!\r\n");
            }
            break;
        case 3:
            if (strcmp(kLedCommand, "on") == 0)
            {
                LED3_RED_ON();
                LED3_BLUE_ON();
                LED3_GREEN_ON();
            }
            else if (strcmp(kLedCommand, "off") == 0)
            {
                LED3_RED_OFF();
                LED3_BLUE_OFF();
                LED3_GREEN_OFF();
            }
            else
            {
                SHELL_Printf("Control conmmand is wrong!\r\n");
            }
            break;
        default:
            SHELL_Printf("LED index is wrong\r\n");
            break;
    }
    return 0;
}