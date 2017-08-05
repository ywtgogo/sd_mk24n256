#ifndef __ST7580_DRV_FSM_INTERFACE_H_
#define __ST7580_DRV_FSM_INTERFACE_H_

#include "drv_st7580.h"
#include "plc_k24_timer.h"

#define START_ST7580_DEBUG

#ifndef WIN32
#define ST7580_TREQ_INTER   (30) 
#define TMIN                (10)    
#else
#define ST7580_TREQ_INTER   (60)//(30000)
#define TMIN                (40)    
#endif

#define TPOLL               (200 + TMIN)    // in ms
#define TACK                (40 + TMIN)    // in ms
#define TTIC                (10 + TMIN)    // in ms
#define TRST                (100 + TMIN)    // in ms
#define TSR                 (200)

#define	STX	0x02	//... Start of TeXt
#define	ACK	0x06	//... Acknowledgment
#define	NAK	0x15	//... Not Acknowledgment
#define STX_RE  0x03

#define ST7580_MIB_MAXLEN   (16)
#define ST7580_MIBBUF_LEN  (ST7580_MIB_MAXLEN+1)  //for MIB Write
#define ST7580_WAIT_CFM     (1000)      // waiting confirm code timeout
extern  uint8_t  pd_status_buf[2];
enum{
    PD_IDLE = 0,
    PD_TREQ_W,
    PD_TX,
    PD_TX_W,        // wait ACK or NAK
    PD_RX,
    PD_RX_PARSE,
    PD_MAX_STAT
};

enum{
    PD_RET_SUCC = 0,
    PD_RET_RX, 
    PD_RET_ONGOING,
    PD_RET_TMOUT,
    PD_RET_ERROR
};
enum{		                       
    BUFFERREQ_CNT = 0,  //驱动接收数据帧开始次数
    DATAINDICATION_CNT, //驱动接收数据帧结束次数
    CSERR_CNT,          //驱动接收命令帧发现校验出错次数
    TXERR_CNT1, //驱动发送错误次数1
    TXERR_CNT2, //驱动发送错误次数2
    TXERR_CNT3, //驱动发送错误次数3
    TXSUCC_CNT, //驱动发送成功次数
    DATAREQ_CNT,   //call Data_Request() times
    STATUSMSG_CNT, //接收plc状态字节开始次数
    TICTMOUT_CNT,  //接收字节间超时次数
    NONSTX_CNT1,
    NONSTX_CNT2,
    STX_CNT1,
    STX_CNT2,
    
    DEBUG_MAXCNT
};

/** phy frames statistical informations type */
typedef struct {
    TU8     HighRxUWPhyCounter;     
    TU8     LowRxUWPhyCounter;  
    TU8     HighRxGoodPhyFrame;   
    TU8     LowRxGoodPhyFrame; 
    TU8     HighTxPhyCounter;
    TU8     LowTxPhyCounter;
    TU8     HighTxPhyRefused; 
    TU8     LowTxPhyRefused; 
    TU8     HighMainsPeriod;  
    TU8     LowMainsPeriod; 
}ST7580_PhyDataTypeDef; //MIB 06 structure

typedef union {
    struct {
        TU8     AcesMode:2;
        TU8     SniffMode:1;
        TU8     CrcLen:2;
        TU8     Pad1:1;
        TU8     CrcCon:1;
        TU8     Pad2:1;  
	} as_fields;
	TU8 as_byte;
}ST7580_ModemCfgTypeDef;

typedef struct {
    TU8     HighHF;
    TU8     MidHF;
    TU8     LowHF;
    TU8     HighLF;
    TU8     MidLF;
    TU8     LowLF;
        
    TU8     RxChl:1;
    TU8     RxModuHigh:1;
    TU8     RxModuLow:1;
    TU8     IsCurrentCtrl:1;
    TU8     Pad1:4;
    
    TU8     TxGain:5;
    TU8     Pad2:3;
    
    TU8     HighTxZCDelay;
    TU8     LowTxZCDelay;
    
    TU8     PskPreambleLen:2;
    TU8     Pad3:6;
    
    TU8     FskBaud:2;
    TU8     FskDeviation:1;
    TU8     FskPreambleLen:2;
    TU8     FskUWLen:1;
    TU8     Pad4:2;
        
    TU8     FskUwMsb; 
    TU8     FskUwLsb;               
}ST7580_PhyCfgTypeDef;
typedef struct  
{
    TU8      nStat;
    
    TU8      nChipStatus:2;
    TU8      nChipTempr:2; 
    TU8      bChipInAL:2;
    TU8      bChipOVC:1;
    TU8      nPad:1;
     
    PLC_RX_Settings_t  tRxSet;
      
    TU8      nFrmLen;
    TU16     nFrmCur;
    TU16     nFrmCS16;
    TU8      nFrmCmd;
    TU8      nFrmAck;        // ACK or NAK
    TU8      nFrmWaitCode;
    TU8      nFrmErrCode;
    TU8 *    pFrmPtr;
    TU8      tFrmBuf[ST7580_MIBBUF_LEN];   // used by TX & RX
    TU8      nResetCode:6;
    TU8      nErrCS:1;
    TU8      nReTry:1;

    Timer_t tTimer;
    Timer_t tTimerTREQ;
    Timer_t tTimerWaitCode;

//    ST7580_PhyCfgTypeDef tCfg;
    TU8      nTxModulation : 3;
    TU8      nTxChannelSelector : 1;
    TU8      nTxZeroCrossingSynchronous : 1;
    TU8      nLenTxPara:3; 
    TU8      nCurTxPara;
    TU8      szTxPara[5]; 
    
    PLC_Data_t   tInfo;

    void (*pTxCb)(TU8);

}TDrvST7580;
typedef struct {  
    TU8     nCmd;
    TU8     *pBuf;
    TU8     nLen;
    TU8     nWaitCode;
    TU8     nErrCode;
    TU16    nTmout;    
}ST7580_SendFrameParaTypedef;
typedef union {
	struct {
          TU8     FreqSel:1;
          TU8     IfWrtFreq:1;
          TU8     FreqChl:1;
          TU8     GainSel:1;
          TU8     TxMode:3;
          TU8     IsZC:1;
	} as_fields;
	TU8 as_byte;              
}ST7580_TxSettingTypeDef;

typedef struct {
    uint8_t     stx;
    uint8_t     length;
    uint8_t     cmd_code;
    uint8_t     *data;
    uint16_t    crc16;
}ST7580_LocalFrame_t;




#endif 

