#include <stdio.h>
#include <string.h>
#include <stdlib.h>

/* FreeRTOS kernel includes. */
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "timers.h"

/* Freescale includes. */
#include "fsl_shell.h"
#include "fsl_debug_console.h"
//#include "fsl"
    char buf[1024];
static int32_t ProfilingControl(p_shell_context_t context, int32_t argc, char **argv);

const shell_command_context_t   xProfilingCommand = {"profiling",
                                "\r\n\"profiling \":\r\n",
                                ProfilingControl, 0};

static int32_t ProfilingControl(p_shell_context_t context, int32_t argc, char **argv)
{
    PRINTF("profiling\r\n");

    
    vTaskGetRunTimeStats(buf);
    
    PRINTF("%s\r\n", buf);
    
    
    return 0;
}

