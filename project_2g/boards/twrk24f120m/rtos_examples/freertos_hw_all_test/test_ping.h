/******************** (C) COPYRIGHT 2012 STMicroelectronics ********************
* File Name          : test_ping.h
* Author             : Metering Team, IPD, GC&SA
* Version            : V1.0
* Date               : 2012.JUL
* Description        : Auto-Meter-Reading (AMR) test interfaces definition 
********************************************************************************
* THE PRESENT FIRMWARE WHICH IS FOR GUIDANCE ONLY AIMS AT PROVIDING CUSTOMERS
* WITH CODING INFORMATION REGARDING THEIR PRODUCTS IN ORDER FOR THEM TO SAVE TIME.
* AS A RESULT, STMICROELECTRONICS SHALL NOT BE HELD LIABLE FOR ANY DIRECT,
* INDIRECT OR CONSEQUENTIAL DAMAGES WITH RESPECT TO ANY CLAIMS ARISING FROM THE
* CONTENT OF SUCH FIRMWARE AND/OR THE USE MADE BY CUSTOMERS OF THE CODING
* INFORMATION CONTAINED HEREIN IN CONNECTION WITH THEIR PRODUCTS.
*******************************************************************************/

#ifndef __ST7580_TEST_PING_H_
#define __ST7580_TEST_PING_H_

#include "plc_k24_api.h"
#include "drv_st7580.h"

#define DFT_FREQ_H          (2687)//235kHz (2687)  //85984 Hz
#define DFT_FREQ_L          (2250)//220kHz (2250)  //72000 Hz

#define MTR_ADDR_MAXLEN     (16)
#define APP_FRM_MAXLEN      (251)


#define MTR_ADDR_LEN        (6)
#define MTR_ADDR_SN1        ("\x00\x11\x22\x33\x44\x55")
#define MTR_ADDR_SN2        ("\x00\x01\x02\x03\x04\x05")
#define MTR_ADDR_SN3        ("\x00\x12\x34\x56\x78\x99")


typedef struct {
    TU16        nResetCntr;
    TU16        nLastRunCntr;  
    TU32        nTxCntr; //counter for Drv_DataRequest return succ
    TU32        nTxSuccCntr; //confirm received, result success
    TU32        nTxFailCntr; //confirm received, but result error
    TU32        nRxCntr; //all received frames
    TU32        nRxSuccCntr; //received my frames
    TU32        nRxLostCntr; //buf busy, discard the received frame (didn't decode the frame)
    TU32        nRxErrCntr; //decode error: nRxCntr = nRxLostCntr (not decode) + nRxErrCntr (decode err) + nRxSuccCntr (decode succ, my frame) + frames to others (decode succ)    
}TCommCntr;

typedef struct {  //for WIN32 demo
    TU8 nDILen;
    TU8 szDI[MTR_ADDR_MAXLEN];   
    TU16 nTestTime; //unit:s
}TestPing_SN_TypeDef;

typedef struct { //for WIN32 demo
    TU8 nDILen;
    TU8 szDI[MTR_ADDR_MAXLEN];
    TU16 nTxInterval; //unit:ms
    TU8  nTxLen;
    TU8  szTxData[APP_FRM_MAXLEN];    
    TU16 nTestTime; //unit:s        
}TestPing_BN_TypeDef;

void TEST_PING_BN(TestPing_BN_TypeDef *pBnPara);
void TEST_PING_SN(TestPing_SN_TypeDef *pSnPara);

void TEST_PING_FetchAppCntr(TCommCntr *pAppCntr);
void TEST_PING_ClrAppCntr(void);
void TEST_PING_FrmRcvd(TU8 * pBuf, TU8 nLen);

TBool TEST_AMR_IsHostTrans(void);
void TEST_AMR_COMMON_FSM(void);

void  PING_GetDefaultPlcCfg(PLC_Config_t * ptCfg);
TU8 * PING_GetTxBuffer(void);
void  PING_ReleaseRxBuffer(void);

#endif // __ST7580_TEST_PING_H_

/************* (C) COPYRIGHT 2013 STMicroelectronics ***** END OF FILE *******/
