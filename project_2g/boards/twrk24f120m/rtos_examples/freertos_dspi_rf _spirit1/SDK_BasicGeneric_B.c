/**
* @file    SDK_BasicGeneric_B.c
* @author  VMA division - AMS
* @version 3.2.1
* @date    May 1, 2016
* @brief   Example of reception of SPIRIT Basic packets.
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
#include "SDK_EVAL_Config.h"
#include "SPIRIT_Config.h"
#include "SDK_Configuration_Common.h"
#include "SPIRIT_SDK_Util.h"
#include "SDK_Common.h"

#define USE_VCOM


/**
 * @addtogroup SDK_Examples
 * @{
 */

/**
 * @addtogroup SDK_Basic_Generic        SDK Basic Generic
 * @{
 */

/**
 * @addtogroup SDK_Basic_Generic_B              SDK Basic Generic B
 * @brief Device B configured as a receiver.
 * @details This code explains how to configure a receiver for
 * basic packets.
 *
 * The user can change the Basic packet configuration parameters editing the defines
 * at the beginning of the file.
 * @{
 */


/**
 * @defgroup Basic_Generic_B_Private_Defines                    Basic Generic B Private Defines
 * @{
 */

/*  Addresses configuration parameters  */
#undef MY_ADDRESS
#define MY_ADDRESS                  0x44
#undef DESTINATION_ADDRESS
#define DESTINATION_ADDRESS         0x34

/**
 *@}
 */


/**
 * @defgroup Basic_Generic_B_Private_Variables                          Basic Generic B Private Variables
 * @{
 */

 /**
  * @brief Radio structure fitting
  */
  SRadioInit xRadioInit = {
    XTAL_OFFSET_PPM,
    BASE_FREQUENCY,
    CHANNEL_SPACE,
    CHANNEL_NUMBER,
    MODULATION_SELECT,
    DATARATE,
    FREQ_DEVIATION,
    BANDWIDTH
  };


/**
 * @brief Packet Basic structure fitting
 */
PktBasicInit xBasicInit={
  PREAMBLE_LENGTH,
  SYNC_LENGTH,
  SYNC_WORD,
  LENGTH_TYPE,
  LENGTH_WIDTH,
  CRC_MODE,
  CONTROL_LENGTH,
  EN_ADDRESS,
  EN_FEC,
  EN_WHITENING
};


/**
 * @brief Address structure fitting
 */
PktBasicAddressesInit xAddressInit={
  EN_FILT_MY_ADDRESS,
  MY_ADDRESS,
  EN_FILT_MULTICAST_ADDRESS,
  MULTICAST_ADDRESS,
  EN_FILT_BROADCAST_ADDRESS,
  BROADCAST_ADDRESS
};



/**
 * @brief GPIO IRQ structure fitting
 */
SGpioInit xGpioIRQ={
  SPIRIT_GPIO_3,
  SPIRIT_GPIO_MODE_DIGITAL_OUTPUT_LP,
  SPIRIT_GPIO_DIG_OUT_IRQ
};


/**
 * @brief IRQ status struct declaration
 */
SpiritIrqs xIrqStatus;


/**
 * @brief Rx buffer declaration: how to store the received data
 */
uint8_t vectcRxBuff[96], cRxData;

/**
 *@}
 */



/**
 * @defgroup Basic_Generic_B_Private_Functions                                  Basic Generic B Private Functions
 * @{
 */

/**
 * @brief  This function handles External interrupt request. In this application it is used
 *         to manage the Spirit IRQ configured to be notified on the Spirit GPIO_3.
 * @param  None
 * @retval None
 */
static uint16_t M2S_GPIO_PIN_IRQ;

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
   if(GPIO_Pin==M2S_GPIO_PIN_IRQ)
   {
    /* Get the IRQ status */
    SpiritIrqGetStatus(&xIrqStatus);
    
    /* Check the SPIRIT RX_DATA_DISC IRQ flag */
    if(xIrqStatus.IRQ_RX_DATA_DISC)
    {
      /* toggle LED1 */
      SdkEvalLedToggle(LED1);
      
      /* RX command - to ensure the device will be ready for the next reception */
      SpiritCmdStrobeRx();
    }
    
    /* Check the SPIRIT RX_DATA_READY IRQ flag */
    if(xIrqStatus.IRQ_RX_DATA_READY)
    {
      /* Get the RX FIFO size */
      cRxData=SpiritLinearFifoReadNumElementsRxFifo();
      
      /* Read the RX FIFO */
      SpiritSpiReadLinearFifo(cRxData, vectcRxBuff);
      
      /* Flush the RX FIFO */
      SpiritCmdStrobeFlushRxFifo();
      
      
      /*  A simple way to check if the received data sequence is correct (in this case LED5 will toggle) */
      {
        SpiritBool xCorrect=S_TRUE;
        
        for(uint8_t i=0 ; i<cRxData ; i++)
          if(vectcRxBuff[i] != i+1)
            xCorrect=S_FALSE;
        
        if(xCorrect) {
          /* toggle LED2 */
          SdkEvalLedToggle(LED2);
#ifdef USE_VCOM
          printf("DATA CORRECT\r\n");
#endif
        }
      }
      
      /* RX command - to ensure the device will be ready for the next reception */
      SpiritCmdStrobeRx();
    
#ifdef USE_VCOM
      /* print the received data */
      printf("B data received: [");
      for(uint8_t i=0 ; i<cRxData ; i++)
        printf("%02x ", vectcRxBuff[i]);
      printf("]\r\n");
#endif

    }
   }
}


/**
 * @brief  System main function.
 * @param  None
 * @retval None
 */
void main (void)
{
  HAL_Init();
  SDK_SYSTEM_CONFIG();

#ifdef USE_VCOM
  SdkEvalComInit();
#endif
    
  /* Spirit ON */
  SpiritEnterShutdown();
  SpiritExitShutdown();
  SpiritManagementWaExtraCurrent();
  
  SpiritManagementIdentificationRFBoard();

  /* if the board has eeprom, we can compensate the offset calling SpiritManagementGetOffset
  (if eeprom is not present this fcn will return 0) */
  xRadioInit.lFrequencyBase = xRadioInit.lFrequencyBase + SpiritManagementGetOffset();
  
  /* Initialize the signals to drive the range extender application board */
  SpiritManagementRangeExtInit();  

  /* uC IRQ config */
  SdkEvalM2SGpioInit(M2S_GPIO_3,M2S_MODE_EXTI_IN);
  M2S_GPIO_PIN_IRQ = SdkEvalGpioGetPin(M2S_GPIO_3);
    
  /* Spirit IRQ config */
  SpiritGpioInit(&xGpioIRQ);

  SdkEvalM2SGpioInterruptCmd(M2S_GPIO_3,0x0A,0x0A,ENABLE);

  /* Spirit Radio config */
  SpiritRadioInit(&xRadioInit);
  
  /* Spirit Packet config */
  SpiritPktBasicInit(&xBasicInit);
  SpiritPktBasicAddressesInit(&xAddressInit);

  /* Spirit IRQs enable */
  SpiritIrqDeInit(&xIrqStatus);
  SpiritIrq(RX_DATA_DISC,S_ENABLE);
  SpiritIrq(RX_DATA_READY,S_ENABLE);

  /* payload length config */
  SpiritPktBasicSetPayloadLength(20);

  /* enable SQI check */
  SpiritQiSetSqiThreshold(SQI_TH_0);
  SpiritQiSqiCheck(S_ENABLE);

  /* RX timeout config */
  SpiritTimerSetRxTimeoutMs(1000.0);
  SpiritTimerSetRxTimeoutStopCondition(SQI_ABOVE_THRESHOLD);

  /* IRQ registers blanking */
  SpiritIrqClearStatus();

  /* RX command */
  SpiritCmdStrobeRx();

  printf("pckt!\n\r");
  /* infinite loop */
  while (1){
  }

}



#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param file: pointer to the source file name
  * @param line: assert_param error line source number
  * @retval : None
  */
void assert_failed(uint8_t* file, uint32_t line)
{
  /* User can add his own implementation to report the file name and line number */
     printf("Wrong parameters value: file %s on line %d\r\n", file, line);

  /* Infinite loop */
  while (1)
  {
  }
}
#endif


/**
 *@}
 */

/**
 *@}
 */

/**
 *@}
 */

/**
 *@}
 */


/******************* (C) COPYRIGHT 2016 STMicroelectronics *****END OF FILE****/
