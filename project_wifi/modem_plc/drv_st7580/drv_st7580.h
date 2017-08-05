/******************** (C) COPYRIGHT 2014 STMicroelectronics ********************/
/**
 * @file    : drv_st7580.h
 * @brief   : .
 * @author  : 
 * @version : V1.0
 * @date    : 02/18/2010
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
/* Define to prevent recursive inclusion ----------------------*/
#ifndef __ST7580_DRV_INTERFACE_H_
#define __ST7580_DRV_INTERFACE_H_

/* Includes ---------------------------------------------------*/
#include "plc_k24_types.h"
/* Exported types ---------------------------------------------*/
/* Exported constants -----------------------------------------*/
#define ST7580_TRANSMISSION_MAXLEN  (254)

//request
#define BIO_ResetRequest_CODE  0x3C
#define MIB_WriteRequest_CODE  0x08
#define MIB_ReadRequest_CODE   0x0C
#define MIB_EraseRequest_CODE  0x10
#define PingRequest_CODE       0x2C
#define PHY_DataRequest_CODE   0x24
#define DL_DataRequest_CODE    0x50
#define SS_DataRequest_CODE    0x54

//confirm
#define BIO_ResetConfirm_CODE  0x3D
#define MIB_WriteConfirm_CODE  0x09
#define MIB_ReadConfirm_CODE   0x0D
#define MIB_EraseConfirm_CODE  0x11
#define PingConfirm_CODE       0x2D
#define PHY_DataConfirm_CODE   0x25
#define DL_DataConfirm_CODE    0x51
#define SS_DataConfirm_CODE    0x55

//error response to request
#define BIO_ResetError_CODE    0x3F
#define MIB_WriteError_CODE    0x0B
#define MIB_ReadError_CODE     0x0F
#define MIB_EraseError_CODE    0x13
#define PHY_DataError_CODE     0x27
#define DL_DataError_CODE      0x53
#define SS_DataError_CODE      0x57
#define CMD_SyntaxError_CODE   0x36

//error cause
#define ErrorCause_WPL         0x02
#define ErrorCause_WPV         0x03
#define ErrorCause_BUSY        0x04
#define ErrorCause_HEAT        0x0B
#define ErrorCause_GEN         0xFF

//indication
#define BIO_ResetIndication_CODE    0x3E
#define PHY_DataIndication_CODE     0x26
#define DL_DataIndication_CODE      0x52
#define DL_SnifferIndication_CODE   0x5A
#define SS_DataIndication_CODE      0x56
#define SS_SnifferIndication_CODE   0x5E

enum {
    MIB_OBJ_ID_MDM_CFG = 0,
    MIB_OBJ_ID_PHY_CFG,
    MIB_OBJ_ID_SS_KEY,
    MIB_OBJ_ID_RES_DATA,
    MIB_OBJ_ID_LST_DATA,
    MIB_OBJ_ID_LST_TX,
    MIB_OBJ_ID_PHY_DATA,
    MIB_OBJ_ID_DL_DATA,
    MIB_OBJ_ID_SS_DATA,
    MIB_OBJ_ID_HI_TMOUT,
    MIB_OBJ_ID_FW_VERSION,

    MIB_OBJ_NUM
};

typedef struct {
    TU8     RxMode:3;         /** Last reception mode */
    TU8     RxChannel:1;      /** Last reception channel */
    TU8     RxPga:4;         /**  PGA value */
    TS8     RxSNR;          /** estimated SNR */
    TU8     HighRxZCDelay;      /** Zero crossing delay in 13us steps */        
    TU8     LowRxZCDelay;
}ST7580_LastRxTypeDef; //MIB 04 structure
typedef struct {
    TU8     TxMaxTemp:2;      /** Max temperature */
    TU8     TxMaxGain:5;      /** Max gain applied */
    TU8     Pad1:1;
    TU8     TxMinTemp:2;      /** Min temperature */
    TU8     TxMinGain:5;     /** Min gain applied */
    TU8     Pad2:1;
    TU8     TxOverCurEvts:7;  /** Over current event counter */
    TU8     TxOverCurFlag:1;          
    TU8     HighTxZCDelay;      /** Zero crossing delay in 13us steps */
    TU8     LowTxZCDelay;
}ST7580_LastTxTypeDef;//MIB 05 structure
/** mac frames statistical informations type */
typedef struct {
    TU8     HighRxGoodMacCounter;  
    TU8     LowRxGoodMacCounter;  
    TU8     HighRxBadMacCounter; 
    TU8     LowRxBadMacCounter; 
    TU8     HighTxMacCounter; 
    TU8     LowTxMacCounter; 
    TU8     HighTxMacRefused; 
    TU8     LowTxMacRefused;    
}ST7580_MacDataTypeDef;//MIB 07 structure

/** ss frames statistical informations type */
typedef struct {
    TU8     HighRxGoodSSCounter;     
    TU8     LowRxGoodSSCounter; 
    TU8     HighRxBadSSCounter1;   
    TU8     LowRxBadSSCounter1; 
    TU8     HighRxBadSSCounter2;
    TU8     LowRxBadSSCounter2;
    TU8     HighTxSSCounter; 
    TU8     LowTxSSCounter;
    TU8     HighTxSSRefused; 
    TU8     LowTxSSRefused;    
}ST7580_SSDataTypeDef;//MIB 08 structure

typedef struct {
    TU8     Tsr;
    TU8     Tack;
    TU8     Tic;  
}ST7580_TmoutTypeDef;//MIB 09 structure
//! Available modulations
typedef enum {
	//! BPSK modulation
	MODE_BPSK = 0x00,
	//! QPSK modulation
	MODE_QPSK = 0x01,
	//! 8PSK modulation
	MODE_8PSK = 0x02,
	//! FSK modulation
	MODE_FSK = 0x03,
	//! BPSK CODED modulation
	MODE_BPSK_COD = 0x04,
	//! QPSK modulation
	MODE_QPSK_COD = 0x05,
	//! Reserved value, do not use
	MODE_RESERVED = 0x06,
	//! BPSK CODED PNA modulation
	MODE_BPSK_COD_PNA = 0x07,
	//! Limit value for this enumerator
	MODE_LIMIT,
	//! Invalid modulation
	MODE_INVALID = 0xFF
} PLC_Modulation_t;

//! Preamble length type
typedef enum {
	//! Preamble is 16 bits long
	BIT_16 = 0,
	//! Preamble is 24 bits long
	BIT_24 = 1,
	//! Preamble is 32 bits long
	BIT_32 = 2,
	//! Preamble is 40 bits long
	BIT_40 = 3
} PLC_PreambleLength_t;

//! Available channels
typedef enum {
	//! High channel
	LOW_CHANNEL = 0,
	//! Low channel
	HIGH_CHANNEL = 1
} PLC_Channel_t;
//! Modulation received on a channel
typedef enum {
	//! Channel receive any PSK modulation
	ANY_PSK_MODULATION = 1,
	//! Channel receive FSK modulation
	FSK_MODULATION = 0
} PLC_ChannelReceivedModulation_t;

//! Receiver mode
typedef enum {
	//! Receiver on high channel only
	SINGLE_CHANNEL = 0,
	//! Receiver on both channels
	DUAL_CHANNEL = 1
} PLC_ReceiverMode_t;
/*!
Part of the overall PLC driver configuration.
\brief Receiver settings type
*/
typedef union {
	struct {
		//! Enable/disable sniffer functionality
		TU8 SnifferMode : 1;
		TU8 unused : 2;
		//! Select PSK modulation preable length, according to \ref PLC_PreambleLength_t
		TU8 PSKModulationPreambleLength : 2;
		//! Low channel modulation, according to \ref PLC_ChannelReceivedModulation_t
		TU8 RxLowChannelModulation : 1;
		//! High channel modulation, according to \ref PLC_ChannelReceivedModulation_t
		TU8 RxHighChannelModulation : 1;
		//! Select single/dual channel receiver, according to \ref PLC_ReceiverMode_t
		TU8 RxChannelSelector : 1;
	} as_fields;
	TU8 as_byte;
} PLC_RX_Settings_t;

/*!
Part of the overall PLC driver configuration.
\brief Transmitter settings type
*/
typedef struct {
	//! Select TX modulation, according to \ref PLC_Modulation_t
	TU8 Modulation : 3;
	//! Transmitter gain
	TU8 TransmitterGain : 5;
	//! Select which default frequency use between high or low channel, according to \ref PLC_Channel_t
	TU16 ChannelSelector : 1;
	//! Enable/Disable zero crossing synchonous transmission
	TU16 ZeroCrossingSynchronous : 1;
	//! Zero crossing delay setup for zero crossing synchronous transmission
	TU16 TransmitDelay : 11;
} PLC_TX_Settings_t;

/*!
Part of the overall PLC driver configuration.
\brief FSK modulation settings type
*/
typedef union {
	struct {
		//! Baud rate selector 0: 1200, 1: 2400, 2: 4800, 3: 9600
		TU8 BaudRate : 2;
		//! Deviation selector 0: 0.5, 1: 0
		TU8 Deviation : 1;
		//! Preamble length selector, according to \ref PLC_PreambleLength_t
		TU8 PreambleLength : 2;
		//! Header length selector: 0: 8bit, 1: 16bit
		TU8 HeaderLength : 1;
	} as_fields;
	TU8 as_byte;
} PLC_FSK_Settings_t;

/*!
This compound collects all PLC driver configurations.
\brief PLC driver configuration type
*/
typedef struct {
	//! High frequency, expressed in steps of 32 Hz
	TU16 fHigh;
	//! Low frequency, expressed in steps of 32 Hz
	TU16 fLow;
	//! Receiver settings
	PLC_RX_Settings_t RX_Settings;
	//! Default transmitter settings
	PLC_TX_Settings_t TX_Settings;
	//! FSK settings
	PLC_FSK_Settings_t	FSK_Settings;
	//! FSK modulation UW value
	TU16 FskUW;
} PLC_Config_t;
typedef enum {
	//! PLC driver is not configured, cannot transmit nor receive.
	PLC_NOT_CONFIGURED = 0,
	//! PLC driver is configured and currently idle. PLC driver layer is ready to transmit or to receive.
	PLC_IDLE,
	//! PLC driver is trasmitting or preparing to transmit data. Transmission starts as soon as DSP synchronize, meanwhile UW event are discarded.
	PLC_TX,
	//! PLC driver is currently receiving data.
	PLC_RX
} PLC_State_t;
/*!
Collects all the information about the last received frame
\brief Receiver information type
*/
typedef struct {
	//! Modulation, channel and PGA value compound
	union {
		struct {
			//! Detected modulation, according to \ref PLC_Modulation_t
			TU8 Modulation : 3;
			//! Detected channel, according to \ref PLC_Channel_t
			TU8 Channel : 1;
			//! PGA value
			TU8 PGA : 4;
		} as_fields;
		TU8 as_byte;
	} Info;
	//! Estimated SNR in dB
	TS8 SNR;
	//! Zero crossing delay in 13us steps
	TS16 ZCDelay;//Delay between the received UW last bit and the mains zerocrossing
} PLC_RX_Information_t;

/*!
Collects all the information about the last transmitted frame
\brief Transmitter information type
*/
typedef struct {
	//! Max temperature, according to \ref PLC_temperature_t
	TU8 MaxTemp : 2;
	//! Max gain applied
	TU8 MaxGain : 5;
	TU8 Unused_TX_1 : 1;		
	//---
	//! Min temperature, according to \ref PLC_temperature_t
	TU8 MinTemp : 2;
	//! Last gain applied
	TU8 LastGain : 5;
	TU8 Unused_TX_2 : 1;	
	//---
	//! Over current event counter
	TU8 OverCurrentEventsNumber : 7;
	//! Over current flag, true if any over current event has happened
	TU8 OverCurrent : 1;
	//---
	//! Zero crossing delay in 13us steps
	TS16 ZCDelay; //Delay between the last transmitted UW last bit and the mains zerocrossing
} PLC_TX_Information_t;

/*!
Collects statistics about the PLC driver
\brief PLC driver statistics type
*/
typedef struct {
	//! UW detected event counter
	TU16 UWEvents;
	//! Received frames: kept frames counter
	TU16 Received;
	//! Transmitted frames counter
	TU16 Transmitted;
	//! Not transmitted frames counter
	TU16 Refused;
	//! Network period as number of 13us long steps
	TS16 ZCPeriod;
} PLC_Statistics_t;

/*!
This compound collects all PLC driver data (receiver, transmitter, statistics).
\brief PLC driver data type
*/
typedef struct {
	//! PHY layer info on the last received frame
	PLC_RX_Information_t RX_Information;
	//! PHY layer info on the last transmitted frame
	PLC_TX_Information_t TX_Information;
	//! PHY layer statistics
	PLC_Statistics_t Statistics;
} PLC_Data_t;

TU8 DrvPlcInit(void);

TU8 Drv_MibReadRequest(TU8 nObjId, TU8 * pBuf);
TU8 Drv_MibEraseRequest(TU8 nObjId);
TU8 Drv_MibWriteRequest(TU8 nObjId, TU8 * pBuf);
PLC_State_t Drv_GetPlcState(void);
TU8 Drv_DataRequest(PLC_TX_Settings_t *control, TU8 *pBuf, TU8 nLen, void (*confirm)(TU8));//note: pBuf must be global buffer, which is allowed to be released till callback (*confirm)(u8) received 

TU8 Drv_PlcInit(void);
TU8 Drv_Configure(PLC_Config_t *pConfig);
void Drv_PlcFsm(void);

TU8 ST7580_PingRequest(TU8 nLen, TU8 * pBuf); 
TU8 ST7580_BioResetRequest(void);
TVoid ST7580_DataIndication(TU8 * pBuf, TU8 nLen, TU16 nDelayInMs);
TU8 *ST7580_BufferRequest(TU8 nLen);
PLC_Data_t* ST7580_GetInformations(void);
TU16 ST7580_SamePhRcvZcDelay(void);
TU8 ST7580_ChkConfigure(PLC_Config_t *pConfig);

#endif /* __ST7580_DRV_INTERFACE_H_ */

/******************* (C) COPYRIGHT 2010 STMicroelectronics *****END OF FILE****/
