/**
* @file    SPIRIT_SDK_Util.h
* @author  VMA division - AMS
* @version 3.2.0
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

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef SPIRIT_SDK_UTIL_H_
#define SPIRIT_SDK_UTIL_H_


/* Includes ------------------------------------------------------------------*/
#include "SPIRIT_Config.h"


#ifdef __cplusplus
  "C" {
#endif


/**
 * @addtogroup SPIRIT_DK
 * @{
 */


/**
 * @defgroup SDK_SPIRIT_MANAGEMENT
 * @{
 */


/**
 * @brief  Range extender type
 */
typedef enum 
{
  RESET = 0, 
  SET = !RESET
} FlagStatus, ITStatus;      
      
      
typedef enum
{
  RANGE_EXT_NONE = 0x00,
  RANGE_EXT_SKYWORKS_169,
  RANGE_EXT_SKYWORKS_868
} RangeExtType;

/**
 * @addgroup SDK_SPIRIT_MANAGEMENT_FUNCTIONS
 * @{
 */
void SpiritManagementIdentificationRFBoard(void);
RangeExtType SpiritManagementGetRangeExtender(void);
void SpiritManagementSetRangeExtender(RangeExtType xRangeType);
void SpiritManagementRangeExtInit(void);
void SpiritManagementSetBand(uint8_t value);
uint8_t SpiritManagementGetBand(void);
void SpiritManagementSetOffset(int32_t value);
int32_t SpiritManagementGetOffset(void);

/**
* @}
*/

/**
* @}
*/

/**
* @}
*/

#ifdef __cplusplus
}
#endif


#endif


 /******************* (C) COPYRIGHT 2016 STMicroelectronics *****END OF FILE****/

