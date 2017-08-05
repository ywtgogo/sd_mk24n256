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
#include "fsl_adc16.h"

volatile bool g_Adc16ConversionDoneFlag = false;
volatile uint32_t g_Adc16ConversionValue;

static int32_t BatControl(p_shell_context_t context, int32_t argc, char **argv);

const shell_command_context_t   xBatCommand = {"bat",
                                "\r\n\"bat \":\r\n",
//                                " Usage:\r\n"
//                                "    arg1: 1|2|3|4                              Led index\r\n"
//                                "    arg2: on|off                               Led status\r\n",
                                BatControl, 0};

/*******************************************************************************
 * Code
 ******************************************************************************/
void BAT_ADC16_IRQ_HANDLER_FUNC(void)
{
    g_Adc16ConversionDoneFlag = true;
    /* Read conversion result to clear the conversion completed flag. */
    g_Adc16ConversionValue = ADC16_GetChannelConversionValue(BAT_ADC16_BASE, BAT_ADC16_CHANNEL_GROUP);
}

adc16_config_t adc16ConfigStruct;
adc16_channel_config_t adc16ChannelConfigStruct;
    
void adc16_batery_init()
{
    BAT_nCHG_INIT();
    BAT_nPG_INIT();
	EnableIRQ(BAT_ADC16_IRQn);
	ADC16_GetDefaultConfig(&adc16ConfigStruct);
	ADC16_Init(BAT_ADC16_BASE, &adc16ConfigStruct);    
	ADC16_EnableHardwareTrigger(BAT_ADC16_BASE, false);
#if defined(FSL_FEATURE_ADC16_HAS_CALIBRATION) && FSL_FEATURE_ADC16_HAS_CALIBRATION
    if (kStatus_Success == ADC16_DoAutoCalibration(BAT_ADC16_BASE))
    {
        PRINTF("ADC16_DoAutoCalibration() Success.\r\n");
    }
    else
    {
        PRINTF("ADC16_DoAutoCalibration() Failed.\r\n");
    }
#endif /* FSL_FEATURE_ADC16_HAS_CALIBRATION */
	adc16ChannelConfigStruct.channelNumber = BAT_ADC16_USER_CHANNEL;
    adc16ChannelConfigStruct.enableInterruptOnConversionCompleted = true;

    #if defined(FSL_FEATURE_ADC16_HAS_DIFF_MODE) && FSL_FEATURE_ADC16_HAS_DIFF_MODE
        adc16ChannelConfigStruct.enableDifferentialConversion = false;
    #endif /* FSL_FEATURE_ADC16_HAS_DIFF_MODE */
}

static int32_t BatControl(p_shell_context_t context, int32_t argc, char **argv)
{   
    uint32_t no_charging;
    uint32_t charger;
    
	PRINTF("\r\nbattery adc check\r\n");
//	pmc_bandgap_buffer_config_t pmc_bandgap_buffer_config;
//	pmc_bandgap_buffer_config.enable = true;
//	PMC_ConfigureBandgapBuffer(PMC,&pmc_bandgap_buffer_config);

    g_Adc16ConversionDoneFlag = false;
    do{
    	   ADC16_SetChannelConfig(BAT_ADC16_BASE, BAT_ADC16_CHANNEL_GROUP, &adc16ChannelConfigStruct);
    	    while (!g_Adc16ConversionDoneFlag);
    	    PRINTF("ADC Value: %d\r\n",g_Adc16ConversionValue);

    	    no_charging = GPIO_ReadPinInput(BAT_nCHG_GPIO, BAT_nCHG_GPIO_PIN);
    	    PRINTF("%s Charger\r\n", no_charging?"Has no ":"HAS");
    }while(g_Adc16ConversionValue<0x840 && no_charging);

    charger = !GPIO_ReadPinInput(BAT_nPG_GPIO, BAT_nPG_GPIO_PIN);
    PRINTF("%s charging\r\n", charger?"Yes":"No");

    return 0;
}