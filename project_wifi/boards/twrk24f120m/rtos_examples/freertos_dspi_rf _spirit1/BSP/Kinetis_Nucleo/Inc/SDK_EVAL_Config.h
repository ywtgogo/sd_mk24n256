/**
 * @file    SDK_EVAL_Config.h
 * @author  VMA division - AMS
 * @version V2.0.2
 * @date    Febrary 7, 2015
 * @brief   This file contains SDK EVAL configuration and useful defines.
 * @details
 *
 * This file is used to include all or a part of the SDK Eval
 * libraries into the application program which will be used.
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
#ifndef __SDK_EVAL_CONFIG_H
#define __SDK_EVAL_CONFIG_H

/* Includes ------------------------------------------------------------------*/
//#include "SDK_EVAL_Button.h"
//#include "SDK_EVAL_Com.h"
//#include "SDK_EVAL_Led.h"
//#include "SDK_EVAL_PM.h"
#include "SDK_EVAL_Gpio.h"
#include "SDK_EVAL_Timers.h"
//#include "SDK_EVAL_EEPROM.h"

#ifdef __cplusplus
extern "C" {
#endif


/** @addtogroup SDK_EVAL_STM32L        SDK EVAL STM32L
 * @brief This module is used to configure the SDK Eval board and
 * allows to manage its peripherals in a simple way.
 * @details The demoboards supported are:
 * <ul><li>STEVAL-IKR002Vx - RF motherboard<ul> 
 * <li>STM32L151RBT6 64-pin microcontroller</li>
 * <li>LIS3DH accelerometer</li>
 * <li>STLM75 temperature sensor</li>
 * <li>Mini USB connector for power supply and I/O</li>
 * <li>RF daughterboard interface</li>
 * <li>JTAG/SWD connector</li>
 * <li>Joystic</li>
 * <li>User button and RESET button</li>
 * <li>User LEDs</li>
 * <li>UART connector</li>
 * <li>Battery holder for 2 AAA batteries</li>
 * </ul></li></ul>
 * <ul><li>STEVAL-IDS001Vx - RF motherboard<ul> 
 * <li>STM32L151CBU6 48-pin microcontroller</li>
 * <li>Chip antenna</li>
 * <li>UFL connector</li>
 * <li>Mini USB connector for power supply and I/O</li>
 * <li>RF daughterboard interface</li>
 * <li>SWD connector</li>
 * <li>Two user buttons</li>
 * <li>User LEDs</li>
  * </ul></li></ul>
 * <ul><li>STEVAL-IKR001Vx - RF motherboard<ul> 
 * <li>STM32L152VB 128-pin microcontroller</li>
 * <li>Mini USB connector for power supply and I/O</li>
 * <li>RF daughterboard interface</li>
 * <li>JTAG/SWD connector</li>
 * <li>Joystic</li>
 * <li>Two user buttons and RESET button</li>
 * <li>User LEDs</li>
 * <li>UART connector</li>
 * <li>Digital potentiometer connected via I2C used to regulate the supply voltage</li>
 * </ul></li></ul>
 * @{
 */

/** @addtogroup SDK_EVAL_Config         SDK EVAL Config
 * @brief SDK EVAL configuration.
 * @details See the file <i>@ref SDK_EVAL_Config.h</i> for more details.
 * @{
 */

/** @addtogroup SDK_EVAL_Config_Exported_Types          SDK EVAL Config Exported Types
 * @{
 */

/**
 * @}
 */

/** @defgroup SDK_EVAL_Config_Exported_Constants        SDK EVAL Config Exported Constants
 * @{
 */

#define SDK_EVAL_VERSION_2_1    0
#define SDK_EVAL_VERSION_3      1
#define SDK_EVAL_VERSION_D1     2
  

#define SDK_EVAL_SPI_PRESCALER	SPI_BAUDRATEPRESCALER_4
#define SDK_EVAL_UART_BAUDRATE  115200
/**
 * @}
 */


/** @defgroup SDK_EVAL_Config_Exported_Macros           SDK EVAL Config Exported Macros
 * @{
 */


/**
 * @}
 */


/** @defgroup SDK_EVAL_Config_Exported_Functions        SDK EVAL Config Exported Functions
 * @{
 */

void SdkEvalIdentification(void);
uint8_t SdkEvalGetVersion(void);
   
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
