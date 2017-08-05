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
#include "fsl_dspi_freertos.h"
#include "fsl_dspi.h"
#include "fsl_shell.h"

#include "pin_mux.h"
#include "clock_config.h"
#include "board.h"

static int32_t KeyControl(p_shell_context_t context, int32_t argc, char **argv);

const shell_command_context_t   xKeyCommand = {"key",
                                "\r\n\"key arg1\":\r\n"
                                " Usage:\r\n"
                                "    arg1: 0|1|2|3|4|5                          Key index\r\n"
                                /*"    arg2: on|off                               Key do\r\n"*/,
                                KeyControl, 1};

static int32_t KeyControl(p_shell_context_t context, int32_t argc, char **argv)
{
    int32_t kKeyIndex = ((int32_t)atoi(argv[1]));
    //char *kUartCommand = argv[2];

    /* Check second argument to control led */
    switch (kKeyIndex)
    {
        case 0:

            break;
        case 1:

            break;
        default:
            SHELL_Printf("Key index is wrong\r\n");
            break;
    }
    return 0;
}