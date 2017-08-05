/******************** (C) COPYRIGHT 2009 STMicroelectronics ********************/
/**
 * @file    : st_hal_types.h
 * @brief   : Definition of hardware abstract layer.
 * @author  : Alex Li (alex.li@st.com)
 * @version : V1.0
 * @date    : 07/16/2009
 *
 **********************************************************************************
 *              (C) COPYRIGHT 2008 STMicroelectronics                        <br>
 * THE PRESENT SOFTWARE WHICH IS FOR GUIDANCE ONLY AIMS AT PROVIDING CUSTOMERS  
 * WITH CODING INFORMATION REGARDING THEIR PRODUCTS IN ORDER FOR THEM TO SAVE TIME.
 * AS A RESULT, STMICROELECTRONICS SHALL NOT BE HELD LIABLE FOR ANY DIRECT, 
 * INDIRECT OR CONSEQUENTIAL DAMAGES WITH RESPECT TO ANY CLAIMS ARISING FROM THE 
 * CONTENT OF SUCH SOFTWARE AND/OR THE USE MADE BY CUSTOMERS OF THE CODING?
 * INFORMATION CONTAINED HEREIN IN CONNECTION WITH THEIR PRODUCTS.
 **********************************************************************************
*/

#ifndef _PLC_K24_TYPES_
#define _PLC_K24_TYPES_

#ifdef __cplusplus
extern "C"{
#endif

typedef unsigned long   TU32;
typedef   signed long   TS32;
typedef unsigned short  TU16;
typedef   signed short  TS16;
typedef unsigned char   TU8;
typedef   signed char   TS8;
typedef          char   TChar;
typedef unsigned char   TBool;
typedef          void   TVoid;
typedef         float   TFloat;
typedef        double   TDouble;

#define TFalse          ((TBool)0)
#define TTrue           ((TBool)(!TFalse))

#define TU8_MAX         ((TU8)255)
#define TS8_MAX         ((TS8)127)
#define TS8_MIN         ((TS8)-128)
#define TU16_MAX        ((TU16)0xFFFF)
#define TS16_MAX        ((TS16)32767)
#define TS16_MIN        ((TS16)-32768)
#define TU32_MAX        ((TU32)4294967295uL)
#define TS32_MAX        ((TS32)2147483647)
#define TS32_MIN        ((TS32)-2147483648)

#ifndef NULL
#define NULL    (0)
#endif


#ifdef __cplusplus
}
#endif

#endif /* _ST_HAL_TYPES_ */

/************* (C) COPYRIGHT 2009 STMicroelectronics ***** END OF FILE *******/
