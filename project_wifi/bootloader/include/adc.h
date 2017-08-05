#include "fsl_adc16.h"

#define DEMO_ADC16_BASE ADC0
#define DEMO_ADC16_CHANNEL_GROUP    0U
#define DEMO_ADC16_USER_CHANNEL     8U

 #define ADC_PDD_SINGLE_ENDED_BANDGAP     0x1BU
#define ADC_PDD_SINGLE_ENDED_VREFSH      0x1DU


#define DEMO_ADC16_IRQn ADC0_IRQn
#define DEMO_ADC16_IRQ_HANDLER_FUNC ADC0_IRQHandler

/*******************************************************************************
 * Variables
 ******************************************************************************/
volatile bool g_Adc16ConversionDoneFlag = false;
volatile uint32_t g_Adc16ConversionValue;
