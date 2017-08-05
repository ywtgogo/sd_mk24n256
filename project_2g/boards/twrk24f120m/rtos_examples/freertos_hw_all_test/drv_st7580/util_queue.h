/******************** (C) COPYRIGHT 2010 STMicroelectronics ********************/
/**
 * @file    : util_queue.h
 * @brief   : macro based queue implementation (fixed length)
 * @author  : Alex Li (alex.li@st.com)
 * @version : V1.0
 * @date    : 06/02/2010
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

#include "plc_k24_types.h"

#ifndef _ST_UTIL_QUEUE_H_
#define _ST_UTIL_QUEUE_H_

#define ST_QUEUE_LEN        (64)
#define ST_QUEUE_MASK       (0x3F)

typedef struct {
    TU8 head;
    TU8 tail;
    TU8 buf[ST_QUEUE_LEN];
}TPlcQueue;

#define Q_INIT(q)       ((q).head = (q).tail = 0)
#define Q_GET_L(q)      (((q).tail + ST_QUEUE_LEN - (q).head) & ST_QUEUE_MASK )
#define Q_GET_E(q)      (ST_QUEUE_LEN - 1 - Q_GET_L(q))
#define Q_EMPTY(q)      Q_INIT(q)
#define Q_READHEAD(q,r) { (r) = (q).buf[(q).head]; (q).head = ((q).head + 1) & ST_QUEUE_MASK; }
#define Q_PUSHTAIL(q,v) { (q).buf[(q).tail] = (v); (q).tail = ((q).tail + 1) & ST_QUEUE_MASK; }

#endif // _ST_UTIL_QUEUE_H_

/************* (C) COPYRIGHT 2010 STMicroelectronics ***** END OF FILE *******/
