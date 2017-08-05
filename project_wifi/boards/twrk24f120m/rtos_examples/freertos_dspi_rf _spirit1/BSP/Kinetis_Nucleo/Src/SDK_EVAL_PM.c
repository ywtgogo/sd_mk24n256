/**
 * @file    SDK_EVAL_PM.c
 * @author  VMA division - AMS
 * @version 3.2.0
 * @date    May 1, 2016
 * @brief   This file provides all the low level API to manage SDK Digipots.
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
#include "SDK_EVAL_PM.h"
#include "SDK_EVAL_Config.h"

/** @addtogroup SDK_EVAL_NUCLEO
 * @{
 */


/** @addtogroup SDK_EVAL_PM
 * @{
 */

/** @defgroup SDK_EVAL_PM_Private_TypesDefinitions      SDK EVAL PM Private Types Definitions
 * @{
 */

/**
 * @}
 */


/** @defgroup SDK_EVAL_PM_Private_Defines               SDK EVAL PM Private Defines
 * @{
 */

/**
 * @}
 */


/** @defgroup SDK_EVAL_PM_Private_Macros                SDK EVAL PM Private Macros
 * @{
 */

#define ABS(a)      (a>0 ? a:-a)


/**
 * @}
 */


/** @defgroup SDK_EVAL_PM_Private_Variables                     SDK EVAL PM Private Variables
 * @{
 */

//static const SdkEvalDigipotAddress s_vectxDigipotAddress[2] = { DIGIPOT1_ADDRESS , DIGIPOT2_ADDRESS};

static I2C_HandleTypeDef I2cHandle;

static ADC_HandleTypeDef    AdcHandle;

/**
 * @}
 */


/**
 * @defgroup SDK_EVAL_PM_Private_FunctionPrototypes             SDK EVAL PM Private Function Prototypes
 * @{
 */

/**
 * @}
 */


/**
 * @defgroup SDK_EVAL_PM_Private_Functions                      SDK EVAL PM Private Functions
 * @{
 */
/**
 * @brief  Configures the I2C interface and the correspondent GPIO pins.
 * @param  None.
 * @retval None.
 */
void SdkEvalPmI2CInit(void)
{
  
}


/**
 * @brief  Writes an 8-bit value in the RDAC register. The potentiometer value in ohm is
 *         given by the formula (cDValue/256*Rab + Rw), where Rab = xxxx and Rw = 60 ohm.
 * @param  xDigipot specifies what digipot has to be set.
 *         This parameter can be DIGIPOT1 or DIGIPOT2.
 * @param  cDValue D value to be written in the RDAC register.
 *         This parameter is an uint8_t.
 * @retval uint8_t Notifies if an I2C error has occured or if the communication has been correctly done.
 *         This parameter can be I2C_DIGIPOT_OK or I2C_DIGIPOT_ERROR.
 */
uint8_t SdkEvalPmDigipotWrite(SdkEvalDigipot xDigipot, uint8_t cDValue)
{
  return 0;
}

/**
 * @brief  Returns the RDAC register. The potentiometer value in ohm is
 *         given by the formula (cDValue/256*Rab + Rw), where Rab = xxxx and Rw = 60 ohm.
 * @param  xDigipot specifies what digipot has to be set.
 *         This parameter can be DIGIPOT1 or DIGIPOT2.
 * @param  pcDValue pointer to the variable in which the D value has to be stored.
 *         This parameter is an uint8_t*.
 * @retval uint8_t Notifies if an I2C error has occured or if the communication has been correctly done.
 *         This parameter can be I2C_DIGIPOT_OK or I2C_DIGIPOT_ERROR.
 */
uint8_t SdkEvalPmDigipotRead(SdkEvalDigipot xDigipot, uint8_t* pcDValue)
{ 
  return 0;
}


/**
 * @brief  Configures the Analog-to-Digital Converter and the correspondent GPIO pins.
 * @param  None.
 * @retval None.
 */
void SdkEvalPmADCInit(void)
{
  
}


/**
 * @brief  Samples, converts and returns the voltage on the specified ADC channel.
 * @param  xAdcCh ADC channel.
 *         This parameter can be any value of @ref SdkEvalAdcChannel.
 * @retval uint16_t Converted voltage in an unsiged integer 16-bit format.
 */
uint16_t SdkEvalPmGetV(SdkEvalAdcChannel xAdcCh)
{
  return 3300;
}

/**
 * @brief  Samples, converts and returns the voltage on the specified ADC channel.
 *         It waits for the value to be constant (with a nError tolerance) before the conversion
 * @param  xAdcCh ADC channel.
 *         This parameter can be any value of @ref SdkEvalAdcChannel.
 * @param  nError tolerance to be used to consider settled the signal to be acquired.
 *         This parameter is a uint16_t.
 * @retval uint16_t Converted voltage in an unsiged integer 16-bit format.
 */
uint16_t SdkEvalPmGetSettledV(SdkEvalAdcChannel xAdcCh , uint16_t nError)
{
  
  return 3300;

}

/**
 * @brief  Implements a control loop to make the specified voltage equal
 *         to a reference one (with a specified tolerance on the steady state).
 *         The control loop ends if the error value is less of cError or if
 *         a number of 7 zero crossing events have been reached by the error.
 * @param  xAdcCh ADC channel.
 *         This parameter can be any value of ADC_CH_V_RF or ADC_CH_V_MCU.
 * @param  fVref Reference voltage to be reached by the specified channel 3
 *         voltage expressed in Volt.
 *         This parameter is a float.
 * @param  cError Desired steady state error expressed in milliVolt.
 *         This parameter is an uint8_t.
 * @retval None.
 */
void SdkEvalPmRegulateVoltage(SdkEvalAdcChannel xAdcCh , float fVref , uint8_t cError)
{
  

}

/**
 * @brief  Implements a control loop to make the specified voltage equal
 *         to a reference one (with a specified tolerance on the steady state).
 *         The control loop vanishes an integral error and it ends when this error
 *         becomes constant.
 * @param  xAdcCh ADC channel.
 *         This parameter can be any value of ADC_CH_V_RF or ADC_CH_V_MCU.
 * @param  fVref Reference voltage to be reached by the specified channel 3
 *         voltage expressed in Volt.
 *         This parameter is a float.
 * @retval None.
 */
void SdkEvalPmRegulateVoltageI(SdkEvalAdcChannel xAdcCh , float fVref)
{
  
}




/**
 * @brief  Configures the RF supply voltage switch GPIO.
 * @param  None.
 * @retval None.
 */
void SdkEvalPmRfSwitchInit(void)
{
  
}


/**
 * @brief  Sets the switch to supply RF voltage.
 * @param  None.
 * @retval None.
 */
void SdkEvalPmRfSwitchToVRf(void)
{
 
}

/**
 * @brief  Sets the switch to supply voltage for the calibration resistor.
 * @param  None.
 * @retval None.
 */
void SdkEvalPmRfSwitchToRcal(void)
{
  
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
