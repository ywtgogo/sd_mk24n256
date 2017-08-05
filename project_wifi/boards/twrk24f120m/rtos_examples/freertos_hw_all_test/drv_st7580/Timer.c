/******************** (C) COPYRIGHT 2009 STMicroelectronics ********************/
/**
 * @file    : timer0.c
 * @brief   : platform independent timer implementation (STM32/STM8/WIN32).
 * @author  : Alex Li (alex.li@st.com)
 * @version : V1.0
 * @date    : 05/03/2010
*
**********************************************************************************
*              (C) COPYRIGHT 2009 STMicroelectronics                        <br>
* THE PRESENT SOFTWARE WHICH IS FOR GUIDANCE ONLY AIMS AT PROVIDING CUSTOMERS  
* WITH CODING INFORMATION REGARDING THEIR PRODUCTS IN ORDER FOR THEM TO SAVE TIME.
* AS A RESULT, STMICROELECTRONICS SHALL NOT BE HELD LIABLE FOR ANY DIRECT, 
* INDIRECT OR CONSEQUENTIAL DAMAGES WITH RESPECT TO ANY CLAIMS ARISING FROM THE 
* CONTENT OF SUCH SOFTWARE AND/OR THE USE MADE BY CUSTOMERS OF THE CODING?
* INFORMATION CONTAINED HEREIN IN CONNECTION WITH THEIR PRODUCTS.
**********************************************************************************/

#include "plc_k24_api.h"

//////////////////////////////////////////////////////////////////////////
#define MAX_TIMER_DELAY_TICK    TS32_MAX
//////////////////////////////////////////////////////////////////////////
void TIMER_Init()
{
}

TBool TIMER_SetDelay_ms(Timer_t *timer, Counter_t delay)
{
    if (delay > MAX_TIMER_DELAY_TICK) return TFalse;

    //timer->start = TIMER_GetNow();
    timer->delay = delay;
    timer->flag = 0;

    return TTrue;
}

void TIMER_Start(Timer_t *timer)
{
    //timer->start = TIMER_GetNow();

    timer->flag = 1;
}

void TIMER_Stop(Timer_t *timer)
{
    timer->flag = 0;
}

//TBool TIMER_Elapsed(Timer_t *timer)
//{
//    Counter_t   nTar;
//
//    if (timer->flag == 0) return TFalse;
//
//    nTar = (Counter_t)(timer->start + timer->delay);
//    
//    return (TBool)((Counter_t)(TIMER_GetNow() - nTar) < (((TU32)1)<<30));
//}

TBool TIMER_isStarted(Timer_t *timer)
{
    return (TBool)(timer->flag != 0);
}

//
// !!! do NOT suggest to invoke this function in State-Machine !!! (alex li)
//
void TIMER_Delay(Counter_t nMs)
{
//    Counter_t nStart = TIMER_GetNow();
//    while (((Counter_t)(TIMER_GetNow() - nStart)) < nMs);
}
/************* (C) COPYRIGHT 2012 STMicroelectronics ***** END OF FILE *******/
