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
#include "fsl_shell.h"
#include "fsl_lptmr.h"

#include "board.h"
#include "pin_mux.h"
#include "clock_config.h"

uint32_t TimingPeakSensorFlag;

static int32_t PeakSensorControl(p_shell_context_t context, int32_t argc, char **argv);

static void TimingPeakSensorCallback(TimerHandle_t xTimer)
{
    TimingPeakSensorFlag = 1;
    LPTMR_ClearStatusFlags(LPTMR0, kLPTMR_TimerCompareFlag);
    PRINTF("LPTMR currentCNR Num%d \r\n", LPTMR_GetCurrentTimerCount(LPTMR0));   
} 

const shell_command_context_t   xPeakSensorCommand = {"peaksensor",
                                "\r\n\"peaksensor arg1 arg2\":\r\n" 
                                " Usage:\r\n"
                                "    arg1: get                                  PeakSensor status\r\n"
                                "    arg2:                                      PeakSensor delay'second\r\n",
                                PeakSensorControl, 2};

static int32_t PeakSensorControl(p_shell_context_t context, int32_t argc, char **argv)
{    
    TickType_t  tick_pre_get = xTaskGetTickCount();
    lptmr_config_t lptmrConfig;
    
    char        *kPeakSensorCommand = argv[1];
    int32_t     delay_second = ((int32_t)atoi(argv[2]));
    
    LPTMR_GetDefaultConfig(&lptmrConfig);
    LPTMR_Init(LPTMR0, &lptmrConfig);

    if (strcmp(kPeakSensorCommand, "get") == 0)
    {
        TimerHandle_t 	TimingPeakSensorHandle = NULL;
        TimingPeakSensorHandle = xTimerCreate("timing_peak_sensor",     /* Text name. */
                                   ( delay_second*1000), 			/* Timer period. */
                                     pdFALSE,                   /* Enable auto reload. */
                                     0,                         /* ID is not used. */
                                     TimingPeakSensorCallback);    /* The callback function. */
        xTimerStart(TimingPeakSensorHandle, 0);
        
        LPTMR_StartTimer(LPTMR0);      
        while(TimingPeakSensorFlag !=1) vTaskDelay(300);
        TimingPeakSensorFlag = 0;    
        xTimerDelete(TimingPeakSensorHandle, 0);
    }

    LPTMR_Deinit(LPTMR0);      
    return 0;
}
