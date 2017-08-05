/******************** (C) COPYRIGHT 2010 STMicroelectronics ********************/
/**
 * @file    : st_hal_timer.h
 * @brief   : timer implementation
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

#ifndef __PLC_K24_TIMER_H_
#define __PLC_K24_TIMER_H_

#include "plc_k24_types.h"

#ifdef __cplusplus
extern "C"{
#endif

typedef TU32 Counter_t;

typedef struct {
	Counter_t delay;
	Counter_t start;
	TU8 flag;
} Timer_t;

TBool TIMER_SetDelay_ms(Timer_t *timer, Counter_t delay);
void  TIMER_Start(Timer_t *timer);
void  TIMER_Stop(Timer_t *timer);
TBool TIMER_Elapsed(Timer_t *timer);
TBool TIMER_isStarted(Timer_t *timer);

//Counter_t TIMER_GetNow(void);

#ifdef __cplusplus
}
#endif

#endif //__ST_HAL_TIMER_H_

/************* (C) COPYRIGHT 2010 STMicroelectronics ***** END OF FILE *******/
