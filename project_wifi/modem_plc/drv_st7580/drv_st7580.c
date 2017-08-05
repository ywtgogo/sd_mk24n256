/******************** (C) COPYRIGHT 2014 STMicroelectronics ********************/
/**
 * @file    : drv_st7580.c
 * @brief   : platform independent ST7580 driver implementation(STM32/STM8/WIN32).
 * @author  : Metering Team, IPD, GC&SA
 * @version : V1.0
 * @date    : 06/20/2014
*
**********************************************************************************
*              (C) COPYRIGHT 2014 STMicroelectronics                        <br>
* THE PRESENT SOFTWARE WHICH IS FOR GUIDANCE ONLY AIMS AT PROVIDING CUSTOMERS  
* WITH CODING INFORMATION REGARDING THEIR PRODUCTS IN ORDER FOR THEM TO SAVE TIME.
* AS A RESULT, STMICROELECTRONICS SHALL NOT BE HELD LIABLE FOR ANY DIRECT, 
* INDIRECT OR CONSEQUENTIAL DAMAGES WITH RESPECT TO ANY CLAIMS ARISING FROM THE 
* CONTENT OF SUCH SOFTWARE AND/OR THE USE MADE BY CUSTOMERS OF THE CODING?
* INFORMATION CONTAINED HEREIN IN CONNECTION WITH THEIR PRODUCTS.
**********************************************************************************
*/

/* Includes ------------------------------------------------------------------*/
#include "FreeRTOS.h"
#include "task.h"

#include "fsl_debug_console.h"

#include "plc_k24_types.h"
#include "plc_k24_error.h"
#include "plc_k24_timer.h"

#include "uartplc_k24.h"
#include "drv_st7580.h"
#include "drv_st7580_fsm.h"
#include <string.h>


//////////////////////////////////////////////////////////////////////////
TDrvST7580  g_tDrv7580;


static const TU8 g_MibObjSize[] = { 1, 14, 16, 1, 4, 5, 10, 8, 10, 3, 4 };

#ifdef START_ST7580_DEBUG
static TU16 g_tDrvDebug[DEBUG_MAXCNT];
#endif

//////////////////////////////////////////////////////////////////////////
static TVoid Increase_DrvDbgCntr(TU8 nIndex)
{
#ifdef START_ST7580_DEBUG
    g_tDrvDebug[nIndex]++; 
#endif  
}

#include "queue.h"
extern QueueHandle_t plc_pkgs_queue;
extern uint32_t CheckPlcStatus(void);
extern void DrvPlcFsm(void*);
static TU8 Send_PlcFrame(ST7580_SendFrameParaTypedef *pSendFrame)
{
    PLC_RQ_MSG          plc_rq_msg;
    if (TIMER_isStarted(&g_tDrv7580.tTimerWaitCode))        
        return ERR_BUSY;

    if (!CheckPlcStatus())
        return ERR_BUSY;
              
    g_tDrv7580.nFrmCmd = pSendFrame->nCmd;
    g_tDrv7580.pFrmPtr = pSendFrame->pBuf;
    g_tDrv7580.nFrmLen = pSendFrame->nLen;
    g_tDrv7580.nFrmWaitCode = pSendFrame->nWaitCode;
    g_tDrv7580.nFrmErrCode = pSendFrame->nErrCode;
    g_tDrv7580.nFrmCur = 0;
    g_tDrv7580.nFrmCS16 = 0;
    g_tDrv7580.nFrmAck = ACK;

    g_tDrv7580.nStat = PD_TX;
    plc_rq_msg.cmd  =  MQ_TX;
    
    vTaskDelay(100);
    xQueueSend(plc_pkgs_queue, ( void * )&plc_rq_msg, ( TickType_t ) 10U );
    vTaskDelay(5);
    PRINTF("wait event bit from TX frame!\r\n");
    vTaskDelay(200);//wait event bit
    PRINTF("wait event bit timeout!\r\n");
//	    DrvPlcFsm();    //发送
//
//      DrvPlcFsm();    //12.25 add check ack
//        
//	    DrvPlcFsm();    //等待ConfirmCode
//
//      TIMER_Stop(&g_tDrv7580.tTimerWaitCode);
//
//	    //DrvPlcFsm();    //转到接收模式      //需要由其它Task执行等待

    return (g_tDrv7580.nFrmAck == ACK) ? (ERR_SUCCESS) : (ERR_ERROR);   //目前返回为ERROR
}

static void Switch_HighLowByte(TU8 *pBuf, TU8 offset)
{
    TU8  nTemp;    
    
    nTemp = *(pBuf+offset);
    *(pBuf+offset) = *(pBuf+offset+1);
    *(pBuf+offset+1) = nTemp;    
}

#ifdef WIN32
extern TVoid Disp_PlcMsg(TU8 nCmd, TU8 *pData, TU8 nDataLen);
#endif
static void Handle_PlcMsg(void)
{
    if ( (g_tDrv7580.nFrmCmd == PHY_DataIndication_CODE) || \
              (g_tDrv7580.nFrmCmd == DL_DataIndication_CODE) || \
              (g_tDrv7580.nFrmCmd == SS_DataIndication_CODE) || \
              (g_tDrv7580.nFrmCmd == DL_SnifferIndication_CODE) || (g_tDrv7580.nFrmCmd == SS_SnifferIndication_CODE) )
    { // Handle data indication       
        // notify up-layer   
        if (g_tDrv7580.pFrmPtr != NULL)   
        {
            Switch_HighLowByte(g_tDrv7580.pFrmPtr, 2); 
            memcpy((TU8 *)&g_tDrv7580.tInfo.RX_Information, g_tDrv7580.pFrmPtr, sizeof(PLC_RX_Information_t));
            g_tDrv7580.nFrmLen -= sizeof(PLC_RX_Information_t);
            g_tDrv7580.pFrmPtr += sizeof(PLC_RX_Information_t);
            ST7580_DataIndication(g_tDrv7580.pFrmPtr, g_tDrv7580.nFrmLen, 0);
        }
        g_tDrv7580.nChipStatus = PLC_IDLE;   
        
        Increase_DrvDbgCntr(DATAINDICATION_CNT);
    }
    else if(g_tDrv7580.nFrmCmd == BIO_ResetIndication_CODE)
    {
        g_tDrv7580.nChipStatus = PLC_NOT_CONFIGURED;
        if (g_tDrv7580.pFrmPtr != NULL)
            g_tDrv7580.nResetCode =  g_tDrv7580.pFrmPtr[0]; 
    }
    
    else if( (g_tDrv7580.nFrmCmd == g_tDrv7580.nFrmErrCode) || (g_tDrv7580.nFrmCmd == g_tDrv7580.nFrmWaitCode) )
    {
        g_tDrv7580.nFrmAck = (g_tDrv7580.nFrmCmd == g_tDrv7580.nFrmWaitCode) ? ACK : NAK;

        if ( (g_tDrv7580.nFrmWaitCode == PHY_DataConfirm_CODE) || \
             (g_tDrv7580.nFrmWaitCode == DL_DataConfirm_CODE) || (g_tDrv7580.nFrmWaitCode == SS_DataConfirm_CODE) )
        {
            g_tDrv7580.nChipStatus = PLC_IDLE;  
            if(g_tDrv7580.nFrmAck == ACK)
            {
                Switch_HighLowByte(g_tDrv7580.pFrmPtr, 3); 
                memcpy((TU8 *)&g_tDrv7580.tInfo.TX_Information, g_tDrv7580.pFrmPtr, sizeof(PLC_TX_Information_t));
                if (g_tDrv7580.pTxCb != NULL)                
                    g_tDrv7580.pTxCb(ERR_SUCCESS);
                Increase_DrvDbgCntr(TXSUCC_CNT);
            }
            else
            {
                if (g_tDrv7580.pTxCb != NULL)                 
                    g_tDrv7580.pTxCb(ERR_ERROR);
                Increase_DrvDbgCntr(TXERR_CNT2);                
            }                         
            g_tDrv7580.pTxCb = NULL;
            //send event bit TX frame foo
            PRINTF("send event bit to TX frame!\r\n");
        }

        // stop the waiting code timer
        TIMER_Stop(&g_tDrv7580.tTimerWaitCode);

        g_tDrv7580.nFrmWaitCode = 0;
    }
    g_tDrv7580.pFrmPtr = NULL;
}
static TBool Update_PlcInfo(void)
{  
    ST7580_PhyDataTypeDef tPhyData;  
    
    if (Drv_MibReadRequest(MIB_OBJ_ID_PHY_DATA, (TU8 *)&tPhyData) != ERR_SUCCESS) return TFalse;   
    Switch_HighLowByte((TU8 *)&tPhyData, 0); 
    Switch_HighLowByte((TU8 *)&tPhyData, 2);
    Switch_HighLowByte((TU8 *)&tPhyData, 4);
    Switch_HighLowByte((TU8 *)&tPhyData, 6);
    Switch_HighLowByte((TU8 *)&tPhyData, 8);
    memcpy((TU8 *)&(g_tDrv7580.tInfo.Statistics), (TU8 *)&tPhyData, sizeof(ST7580_PhyDataTypeDef));
    return TTrue;
}
//////////////////////////////////////////////////////////////////////////
TU8 Drv_MibReadRequest(TU8 nObjId, TU8 * pBuf)
{
    TU8  nRet;
    ST7580_SendFrameParaTypedef tSendFrame;
    
    //if (g_tDrv7580.nStat != PD_IDLE)
    if (g_tDrv7580.nStat != PD_IDLE && g_tDrv7580.nStat != PD_TREQ_W) 
        return ERR_BUSY;
    
    //    LOG(STLOG_DEBUG, "MIB_ReadRequest()");
    
    g_tDrv7580.tFrmBuf[0] = nObjId;
    
    tSendFrame.nCmd = MIB_ReadRequest_CODE;
    tSendFrame.pBuf = g_tDrv7580.tFrmBuf;
    tSendFrame.nLen = 1;
    tSendFrame.nWaitCode = MIB_ReadConfirm_CODE;
    tSendFrame.nErrCode = MIB_ReadError_CODE;
    tSendFrame.nTmout = ST7580_WAIT_CFM;
    g_tDrv7580.nLenTxPara = 0;
    nRet = Send_PlcFrame(&tSendFrame);
    
    if (nRet == ERR_SUCCESS && pBuf != NULL)
    {
        memcpy(pBuf, g_tDrv7580.tFrmBuf, g_MibObjSize[nObjId]);
    }
    
    return nRet;
}
TU8 Drv_MibEraseRequest(TU8 nObjId)
{   
    ST7580_SendFrameParaTypedef tSendFrame;
    
    if (g_tDrv7580.nStat != PD_IDLE && g_tDrv7580.nStat != PD_TREQ_W) 
        return ERR_BUSY;
    
    //    LOG(STLOG_DEBUG, "MIB_ReadRequest()");
    
    g_tDrv7580.tFrmBuf[0] = nObjId;
    
    tSendFrame.nCmd = MIB_EraseRequest_CODE;
    tSendFrame.pBuf = g_tDrv7580.tFrmBuf;
    tSendFrame.nLen = 1;
    tSendFrame.nWaitCode = MIB_EraseConfirm_CODE;
    tSendFrame.nErrCode = MIB_EraseError_CODE;
    tSendFrame.nTmout = ST7580_WAIT_CFM;
    g_tDrv7580.nLenTxPara = 0;        
    return Send_PlcFrame(&tSendFrame);        
}
TU8 Drv_MibWriteRequest(TU8 nObjId, TU8 * pBuf) 
{
    ST7580_SendFrameParaTypedef tSendFrame;
    
    //if (g_tDrv7580.nStat != PD_IDLE)
    if (g_tDrv7580.nStat != PD_IDLE && g_tDrv7580.nStat != PD_TREQ_W) 
        return ERR_BUSY;
    
    g_tDrv7580.tFrmBuf[0] = nObjId;

    if (pBuf != NULL)
    {
        memcpy(g_tDrv7580.tFrmBuf + 1, pBuf, g_MibObjSize[nObjId]);
    }
    tSendFrame.nCmd = MIB_WriteRequest_CODE;
    tSendFrame.pBuf = g_tDrv7580.tFrmBuf;
    tSendFrame.nLen = (TU8)(g_MibObjSize[nObjId]+1);
    tSendFrame.nWaitCode = MIB_WriteConfirm_CODE;
    tSendFrame.nErrCode = MIB_WriteError_CODE;
    tSendFrame.nTmout = ST7580_WAIT_CFM;
    g_tDrv7580.nLenTxPara = 0;     
    return Send_PlcFrame(&tSendFrame);
}

TU8 ST7580_PingRequest(TU8 nLen, TU8 * pBuf) 
{ 
    TU8  nRet;
    ST7580_SendFrameParaTypedef tSendFrame;
        
    if ((pBuf == NULL) || (nLen > ST7580_MIBBUF_LEN) || (nLen == 0)) return ERR_WPL;
    
    if (g_tDrv7580.nStat != PD_IDLE && g_tDrv7580.nStat != PD_TREQ_W) 
        return ERR_BUSY;
    
    memcpy(g_tDrv7580.tFrmBuf, pBuf, nLen);
              
    tSendFrame.nCmd = PingRequest_CODE;
    tSendFrame.pBuf = g_tDrv7580.tFrmBuf;
    tSendFrame.nLen = nLen;
    tSendFrame.nWaitCode = PingConfirm_CODE;
    tSendFrame.nErrCode = PingConfirm_CODE;
    tSendFrame.nTmout = ST7580_WAIT_CFM;
    g_tDrv7580.nLenTxPara = 0;      
    nRet = Send_PlcFrame(&tSendFrame);
    
    if (nRet == ERR_SUCCESS) 
    {
        if(memcmp(pBuf, g_tDrv7580.tFrmBuf, nLen))
            return ERR_ERROR;
    }    
    return nRet;    
}

TU8 ST7580_BioResetRequest(void)
{
    ST7580_SendFrameParaTypedef tSendFrame;
    if (g_tDrv7580.nStat != PD_IDLE && g_tDrv7580.nStat != PD_TREQ_W) 
        return ERR_BUSY;
    
    tSendFrame.nCmd = BIO_ResetRequest_CODE;
    tSendFrame.pBuf = g_tDrv7580.tFrmBuf;
    tSendFrame.nLen = 0;
    tSendFrame.nWaitCode = BIO_ResetConfirm_CODE;
    tSendFrame.nErrCode = BIO_ResetError_CODE;
    tSendFrame.nTmout = ST7580_WAIT_CFM;
    g_tDrv7580.nLenTxPara = 0;     
    return Send_PlcFrame(&tSendFrame);  
}

TU8 ST7580_ChkConfigure(PLC_Config_t *pConfig) //check if same, configure if not same
{
    ST7580_ModemCfgTypeDef tModemCfg;
    ST7580_PhyCfgTypeDef tPhyCfg;
    TU32  nFreqH, nFreqL;      
    TU8   wait_ret;
    
    wait_ret = Drv_MibReadRequest(MIB_OBJ_ID_MDM_CFG, &tModemCfg.as_byte);
    if(wait_ret == ERR_SUCCESS)
    {
        if((tModemCfg.as_fields.AcesMode != 1) || (tModemCfg.as_fields.SniffMode != pConfig->RX_Settings.as_fields.SnifferMode) || \
           (tModemCfg.as_fields.CrcLen != 2) || (tModemCfg.as_fields.Pad1 != 0) || (tModemCfg.as_fields.CrcCon != 1) || (tModemCfg.as_fields.Pad2 != 0))  
            return Drv_Configure(pConfig);  
          
        wait_ret = Drv_MibReadRequest(MIB_OBJ_ID_PHY_CFG, (TU8 *)&tPhyCfg);
        if(wait_ret == ERR_SUCCESS)
        {
            nFreqH = ((TU32)(pConfig->fHigh))<<5; 
            nFreqL = ((TU32)(pConfig->fLow))<<5;
            if((tPhyCfg.HighHF != (TU8)(nFreqH>>16)) || (tPhyCfg.MidHF != (TU8)((nFreqH>>8)&0xff)) || (tPhyCfg.LowHF != (TU8)(nFreqH&0xff)) || \
               (tPhyCfg.HighLF != (TU8)(nFreqL>>16)) || (tPhyCfg.MidLF != (TU8)((nFreqL>>8)&0xff)) || (tPhyCfg.LowLF != (TU8)(nFreqL&0xff)) || \
               (tPhyCfg.RxChl != pConfig->RX_Settings.as_fields.RxChannelSelector) || (tPhyCfg.RxModuHigh != pConfig->RX_Settings.as_fields.RxHighChannelModulation) || \
               (tPhyCfg.RxModuLow != pConfig->RX_Settings.as_fields.RxLowChannelModulation) || (tPhyCfg.IsCurrentCtrl != 1) || (tPhyCfg.TxGain != pConfig->TX_Settings.TransmitterGain) || \
               (tPhyCfg.Pad1 != 0) || (tPhyCfg.Pad2 != 0) || (tPhyCfg.HighTxZCDelay != (TU8)((pConfig->TX_Settings.TransmitDelay>>8)&0xff)) || \
               (tPhyCfg.LowTxZCDelay != (TU8)((pConfig->TX_Settings.TransmitDelay)&0xff)) || (tPhyCfg.PskPreambleLen != pConfig->RX_Settings.as_fields.PSKModulationPreambleLength) || \
               (tPhyCfg.Pad3 != 0) || (tPhyCfg.FskBaud != pConfig->FSK_Settings.as_fields.BaudRate) || (tPhyCfg.FskDeviation != pConfig->FSK_Settings.as_fields.Deviation) || \
               (tPhyCfg.FskPreambleLen != pConfig->FSK_Settings.as_fields.PreambleLength) || (tPhyCfg.FskUWLen != pConfig->FSK_Settings.as_fields.HeaderLength) || \
               (tPhyCfg.Pad4 != 0) || (tPhyCfg.FskUwMsb != (TU8)(((pConfig->FskUW)>>8)&0xff)) || (tPhyCfg.FskUwLsb != (TU8)((pConfig->FskUW)&0xff)))             
                return Drv_Configure(pConfig);  
        }            
    }
    return wait_ret;
}

TU16 ST7580_SamePhRcvZcDelay(void)//only for PSK mode
{
    TU16 i;
    
    switch(g_tDrv7580.tRxSet.as_fields.PSKModulationPreambleLength)
    {
    case BIT_16: i = 385; break; //PRE+UW time expressed with 13us
    case BIT_24: i = 449; break;
    case BIT_32: i = 513; break;
    case BIT_40: i = 577; break;
    }
    return i;     
}
TU8 Drv_Configure(PLC_Config_t *pConfig)
{
    ST7580_ModemCfgTypeDef tModemCfg;
    ST7580_PhyCfgTypeDef tPhyCfg;
    TU32  nFreq;      
    TU8   wait_ret;
        
    tModemCfg.as_fields.AcesMode = 1;  
    tModemCfg.as_fields.SniffMode = pConfig->RX_Settings.as_fields.SnifferMode;
    tModemCfg.as_fields.CrcLen = 2;
    tModemCfg.as_fields.Pad1 = 0;//1//0  
    tModemCfg.as_fields.CrcCon = 0;//1;  
    tModemCfg.as_fields.Pad2 = 0;
         
    wait_ret = Drv_MibWriteRequest(MIB_OBJ_ID_MDM_CFG, &tModemCfg.as_byte);
    if(wait_ret == ERR_SUCCESS)
    { 
        nFreq = ((TU32)(pConfig->fHigh))<<5;            
        tPhyCfg.HighHF = (TU8)(nFreq>>16);      //01
        tPhyCfg.MidHF = (TU8)((nFreq>>8)&0xff); //4f
        tPhyCfg.LowHF = (TU8)(nFreq&0xff);      //f0
        nFreq = ((TU32)(pConfig->fLow))<<5;          
        tPhyCfg.HighLF = (TU8)(nFreq>>16);      
        tPhyCfg.MidLF = (TU8)((nFreq>>8)&0xff); 
        tPhyCfg.LowLF = (TU8)(nFreq&0xff); 
        tPhyCfg.RxChl = pConfig->RX_Settings.as_fields.RxChannelSelector;
        tPhyCfg.RxModuHigh = pConfig->RX_Settings.as_fields.RxHighChannelModulation;
        tPhyCfg.RxModuLow = pConfig->RX_Settings.as_fields.RxLowChannelModulation;
        tPhyCfg.IsCurrentCtrl = 1;
        tPhyCfg.TxGain = pConfig->TX_Settings.TransmitterGain;
        tPhyCfg.Pad1 = 0;
        tPhyCfg.Pad2 = 0;
        tPhyCfg.HighTxZCDelay = (TU8)((pConfig->TX_Settings.TransmitDelay>>8)&0xff);
        tPhyCfg.LowTxZCDelay = (TU8)((pConfig->TX_Settings.TransmitDelay)&0xff);
        tPhyCfg.PskPreambleLen = pConfig->RX_Settings.as_fields.PSKModulationPreambleLength;        
        tPhyCfg.Pad3 = 0;
        tPhyCfg.FskBaud = pConfig->FSK_Settings.as_fields.BaudRate;
        tPhyCfg.FskDeviation = pConfig->FSK_Settings.as_fields.Deviation;
        tPhyCfg.FskPreambleLen = pConfig->FSK_Settings.as_fields.PreambleLength;
        tPhyCfg.FskUWLen = pConfig->FSK_Settings.as_fields.HeaderLength;
        tPhyCfg.Pad4 = 0;
        tPhyCfg.FskUwMsb = (TU8)(((pConfig->FskUW)>>8)&0xff);
        tPhyCfg.FskUwLsb = (TU8)((pConfig->FskUW)&0xff);
        wait_ret = Drv_MibWriteRequest(MIB_OBJ_ID_PHY_CFG, (TU8 *)&tPhyCfg);
        if (wait_ret == ERR_SUCCESS)
        {                                        
            g_tDrv7580.nTxModulation = pConfig->TX_Settings.Modulation;
            g_tDrv7580.nTxChannelSelector = pConfig->TX_Settings.ChannelSelector;
            g_tDrv7580.nTxZeroCrossingSynchronous = pConfig->TX_Settings.ZeroCrossingSynchronous;
            g_tDrv7580.tRxSet.as_fields.PSKModulationPreambleLength = tPhyCfg.PskPreambleLen;

            // set status to configured
            g_tDrv7580.nChipStatus = PLC_IDLE;
        }                      
    }

    if(wait_ret != ERR_SUCCESS) return ERR_BUSY; 

    return ERR_SUCCESS;          
}

TU8 Drv_DataRequest(PLC_TX_Settings_t *control, TU8 *pBuf, TU8 nLen, void (*confirm)(TU8))
{  
    ST7580_TxSettingTypeDef tTxSettings;
    ST7580_SendFrameParaTypedef tSendFrame;
    ST7580_PhyCfgTypeDef tPhyCfg;
    TU8   wait_ret;    
    
    if(nLen > ST7580_TRANSMISSION_MAXLEN) return ERR_WPL;
    if (g_tDrv7580.nStat != PD_IDLE && g_tDrv7580.nStat != PD_TREQ_W)
    {
    	PRINTF("DataRequest ERR_BUSY %d\r\n", g_tDrv7580.nStat);
        return ERR_BUSY;
    }
         
    tTxSettings.as_fields.FreqSel = 0;
    tTxSettings.as_fields.IfWrtFreq = 0;
    if (control == NULL)
    {
        tTxSettings.as_fields.FreqChl = g_tDrv7580.nTxChannelSelector;
        tTxSettings.as_fields.GainSel = 0;
        tTxSettings.as_fields.TxMode = g_tDrv7580.nTxModulation;
        tTxSettings.as_fields.IsZC = g_tDrv7580.nTxZeroCrossingSynchronous;
    }
    else
    {
        tTxSettings.as_fields.FreqChl = control->ChannelSelector;
        tTxSettings.as_fields.GainSel = 1;   //will use control->TransmitterGain
        tTxSettings.as_fields.TxMode = control->Modulation;
        tTxSettings.as_fields.IsZC = control->ZeroCrossingSynchronous;
        
        wait_ret = Drv_MibReadRequest(MIB_OBJ_ID_PHY_CFG, (TU8 *)&tPhyCfg);
        if(wait_ret == ERR_SUCCESS)
        {
            tPhyCfg.HighTxZCDelay = (TU8)(((control->TransmitDelay)>>8)&0xff);
	    tPhyCfg.LowTxZCDelay = (TU8)((control->TransmitDelay)&0xff); 
            wait_ret = Drv_MibWriteRequest(MIB_OBJ_ID_PHY_CFG, (TU8 *)&tPhyCfg);                              
        }
        if(wait_ret != ERR_SUCCESS) return ERR_BUSY;
    }
        
    g_tDrv7580.szTxPara[0] = tTxSettings.as_byte;
    if(tTxSettings.as_fields.GainSel)
    {
        g_tDrv7580.szTxPara[1] = control->TransmitterGain;  
        g_tDrv7580.nLenTxPara = 2;  
    }
    else
    {        
        g_tDrv7580.nLenTxPara = 1; 
    }
    g_tDrv7580.nCurTxPara = 0;
        
    g_tDrv7580.pTxCb = confirm;
    Increase_DrvDbgCntr(DATAREQ_CNT);
    
    tSendFrame.nCmd = DL_DataRequest_CODE;//PHY_DataRequest_CODE;
    tSendFrame.pBuf = pBuf;
    tSendFrame.nLen = nLen;
    tSendFrame.nWaitCode = DL_DataConfirm_CODE;//PHY_DataConfirm_CODE;
    tSendFrame.nErrCode = DL_DataError_CODE;//PHY_DataError_CODE;
    tSendFrame.nTmout = 0;      

    return Send_PlcFrame(&tSendFrame);             
}

PLC_State_t Drv_GetPlcState(void)
{
   TU8  i;
   
   if(g_tDrv7580.nChipStatus == PLC_NOT_CONFIGURED) return PLC_NOT_CONFIGURED;
   i = PLC_TxRx_StatusPoll();
   if(i == 1) return PLC_IDLE;
   else if(i == 2) return PLC_TX;
   return PLC_RX;
   // return (PLC_State_t)g_tDrv7580.nChipStatus;
}

PLC_Data_t* ST7580_GetInformations(void)
{
    // TBD: intelligent update to call it only if TX or RX happened.
    if (!Update_PlcInfo())
    {
        memset(&g_tDrv7580.tInfo, 0, sizeof(PLC_Data_t));	
    }

    return &g_tDrv7580.tInfo;
}

#include "fsl_uart_freertos.h"
extern uart_rtos_handle_t console_handle;
extern uart_rtos_handle_t plc_handle;
#include "queue.h"
extern QueueHandle_t plc_pkgs_queue;
#include "uartplc_k24.h"
extern uint32_t UpdatePlcStatus(void);

void DrvPlcFsm_task(void *pvParameters)
{
    TU8         nChar;
    uint8_t     plc_pkgs_tx[300];
    PLC_RQ_MSG  rq_msg;
    static  TickType_t  timeout = 1000;
    
    //vTaskDelay(3000);
    while(1){
        if (g_tDrv7580.nStat != PD_TX)
        {
            if (xQueueReceive( plc_pkgs_queue, &rq_msg, timeout ))
            {
                switch (rq_msg.cmd)
                {
                case MQ_TX:
                    g_tDrv7580.nStat = PD_TX;
                    break;
                case MQ_RX:
                    g_tDrv7580.nStat = PD_RX;
                    break;
                case MQ_TX_W_ACK:
                    g_tDrv7580.nFrmAck = ACK;
                    g_tDrv7580.nStat = PD_TX_W;
                    break;
                case MQ_TX_W_NAK:
                    g_tDrv7580.nFrmAck = NAK;
                    g_tDrv7580.nStat = PD_TX_W;           
                    break;
                case MQ_PD_STATE:
                    //UpdatePlcStatus();
                    break;
                }
            }
        }else{
            //超时判断处于哪个状态，是否重发
        }
        
        switch (g_tDrv7580.nStat)
        {
        case PD_IDLE:
            // check TX wait code//仅仅为了等待ConfirmCode
            
            if ( (g_tDrv7580.nFrmWaitCode == PHY_DataConfirm_CODE) || (g_tDrv7580.nFrmWaitCode == DL_DataConfirm_CODE) \
               ||(g_tDrv7580.nFrmWaitCode == SS_DataConfirm_CODE) )                             
            {
                // notify up-layer
                if (g_tDrv7580.pTxCb != NULL)
                {
                    g_tDrv7580.pTxCb(ERR_ERROR); //wait confirm timeout
                    Increase_DrvDbgCntr(TXERR_CNT3);
                    g_tDrv7580.pTxCb = NULL; 
                }
            }
            else
            {
                Increase_DrvDbgCntr(STX_CNT1);
                g_tDrv7580.nStat = PD_IDLE;
                g_tDrv7580.nFrmCur = 0;
                g_tDrv7580.nFrmCS16 = 0;
                g_tDrv7580.nErrCS = 0;
                timeout = portMAX_DELAY;
            }
            break;

        case PD_TX:
            // { STX | LEN | CMD | PAYLOAD | CRC16[2] }
            plc_pkgs_tx[0]  = STX;
            plc_pkgs_tx[1]  = g_tDrv7580.nFrmLen+g_tDrv7580.nLenTxPara;
            plc_pkgs_tx[2]  = g_tDrv7580.nFrmCmd;
            
            for (uint32_t j=0; j<g_tDrv7580.nLenTxPara; j++)
            {
                plc_pkgs_tx[3+j] = g_tDrv7580.szTxPara[j];
            }
            for (uint32_t j=0; j<g_tDrv7580.nFrmLen; j++)
            {
                plc_pkgs_tx[3+g_tDrv7580.nLenTxPara+j] = g_tDrv7580.pFrmPtr[j];
            }
    //        local_frame_pkgs.data       = g_tDrv7580.pFrmPtr;
    //        local_frame_pkgs.crc16      = g_tDrv7580.nFrmCS16;
            for (uint32_t i=1; i<3+g_tDrv7580.nFrmLen+g_tDrv7580.nLenTxPara; i++)
            {
                g_tDrv7580.nFrmCS16 += plc_pkgs_tx[i];
            }
            plc_pkgs_tx[plc_pkgs_tx[1]+3]   = (TU8)(g_tDrv7580.nFrmCS16 & 0x00ff);
            plc_pkgs_tx[plc_pkgs_tx[1]+4]   = (TU8)((g_tDrv7580.nFrmCS16>> 8) & 0x00ff);
            
            PRINTF("\r\n");
            for (uint32_t i=0; i<plc_pkgs_tx[1]+5; i++)
            {
                printf("%02x", plc_pkgs_tx[i]);
            }
            UART_RTOS_Send(&plc_handle, plc_pkgs_tx, g_tDrv7580.nFrmLen+g_tDrv7580.nLenTxPara+5);
            
            g_tDrv7580.nStat = PD_TX_W;
            timeout = TACK;
            
            break;

        case PD_TX_W:

            PRINTF("FsmRX:");
            //g_tDrv7580.nFrmAck = plc_pkgs_rx[0];
            if (g_tDrv7580.nFrmAck != ACK) 
            {
                PRINTF("NAK\r\n");
                 g_tDrv7580.nFrmAck = NAK;                                     
            }
            else
            {
                PRINTF("ACK\r\n");
            }
            g_tDrv7580.nStat = PD_IDLE;
            timeout = ST7580_WAIT_CFM;
           
            break;

        case PD_RX:
            // { STX | LEN | CMD | PAYLOAD | CRC16[2] }
            if((plc_pkgs_rx[0] == STX_RE)||(plc_pkgs_rx[0] == STX))
            {
                Increase_DrvDbgCntr(NONSTX_CNT1);
                g_tDrv7580.nStat = PD_IDLE;
                g_tDrv7580.nFrmCur = 1;
                g_tDrv7580.nFrmCS16 = 0;
                g_tDrv7580.nErrCS = 0;
            }
            else{
                g_tDrv7580.nStat = PD_IDLE;
                timeout = portMAX_DELAY;
                break;
            }
            g_tDrv7580.nFrmLen = plc_pkgs_rx[1];
            g_tDrv7580.nFrmCur++;
            g_tDrv7580.nFrmCS16 += plc_pkgs_rx[1]; 
            g_tDrv7580.nFrmCmd = plc_pkgs_rx[2];
            g_tDrv7580.nFrmCur++;
            g_tDrv7580.nFrmCS16 += plc_pkgs_rx[2]; 
            if ( ( (g_tDrv7580.nFrmCmd == PHY_DataIndication_CODE) || \
                   (g_tDrv7580.nFrmCmd == DL_DataIndication_CODE) || \
                   (g_tDrv7580.nFrmCmd == SS_DataIndication_CODE) || \
                   (g_tDrv7580.nFrmCmd == DL_SnifferIndication_CODE) || (g_tDrv7580.nFrmCmd == SS_SnifferIndication_CODE))
              || (g_tDrv7580.nFrmLen > ST7580_MIBBUF_LEN) ) 
            {
                    g_tDrv7580.pFrmPtr = ST7580_BufferRequest(g_tDrv7580.nFrmLen);
                    Increase_DrvDbgCntr(BUFFERREQ_CNT);                        
            }
            else
            {
                // use internal temporary buffer
                g_tDrv7580.pFrmPtr = g_tDrv7580.tFrmBuf;
            }
            for (uint32_t i=0; i<g_tDrv7580.nFrmLen; i++)
            {
                g_tDrv7580.pFrmPtr[i] = plc_pkgs_rx[3+i];
                g_tDrv7580.nFrmCur++;
                g_tDrv7580.nFrmCS16 += plc_pkgs_rx[3+i]; 
            }
            if ((plc_pkgs_rx[g_tDrv7580.nFrmLen+3] != (TU8)(g_tDrv7580.nFrmCS16 & 0x00FF))||\
                (plc_pkgs_rx[g_tDrv7580.nFrmLen+4] != (TU8)(g_tDrv7580.nFrmCS16>>8 & 0x00FF))) 
            {
                g_tDrv7580.nErrCS = 1;          
            }
    //            PRINTF("\r\n");
    //            for (uint32_t i=0; i<g_tDrv7580.nFrmLen+5; i++)
    //            {
    //                printf("%02x", plc_pkgs_rx[i]);
    //            }
            PRINTF("FsmTX:");
            if (g_tDrv7580.nErrCS && g_tDrv7580.nReTry == 0)  
            {
                nChar = NAK;
                PRINTF("NAK\r\n");
                UART_RTOS_Send(&plc_handle, &nChar, 1);
                Increase_DrvDbgCntr(CSERR_CNT);
                g_tDrv7580.nReTry = 1;  
                
            }
            else
            {
                nChar =ACK;
                PRINTF("ACK\r\n");
                UART_RTOS_Send(&plc_handle, &nChar, 1);
                g_tDrv7580.nReTry = 0;

                // send to message handler
                Handle_PlcMsg();     
                
                // send msg queue to plc app task or call api
                
            }
            g_tDrv7580.nStat = PD_IDLE;      
            timeout = portMAX_DELAY;
                
            break;
        }
    }
}

//add by ywt
#include "queue.h"
extern QueueHandle_t plc_status_queue;
uint8_t  pd_status_buf[2];
uint32_t UpdatePlcStatus(void)
{
    TU8     nVal;
    PLC_RQ_MSG  status_rq_msg;
    switch (g_tDrv7580.nStat)
    {
    case PD_IDLE:               // restart T_REQ procedure       

        //printf("QueueSpaces=%d\r\n", uxQueueSpacesAvailable(plc_status_queue));
        
        if (uxQueueSpacesAvailable(plc_status_queue)==0)
        {
            xQueueReceive( plc_status_queue, &status_rq_msg, ( TickType_t ) 0 );
        }
        
        PLC_Set_T_REQ(0);
        g_tDrv7580.nStat = PD_TREQ_W;

        break;
    
    case PD_TREQ_W:             // T_REQ procedure started
        
         //at least 2 bytes received
        if (xQueueReceive( plc_status_queue, &status_rq_msg, ( TickType_t ) (ST7580_TREQ_INTER+TSR) ))
        {
            PRINTF("PD msg:0x%x\r\n", status_rq_msg.cmd);
            nVal = pd_status_buf[0];
            if (nVal == '?')   // "0x3F"
            {
                Increase_DrvDbgCntr(STATUSMSG_CNT);                

                nVal = pd_status_buf[1]; 
                
                g_tDrv7580.nChipStatus = PLC_IDLE;
                if((nVal&0x18) == 0x18)
                {
                    g_tDrv7580.nChipStatus = PLC_NOT_CONFIGURED;  
                }
                else if (nVal & 0x02)
                {
                    g_tDrv7580.nChipStatus = PLC_TX;
                }
                else if (nVal & 0x04)
                {
                    g_tDrv7580.nChipStatus = PLC_RX;
                }
                
                g_tDrv7580.nChipTempr = (nVal >> 6);               
                g_tDrv7580.bChipInAL = ((nVal>>3)&0x03);
                g_tDrv7580.bChipOVC = (nVal>>5)&0x01;                                                          
                
                PLC_Set_T_REQ(1);
                
                g_tDrv7580.nStat = PD_IDLE;                                

                return PD_RET_SUCC;
            }
            else
            {
                PLC_Set_T_REQ(1);        
                if(nVal == STX || nVal == STX_RE)
                {
                    // turn to PD_RX state
                    g_tDrv7580.nStat = PD_IDLE;
                    Increase_DrvDbgCntr(STX_CNT2); 
//                    g_tDrv7580.nFrmCur = 1;
//                    g_tDrv7580.nFrmCS16 = 0;
//                    g_tDrv7580.nErrCS = 0;
//                    TIMER_SetDelay_ms(&g_tDrv7580.tTimer, TTIC);
//                    TIMER_Start(&g_tDrv7580.tTimer);
                    //发送信号给接收处理
//                    status_rq_msg.cmd = MQ_RX;
//                    xQueueSend(plc_pkgs_queue, ( void * )&status_rq_msg, ( TickType_t ) 10U );
                }
                else  
                {
                    g_tDrv7580.nStat = PD_IDLE;  
                    Increase_DrvDbgCntr(NONSTX_CNT2);  
                }

                return PD_RET_RX;
            }
        }
        else
        {
            // T_REQ -> HIGH
            PLC_Set_T_REQ(1);

            // turn to PD_IDLE state
            g_tDrv7580.nStat = PD_IDLE;
            
            return PD_RET_TMOUT;
        }
        break;

    default:
        return PD_RET_ERROR;
    }

    return PD_RET_ONGOING;
}

uint32_t CheckPlcStatus(void)
{
    TU8     nRet, nRetry = 3;

    while (nRetry-- > 0)
    {
        do 
        {
            nRet = UpdatePlcStatus();
        } while (nRet == PD_RET_ONGOING);
       
        if (nRet == PD_RET_SUCC || nRet == PD_RET_RX) break;       
    }
    
    if (nRet != PD_RET_SUCC)
    {
        PRINTF("nRet=%d", nRet);
    }
    
    return (TBool)(nRet == ((TU8)PD_RET_SUCC));
}

TU8 DrvPlcInit(void)
{
    memset(&g_tDrv7580, 0, sizeof(TDrvST7580));
#ifdef START_ST7580_DEBUG    
    memset((TU8 *)&g_tDrvDebug, 0x00, sizeof(g_tDrvDebug));
#endif    
       
    // reset the HW
    //PLC_HwReset(); vTaskDelay(1000);
            
    // Ensure the T_REQ -> HIGH
    PLC_Set_T_REQ(1);
    
    g_tDrv7580.nStat = PD_IDLE; 
    g_tDrv7580.nReTry = 0; 

    // check ST7580 availability
    if (!CheckPlcStatus())
        return ERR_BUSY;

    g_tDrv7580.nChipStatus = PLC_NOT_CONFIGURED;
    g_tDrv7580.tRxSet.as_fields.PSKModulationPreambleLength = 2;//default value
   
    return ERR_SUCCESS;
}
