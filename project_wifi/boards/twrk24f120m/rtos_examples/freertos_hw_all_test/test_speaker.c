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

#include "fsl_shell.h"
#include "fsl_dac.h"

#include "wav_record.h"

#define DEMO_DAC_INSTANCE DAC0

static int32_t SpkrControl(p_shell_context_t context, int32_t argc, char **argv);

const shell_command_context_t   xSpkrCommand = {"spkr",
                                "\r\n\"spkr \":\r\n",
//                                " Usage:\r\n"
//                                "    arg1: 1|2|3|4                              Led index\r\n"
//                                "    arg2: on|off                               Led status\r\n",
                                SpkrControl, 0};

static int32_t SpkrControl(p_shell_context_t context, int32_t argc, char **argv)
{   
    uint32_t i=0, j;
    dac_config_t dacConfigStruct;
    uint32_t dacValue=0;
    
    SPKR_GPIO_INIT();
    AUDIO_MSG_INIT();
    AUDIO_MSG_DAC();    
    PRINTF("\r\nDAC basic Example.\r\n");
    
    DAC_GetDefaultConfig(&dacConfigStruct);
    DAC_Init(DEMO_DAC_INSTANCE, &dacConfigStruct);
    DAC_Enable(DEMO_DAC_INSTANCE, true);
    DAC_SetBufferReadPointer(DEMO_DAC_INSTANCE, 0U);
    
    signed short *p =(signed short *)wav_record_file;
       
    while (1)
    {
        //PRINTF("\r\nPlease input a value (0 - 4095) to output with DAC: ");
        //SCANF("%d", &dacValue);
        //dacValue = wav_record_file[i++];
        
        //PRINTF("\r\nInput value is %d\r\n", dacValue);
        i++;
        
        dacValue = ((p[i])+32767)>>4;
        
        if (dacValue > 0xFFFU)
        {
            PRINTF("output.\r\n");
            //continue;
        }
        DAC_SetBufferValue(DEMO_DAC_INSTANCE, 0U, dacValue);
        if (i >= sizeof(wav_record_file)/2) {
            //i=0;
            break;
        }
        //if (i%2 == 0)
        {
            j = 1500;
            while(j--);
        }
        //PRINTF("DAC out: %d\r\n", dacValue);
        /*
        * The value in the first item would be converted. User can measure the output voltage from DAC_OUTx pin.
        */
    }
    
    return 0;
}