/******************** (C) COPYRIGHT 2010 STMicroelectronics ********************/
/**
 * @file    : st_hal_error.h
 * @brief   : error code definition
 * @author  : Alex Li (alex.li@st.com)
 * @version : V1.0
 * @date    : 06/12/2010
*
**********************************************************************************
*              (C) COPYRIGHT 2010 STMicroelectronics                        <br>
* THE PRESENT SOFTWARE WHICH IS FOR GUIDANCE ONLY AIMS AT PROVIDING CUSTOMERS  
* WITH CODING INFORMATION REGARDING THEIR PRODUCTS IN ORDER FOR THEM TO SAVE TIME.
* AS A RESULT, STMICROELECTRONICS SHALL NOT BE HELD LIABLE FOR ANY DIRECT, 
* INDIRECT OR CONSEQUENTIAL DAMAGES WITH RESPECT TO ANY CLAIMS ARISING FROM THE 
* CONTENT OF SUCH SOFTWARE AND/OR THE USE MADE BY CUSTOMERS OF THE CODING?
* INFORMATION CONTAINED HEREIN IN CONNECTION WITH THEIR PRODUCTS.
**********************************************************************************
*/

/* Define to prevent recursive inclusion ----------------------*/
#ifndef __PLC_K24_ERRORS_H_
#define __PLC_K24_ERRORS_H_

//! Success code
#define ERR_SUCCESS		0x00
// Free code 0x01
//! Wrong parameters length: too much or not enough.
#define ERR_WPL         0x02    
//! Wrong parameters value: one or more parameter are invalid
#define ERR_WPV         0x03
//! Target sub system is busy, operation can't be performed
#define ERR_BUSY        0x04
// Free code 0x05
//! Target sub system is not present
#define ERR_NOT_PRESENT	0x06
//! Target sub system is disabled
#define ERR_DISABLED	0x07
//! Timeout while executing
#define ERR_TIMEOUT 	0x08
//! Duplicate message
#define ERR_DUPLICATE   0x09
//! No free record in tx transaction table
#define ERR_NOFREE      0x0A
//! Temperature is too hot!
#define ERR_THERMAL_ERR	0x0B
//! Poll state error, maybe chip not available
#define ERR_CHIP_FAULT  0x0C
//! Generic error
#define ERR_ERROR       0xFF

#endif //__ST_HAL_ERRORS_H_

/************* (C) COPYRIGHT 2010 STMicroelectronics ***** END OF FILE *******/
