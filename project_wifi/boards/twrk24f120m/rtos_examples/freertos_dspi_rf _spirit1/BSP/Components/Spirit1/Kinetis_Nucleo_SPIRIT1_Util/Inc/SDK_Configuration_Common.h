/**
* @file    SDK_Configuration_Common_Common.h
* @author  VMA division - AMS
* @version 3.2.0
* @date    May 1, 2016
* @brief   Common configuration header file. This file is shared among the SDK examples.
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
#ifndef __SDK_PKT_TEST_COMMON_H
#define __SDK_PKT_TEST_COMMON_H

#define USE_HIGH_BAND

#ifdef __cplusplus
 extern "C" {
#endif

/**
 * @addtogroup SDK_Examples
 * @{
 */

/**
 * @defgroup SDK_Configuration_Common
 * @{
 */

/**
 * @addtogroup SDK_Configuration_Common                                    Configuration Common
 * @brief Radio and packet parameters definitions.
 * @details These parameters are in common between the device A and B.
 *
 * The user can change the configuration parameters editing these defines.
 * @{
 */

/**
 * @defgroup Configuration_Common_Exported_Types                         Configuration Common Exported Types
 * @{
 */

/**
 *@}
 */


/**
 * @defgroup Configuration_Common_Exported_Constants                     Configuration Common Exported Constants
 * @{
 */


/*  Radio configuration parameters  */
#define XTAL_OFFSET_PPM             0

#ifdef USE_VERY_LOW_BAND
#define BASE_FREQUENCY              169.0e6
#endif

#ifdef USE_LOW_BAND
#define BASE_FREQUENCY              315.0e6
#endif

#ifdef USE_MIDDLE_BAND
#define BASE_FREQUENCY              433.0e6
#endif

#ifdef USE_HIGH_BAND
#define BASE_FREQUENCY              865.0e6             //868.0e6
#endif

#define CHANNEL_SPACE               15258               //default:20e3
#define CHANNEL_NUMBER              0//32               //default:0
#define MODULATION_SELECT           FSK
#define DATARATE                    38400
#define FREQ_DEVIATION              20e3
#define BANDWIDTH                   100E3

#define POWER_DBM                   11.6

#define RSSI_THRESHOLD              0x20

/*  Packet configuration parameters  */
#define PREAMBLE_LENGTH             PKT_PREAMBLE_LENGTH_04BYTES
#define SYNC_LENGTH                 PKT_SYNC_LENGTH_4BYTES
#define SYNC_WORD                   0x78877887          //default:0x88888888£¬0x78877887£¬
#define LENGTH_TYPE                 PKT_LENGTH_VAR      //default:PKT_LENGTH_VAR
#define LENGTH_WIDTH                8
#define CRC_MODE                    PKT_NO_CRC          //PKT_CRC_MODE_8BITS
#define CONTROL_LENGTH              PKT_CONTROL_LENGTH_0BYTES
#define EN_ADDRESS                  S_DISABLE
#define EN_FEC                      S_DISABLE
#define EN_WHITENING                S_DISABLE           //S_ENABLE

/*  Addresses configuration parameters  */
#define EN_FILT_MY_ADDRESS          S_DISABLE
#define MY_ADDRESS                  0x34
#define EN_FILT_MULTICAST_ADDRESS   S_DISABLE
#define MULTICAST_ADDRESS           0xEE
#define EN_FILT_BROADCAST_ADDRESS   S_DISABLE
#define BROADCAST_ADDRESS           0xFF
#define DESTINATION_ADDRESS         0x44

/* Wake Up timer in ms for LDC mode */
#define WAKEUP_TIMER                30000.0
/**
 *@}
 */


/**
 * @defgroup Configuration_Common_Exported_Macros                                Configuration Common Exported Macros
 * @{
   */

#ifdef STM8L
/* For STM8L we don't have the SPIRIT_SDK_Util module, so use this macro to set the RANGE EXT config */
typedef enum
{
  RANGE_EXT_NONE = 0x00,
  RANGE_EXT_SKYWORKS_169,
  RANGE_EXT_SKYWORKS_868
} RangeExtType;   


#define RANGE_EXT_INIT(RANGE_TYPE)        {\
   if(RANGE_TYPE==RANGE_EXT_SKYWORKS_169) {\
    SpiritGeneralSetExtRef(MODE_EXT_XIN);\
    uint8_t tmp = 0x01; SpiritSpiWriteRegisters(0xB6,1,&tmp);\
    SpiritGpioInit(&(SGpioInit){SPIRIT_GPIO_0, SPIRIT_GPIO_MODE_DIGITAL_OUTPUT_HP, SPIRIT_GPIO_DIG_OUT_TX_RX_MODE});\
    SpiritGpioInit(&(SGpioInit){SPIRIT_GPIO_1, SPIRIT_GPIO_MODE_DIGITAL_OUTPUT_HP, SPIRIT_GPIO_DIG_OUT_TX_STATE});\
    SpiritGpioInit(&(SGpioInit){SPIRIT_GPIO_2, SPIRIT_GPIO_MODE_DIGITAL_OUTPUT_HP, SPIRIT_GPIO_DIG_OUT_RX_STATE});\
  }else if(RANGE_TYPE==RANGE_EXT_SKYWORKS_868){\
    SpiritGpioInit(&(SGpioInit){SPIRIT_GPIO_0, SPIRIT_GPIO_MODE_DIGITAL_OUTPUT_HP, SPIRIT_GPIO_DIG_OUT_TX_RX_MODE});\
    SpiritGpioInit(&(SGpioInit){SPIRIT_GPIO_1, SPIRIT_GPIO_MODE_DIGITAL_OUTPUT_HP, SPIRIT_GPIO_DIG_OUT_RX_STATE});\
    SpiritGpioInit(&(SGpioInit){SPIRIT_GPIO_2, SPIRIT_GPIO_MODE_DIGITAL_OUTPUT_HP, SPIRIT_GPIO_DIG_OUT_TX_STATE});\
  }\
 }
#endif
/**
 *@}
 */


/**
 * @defgroup Configuration_Common_Exported_Functions                             Configuration Common Exported Functions
 * @{
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

/**
 *@}
 */

#ifdef __cplusplus
}
#endif

#endif

/******************* (C) COPYRIGHT 2016 STMicroelectronics *****END OF FILE****/
