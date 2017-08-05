/**
* @file    SPIRIT_SDK_Util.c
* @author  VMA division - AMS
* @version 3.2.1
* @date    May 1, 2016
* @brief   Identification functions for SPIRIT DK.
* @details
*
* THE PRESENT FIRMWARE WHICH IS FOR GUIDANCE ONLY AIMS AT PROVIDING CUSTOMERS
* WITH CODING INFORMATION REGARDING THEIR PRODUCTS IN ORDER FOR THEM TO SAVE
* TIME. AS A RESULT, STMICROELECTRONICS SHALL NOT BE HELD LIABLE FOR ANY
* DIRECT, INDIRECT OR CONSEQUENTIAL DAMAGES WITH RESPECT TO ANY CLAIMS ARISING
* FROM THE CONTENT OF SUCH FIRMWARE AND/OR THE USE MADE BY CUSTOMERS OF THE
* CODING INFORMATION CONTAINED HEREIN IN CONNECTION WITH THEIR PRODUCTS.
*
* THIS SOURCE CODE IS PROTECTED BY A LICENSE.
* FOR MORE INFORMATION PLEASE CAREFULLY READ THE LICENSE AGREEMENT FILE LOCATED
* IN THE ROOT DIRECTORY OF THIS FIRMWARE PACKAGE.
*
* <h2><center>&copy; COPYRIGHT 2016 STMicroelectronics</center></h2>
*/  


/* Includes ------------------------------------------------------------------*/
#include "SPIRIT_SDK_Util.h"

/**
* @addtogroup SPIRIT_DK                   SPIRIT DK
* @{
*/


/**
* @defgroup SDK_SPIRIT_MANAGEMENT              SDK SPIRIT Management
* @{
*/


/**
* @brief This flag is used to synchronize the TIM3 ISR with the XtalMeasurement routine.
*/
static volatile FlagStatus s_xTIMChCompareModeRaised = RESET;

/**
* @brief This flag is used to synchronize the TIM3 ISR with the XtalMeasurement routine.
*/
static RangeExtType xRangeExtType = RANGE_EXT_NONE;

static uint8_t s_RfModuleBand = 0;
static int32_t s_RfModuleOffset=0;
//static TIM_HandleTypeDef  TimHandle;

#define ENABLE_TCXO()           {__GPIOC_CLK_ENABLE();\
                                HAL_GPIO_Init(GPIOC, &(GPIO_InitTypeDef){GPIO_PIN_2, GPIO_MODE_OUTPUT_PP, GPIO_NOPULL, GPIO_SPEED_HIGH });\
                                HAL_GPIO_WritePin(GPIOC, GPIO_PIN_2, GPIO_PIN_SET);}


/**
* @defgroup SDK_SPIRIT_MANAGEMENT_FUNCTIONS    SDK SPIRIT Management Functions
* @{
*/

/**
* @defgroup IDENTIFICATION_FUNCTIONS      SDK SPIRIT Management Identification Functions
* @{
*/


/**
* @brief This function handles TIM3 global interrupt
* @param None.
* @retval None.
*/
//void TIM3_IRQHandler(void)
//{
//    HAL_TIM_IRQHandler(&TimHandle);
//
//}
//
//void HAL_TIM_IC_CaptureCallback(TIM_HandleTypeDef *htim) 
//{
//  s_xTIMChCompareModeRaised = SET;
//}



/**
* @brief  This function can be used to automatically measure the XTAL frequency making use of the
*         Spirit clock output to pin and an STM32L timer in compare mode.
* @param  None.
* @retval None.
*/
uint32_t SpiritManagementComputeXtalFrequency(void)
{   
  SpiritRadioSetXtalFrequency(50000000);
  return 50000000;
}

//#define N_SAMPLES 20
//#define SETTLING_PERIODS 4
//#define A 0.4
//uint32_t SpiritManagementComputeXtalFrequency(void)
//{   
//  GPIO_TypeDef *pGpioPeriph;
//  GPIO_InitTypeDef GPIO_InitStructure;
//  TIM_IC_InitTypeDef     sICConfig;
//  
//  if(SdkEvalGetVersion()==SDK_EVAL_VERSION_D1) {
//    TimHandle.Instance = TIM4;
//    pGpioPeriph=GPIOB;
//    /* TIM4 clock enable */
//    __HAL_RCC_TIM4_CLK_ENABLE();
//    /* GPIOB clock enable */
//    __GPIOB_CLK_ENABLE();
//  }
//  else {
//    TimHandle.Instance = TIM3;
//    pGpioPeriph=GPIOC;
//    /* TIM3 clock enable */
//    __HAL_RCC_TIM3_CLK_ENABLE();
//    /* GPIOC clock enable */
//    __GPIOC_CLK_ENABLE();
//  }    
//  
//  //#warning It is more safe disable all the other interrupt source.
//  
//  uint32_t lMeasuredXtalFrequency;
//
//  if(SpiritManagementGetRangeExtender()==RANGE_EXT_SKYWORKS_169)
//  {
//    ENABLE_TCXO();
//  }
//  
//  /* Instance the variables used to compute the XTAL frequency */
//  uint8_t CaptureNumber=0;
//  uint16_t IC3ReadValue1=0,IC3ReadValue2=0,Capture=0;
//  volatile uint16_t cWtchdg = 0;
//  uint32_t TIMFreq=0,lXoFreq=0;
//  float fXoFreqRounded;
//  
//  /* TIM3 channel 2 pin (PC.07) configuration */
//  GPIO_InitStructure.Mode  = GPIO_MODE_AF_PP;
//  GPIO_InitStructure.Speed = GPIO_SPEED_HIGH;
//  GPIO_InitStructure.Pull  = GPIO_PULLUP;
//  GPIO_InitStructure.Pin   = GPIO_PIN_7;
//  if(TimHandle.Instance ==TIM3) {
//    GPIO_InitStructure.Alternate   = GPIO_AF2_TIM3;
//  }
//  else {
//    GPIO_InitStructure.Alternate   = GPIO_AF2_TIM4;
//  }
//  HAL_GPIO_Init(pGpioPeriph, &GPIO_InitStructure);
//    
//  /* Configure the timer IRQ to be raised on the rising fronts */
//  sICConfig.ICPolarity  = TIM_ICPOLARITY_RISING;
//  
//  /* Input capture selection setting */
//  sICConfig.ICSelection = TIM_ICSELECTION_DIRECTTI;
//   
//  /* Input capture prescaler setting. Setting it to TIM_ICPSC_DIV8 makes the IRQ are raised every 8 rising fronts detected by hardware.  */
//  sICConfig.ICPrescaler = TIM_ICPSC_DIV8;
//  
//  /* Disable every kind of capture filter */
//  sICConfig.ICFilter = 0x0;
//
//  /* Timer initialization */
//  HAL_TIM_IC_ConfigChannel(&TimHandle, &sICConfig, TIM_CHANNEL_2);
//  
//  /* TIM enable counter */
//  HAL_TIM_IC_Start_IT(&TimHandle, TIM_CHANNEL_2);
//        
//  if(TimHandle.Instance ==TIM3) {
//    HAL_NVIC_SetPriority(TIM3_IRQn, 0x0F, 0x0F);
//    HAL_NVIC_EnableIRQ(TIM3_IRQn);
//  }
//  else {
//    HAL_NVIC_SetPriority(TIM4_IRQn, 0x0F, 0x0F);
//    HAL_NVIC_EnableIRQ(TIM4_IRQn);
//  }
//  
//  /* Disable the clock divider to measure the max frequency of the clock. */
//  uint8_t tmp= 0x29; SpiritSpiWriteRegisters(0xB4, 1, &tmp);
//  
//  /* Spirit1 side clock configuration */
//  SpiritGpioClockOutputInit(&(ClockOutputInit){XO_RATIO_1_192, RCO_RATIO_1, EXTRA_CLOCK_CYCLES_0});
//  
//  /* Instance the structure used to configure the Spirit clock frequency to be divided by a 192 factor. */
//  SpiritGpioInit(&(SGpioInit){SPIRIT_GPIO_0, SPIRIT_GPIO_MODE_DIGITAL_OUTPUT_LP, SPIRIT_GPIO_DIG_OUT_MCU_CLOCK});
//  
//  SpiritGpioClockOutput(S_ENABLE);
//  
//  /* measure the frequency and average it on N_SAMPLES. Moreover cycle to wait for same SETTLING_PERIODS */
//  for(uint32_t i=0;i<2*(N_SAMPLES+SETTLING_PERIODS);i++) {
//    /* block the routine until the TIM CCP2 IRQ is raised */
//    while(!s_xTIMChCompareModeRaised && (cWtchdg!=0xFFFF)) {
//      cWtchdg++;    
//    }
//    
//    if(cWtchdg==0xFFFF) {
//      break;
//    }
//    else {
//      cWtchdg=0;
//    }
//    
//    /* reset the IRQ raised flag */
//    s_xTIMChCompareModeRaised = RESET;
//    
//    /* if the SETTLING PERIODS expired */
//    if(i>=SETTLING_PERIODS*2) {
//      /* First TIMER capture */
//      if(CaptureNumber == 0)
//      {
//        /* Get the Input Capture value */
//        IC3ReadValue1 = HAL_TIM_ReadCapturedValue(&TimHandle, TIM_CHANNEL_2);
//        CaptureNumber = 1;
//      }
//      /* Second TIMER capture */
//      else if(CaptureNumber == 1)
//      {
//        /* Get the Input Capture value */
//        IC3ReadValue2 = HAL_TIM_ReadCapturedValue(&TimHandle, TIM_CHANNEL_2);
//        
//        /* Capture computation */
//        if (IC3ReadValue2 > IC3ReadValue1)
//        {
//          /* If the TIMER didn't overflow between the first and the second capture. Compute it as the difference between the second and the first capture values. */
//          Capture = (IC3ReadValue2 - IC3ReadValue1) - 1;
//        }
//        else
//        {
//          /* .. else, if overflowed 'roll' the first measure to be complementar of 0xFFFF */
//          Capture = ((0xFFFF - IC3ReadValue1) + IC3ReadValue2) - 1;
//        }
//        
//        /* Punctual frequency computation */
//        TIMFreq = (uint32_t) SystemCoreClock / Capture;
//        
//        /* Averaged frequency computation */
//        lXoFreq =(uint32_t)(A*(float)lXoFreq+(1.0-A)*(float)TIMFreq);
//        
//        CaptureNumber = 0;
//      }
//    }
//  }
//  
//  /* Compute the real frequency in Hertz tanking in account the MCU and Spirit divisions */
//  lXoFreq *=(192*8);
//  
//  /* Disable the output clock */
//  SpiritGpioClockOutput(S_DISABLE);
//  
//  /* TIM disable counter */
//  HAL_TIM_IC_Stop(&TimHandle, TIM_CHANNEL_2);
//    
//  /* SPIRIT GPIO 0 to the default configuration */
//  SpiritGpioSetLevel(SPIRIT_GPIO_0, LOW);
//
//  tmp= 0x21; SpiritSpiWriteRegisters(0xB4, 1, &tmp);
//  
//  /* Round the measured frequency to be measured as an integer MHz value */
//  fXoFreqRounded = (float)lXoFreq/1e6;
//  
//  if( fXoFreqRounded-(float)((uint32_t)fXoFreqRounded)>0.5)
//  {
//    lMeasuredXtalFrequency = (((uint32_t)fXoFreqRounded+1)*1000000);
//  }
//  else
//  {
//    lMeasuredXtalFrequency = (((uint32_t)fXoFreqRounded)*1000000);
//  }
//  
//  SdkEvalM2SGpioInit(M2S_GPIO_0, M2S_MODE_GPIO_IN);
//  
//  SpiritRadioSetXtalFrequency(lMeasuredXtalFrequency);
//  
//  return lMeasuredXtalFrequency;
//}

uint32_t SpiritManagementComputeXtalFrequencyGpio2(void)
{
  SpiritRadioSetXtalFrequency(50000000);
  return 50000000;
}
///* This function is used to detect the pa ext board, due to the unworking measurement algorithm */
//uint32_t SpiritManagementComputeXtalFrequencyGpio2(void)
//{
//  GPIO_TypeDef *pGpioPeriph;
//  GPIO_InitTypeDef GPIO_InitStructure;
//  TIM_IC_InitTypeDef     sICConfig;
//  
//  if(SdkEvalGetVersion()==SDK_EVAL_VERSION_D1) {
//    TimHandle.Instance = TIM4;
//    pGpioPeriph=GPIOB;
//    /* TIM4 clock enable */
//    __HAL_RCC_TIM4_CLK_ENABLE();
//    /* GPIOB clock enable */
//    __GPIOB_CLK_ENABLE();
//  }
//  else {
//    TimHandle.Instance = TIM3;
//    pGpioPeriph=GPIOC;
//    /* TIM3 clock enable */
//    __HAL_RCC_TIM3_CLK_ENABLE();
//    /* GPIOC clock enable */
//    __GPIOC_CLK_ENABLE();
//  }  
//  
//  /* MCU GPIO, NVIC and timer configuration structures */
//  
//  uint32_t lMeasuredXtalFrequency;
//  
//  /* Instance the variables used to compute the XTAL frequency */
//  uint8_t CaptureNumber=0;
//  uint16_t IC3ReadValue1=0,IC3ReadValue2=0,Capture=0;
//  volatile uint16_t cWtchdg = 0;
//  uint32_t TIM3Freq=0,lXoFreq=0;
//  float fXoFreqRounded;
//    
//  /* TIM3 channel 2 pin (PC.07) configuration */
//  GPIO_InitStructure.Mode  = GPIO_MODE_AF_PP;
//  GPIO_InitStructure.Speed = GPIO_SPEED_HIGH;
//  GPIO_InitStructure.Pull  = GPIO_PULLUP;
//  GPIO_InitStructure.Pin   = GPIO_PIN_9;
//  if(TimHandle.Instance ==TIM3) {
//    GPIO_InitStructure.Alternate   = GPIO_AF2_TIM3;
//  }
//  else {
//    GPIO_InitStructure.Alternate   = GPIO_AF2_TIM4;
//  }
//  HAL_GPIO_Init(pGpioPeriph, &GPIO_InitStructure);
//    
//  /* Configure the timer IRQ to be raised on the rising fronts */
//  sICConfig.ICPolarity  = TIM_ICPOLARITY_RISING;
//  
//  /* Input capture selection setting */
//  sICConfig.ICSelection = TIM_ICSELECTION_DIRECTTI;
//  
//  /* Input capture prescaler setting. Setting it to TIM_ICPSC_DIV8 makes the IRQ are raised every 8 rising fronts detected by hardware.  */
//  sICConfig.ICPrescaler = TIM_ICPSC_DIV8;
//  
//  /* Disable every kind of capture filter */
//  sICConfig.ICFilter = 0x0;
//  
//  /* Timer initialization */
//  HAL_TIM_IC_ConfigChannel(&TimHandle, &sICConfig, TIM_CHANNEL_4);
//
//  HAL_TIM_IC_Start_IT(&TimHandle, TIM_CHANNEL_4);
//  
//  if(TimHandle.Instance ==TIM3) {
//    HAL_NVIC_SetPriority(TIM3_IRQn, 0x0F, 0x0F);
//    HAL_NVIC_EnableIRQ(TIM3_IRQn);
//  }
//  else {
//    HAL_NVIC_SetPriority(TIM4_IRQn, 0x0F, 0x0F);
//    HAL_NVIC_EnableIRQ(TIM4_IRQn);
//  }
//
//  /* Spirit1 side clock configuration */
//  SpiritGpioClockOutputInit(&(ClockOutputInit){XO_RATIO_1_192, RCO_RATIO_1, EXTRA_CLOCK_CYCLES_0});
//  
//  /* Instance the structure used to configure the Spirit clock frequency to be divided by a 192 factor. */
//  SpiritGpioInit(&(SGpioInit){SPIRIT_GPIO_2, SPIRIT_GPIO_MODE_DIGITAL_OUTPUT_LP, SPIRIT_GPIO_DIG_OUT_MCU_CLOCK});
//  SpiritGpioClockOutput(S_ENABLE);
//  
//  /* measure the frequency and average it on N_SAMPLES. Moreover cycle to wait for same SETTLING_PERIODS */
//  for(uint32_t i=0;i<2*(N_SAMPLES+SETTLING_PERIODS);i++)
//  {
//    /* block the routine until the TIM3 CCP2 IRQ is raised */
//    while(!s_xTIMChCompareModeRaised && (cWtchdg!=0xFFFF))
//      cWtchdg++;    
//    
//    if(cWtchdg==0xFFFF)
//      break;
//    else
//      cWtchdg=0;
//    
//    /* reset the IRQ raised flag */
//    s_xTIMChCompareModeRaised = RESET;
//    
//    /* if the SETTLING PERIODS expired */
//    if(i>=SETTLING_PERIODS*2)
//    {
//      /* First TIMER3 capture */
//      if(CaptureNumber == 0)
//      {
//        /* Get the Input Capture value */
//        IC3ReadValue1 = HAL_TIM_ReadCapturedValue(&TimHandle, TIM_CHANNEL_2);
//        CaptureNumber = 1;
//      }
//      /* Second TIMER3 capture */
//      else if(CaptureNumber == 1)
//      {
//        /* Get the Input Capture value */
//        IC3ReadValue2 = HAL_TIM_ReadCapturedValue(&TimHandle, TIM_CHANNEL_2);
//        
//        /* Capture computation */
//        if (IC3ReadValue2 > IC3ReadValue1)
//        {
//          /* If the TIMER3 didn't overflow between the first and the second capture. Compute it as the difference between the second and the first capture values. */
//          Capture = (IC3ReadValue2 - IC3ReadValue1) - 1;
//        }
//        else
//        {
//          /* .. else, if overflowed 'roll' the first measure to be complementar of 0xFFFF */
//          Capture = ((0xFFFF - IC3ReadValue1) + IC3ReadValue2) - 1;
//        }
//        
//        /* Punctual frequency computation */
//        TIM3Freq = (uint32_t) SystemCoreClock / Capture;
//        
//        /* Averaged frequency computation */
//        lXoFreq =(uint32_t)(A*(float)lXoFreq+(1.0-A)*(float)TIM3Freq);
//        
//        CaptureNumber = 0;
//      }
//    }
//  }
//  
//  /* Compute the real frequency in Hertz tanking in account the MCU and Spirit divisions */
//  lXoFreq *=(192*8);
//  
//  /* Disable the output clock */
//  SpiritGpioClockOutput(S_DISABLE);
//  
//  HAL_TIM_IC_Stop(&TimHandle, TIM_CHANNEL_4);
//
//  /* SPIRIT GPIO 2 to the default configuration */
//  SpiritGpioSetLevel(SPIRIT_GPIO_2, LOW);
//  
//  /* Round the measured frequency to be measured as an integer MHz value */
//  fXoFreqRounded = (float)lXoFreq/1e6;
//  
//  if( fXoFreqRounded-(float)((uint32_t)fXoFreqRounded)>0.5)
//  {  
//    lMeasuredXtalFrequency = (((uint32_t)fXoFreqRounded+1)*1000000);
//  }
//  else
//  {
//    lMeasuredXtalFrequency = (((uint32_t)fXoFreqRounded)*1000000);
//  }
//  
//  SdkEvalM2SGpioInit(M2S_GPIO_2, M2S_MODE_GPIO_IN);
//  
//  return lMeasuredXtalFrequency;
//}



/**
* @brief  Read the status register.
* @param  None
* @retval Status
*/
uint8_t EepromIdentification(void)
{
//  uint8_t status;
//  status = EepromSetSrwd();
//  status = EepromStatus();
//  if((status&0xF0) == EEPROM_STATUS_SRWD) { // 0xF0 mask [SRWD 0 0 0]
//    status = 1;
//    EepromResetSrwd();
//  }
//  else
//    status = 0;
//  
//  return status;
}


void SpiritManagementIdentificationRFBoard(void)
{
    do{
      /* Delay for state transition */
      for(volatile uint8_t i=0; i!=0xFF; i++);
      
      /* Reads the MC_STATUS register */
     SpiritRefreshStatus();
    }while(g_xStatus.MC_STATE!=MC_STATE_READY);

    //SdkEvalSetHasEeprom(EepromIdentification());

    if(!0)//(!SdkEvalGetHasEeprom()) // EEPROM is not present
    {
        SpiritManagementComputeXtalFrequency();
    }
//  else  // EEPROM found
//  {
//    //read the memory and set the variable
//    uint8_t tmpBuffer[32];
//    EepromRead(0x0000, 32, tmpBuffer);
//    uint32_t xtal;
//    float foffset=0;
//    if(tmpBuffer[0]==0 || tmpBuffer[0]==0xFF) {
//      SpiritManagementComputeXtalFrequency();
//      return;
//    }
//    switch(tmpBuffer[1]) {
//    case 0:
//      xtal = 24000000;
//      SpiritRadioSetXtalFrequency(xtal);
//      break;
//    case 1:
//      xtal = 25000000;
//      SpiritRadioSetXtalFrequency(xtal);
//      break;
//    case 2:
//      xtal = 26000000;
//      SpiritRadioSetXtalFrequency(xtal);
//      break;
//    case 3:
//      xtal = 48000000;
//      SpiritRadioSetXtalFrequency(xtal);
//      break;
//    case 4:
//      xtal = 50000000;
//      SpiritRadioSetXtalFrequency(xtal);
//      break;
//    case 5:
//      xtal = 52000000;
//      SpiritRadioSetXtalFrequency(xtal);
//      break;
//    default:
//      SpiritManagementComputeXtalFrequency();
//      break;
//    }
//        
//    RangeExtType range;
//    if(tmpBuffer[5]==0) {
//      range = RANGE_EXT_NONE;
//    }
//    else if(tmpBuffer[5]==1) {
//      range = RANGE_EXT_SKYWORKS_169;
//    }
//    else if(tmpBuffer[5]==2) {
//      range = RANGE_EXT_SKYWORKS_868;
//    }
//    else {
//      range = RANGE_EXT_NONE;
//    }
//    SpiritManagementSetRangeExtender(range);
//    
//    SpiritManagementSetBand(tmpBuffer[3]);
//    
//
//    EepromRead(0x0021,4,tmpBuffer);
//    for(uint8_t i=0;i<4;i++)
//    {
//      ((uint8_t*)&foffset)[i]=tmpBuffer[3-i];
//    }
//    SpiritManagementSetOffset((int32_t)foffset);
//  }
}

void SpiritManagementSetBand(uint8_t value)
{
  s_RfModuleBand = value;
}

uint8_t SpiritManagementGetBand(void)
{
  return s_RfModuleBand;
}

void SpiritManagementSetOffset(int32_t value)
{
  s_RfModuleOffset=value;
}

int32_t SpiritManagementGetOffset(void)
{
  return s_RfModuleOffset;
}


/**
* @}
*/



/**
* @}
*/



/**
* @defgroup RANGE_EXT_MANAGEMENT_FUNCTIONS              SDK SPIRIT Management Range Extender Functions
* @{
*/

void SpiritManagementRangeExtInit(void)
{
//  RangeExtType range_type = SpiritManagementGetRangeExtender();
//    
//  if(range_type==RANGE_EXT_SKYWORKS_169) {
//    ENABLE_TCXO();
//    /* TCXO optimization power consumption */
//    SpiritGeneralSetExtRef(MODE_EXT_XIN);
//    uint8_t tmp = 0x01; SpiritSpiWriteRegisters(0xB6,1,&tmp);
//    
//    /* CSD control */
//    SpiritGpioInit(&(SGpioInit){SPIRIT_GPIO_0, SPIRIT_GPIO_MODE_DIGITAL_OUTPUT_HP, SPIRIT_GPIO_DIG_OUT_TX_RX_MODE});
//    
//    /* CTX/BYP control */
//    SpiritGpioInit(&(SGpioInit){SPIRIT_GPIO_1, SPIRIT_GPIO_MODE_DIGITAL_OUTPUT_HP, SPIRIT_GPIO_DIG_OUT_TX_STATE});
//    
//    /* Vcont control */
//    SpiritGpioInit(&(SGpioInit){SPIRIT_GPIO_2, SPIRIT_GPIO_MODE_DIGITAL_OUTPUT_HP, SPIRIT_GPIO_DIG_OUT_RX_STATE});
//  }
//  else if(range_type==RANGE_EXT_SKYWORKS_868) {   
//    /* CSD control */
//    SpiritGpioInit(&(SGpioInit){SPIRIT_GPIO_0, SPIRIT_GPIO_MODE_DIGITAL_OUTPUT_HP, SPIRIT_GPIO_DIG_OUT_TX_RX_MODE});
//    
//    /* CTX/BYP control */
//    SpiritGpioInit(&(SGpioInit){SPIRIT_GPIO_1, SPIRIT_GPIO_MODE_DIGITAL_OUTPUT_HP, SPIRIT_GPIO_DIG_OUT_RX_STATE});
//    
//    /* Vcont control */
//    SpiritGpioInit(&(SGpioInit){SPIRIT_GPIO_2, SPIRIT_GPIO_MODE_DIGITAL_OUTPUT_HP, SPIRIT_GPIO_DIG_OUT_TX_STATE});
//  }
}

RangeExtType SpiritManagementGetRangeExtender(void)
{
  return xRangeExtType;
}

void SpiritManagementSetRangeExtender(RangeExtType xRangeType)
{
  xRangeExtType = xRangeType;
}

/**
* @}
*/




/**
* @}
*/

/**
* @}
*/


/******************* (C) COPYRIGHT 2016 STMicroelectronics *****END OF FILE****/
