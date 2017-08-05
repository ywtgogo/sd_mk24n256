#ifndef __ST_UARTPLC_ST7580_H_
#define __ST_UARTPLC_ST7580_H_



#ifdef __cplusplus
extern "C"{
#endif

/* Includes ------------------------------------------------------------------*/
#include "plc_k24_types.h"
#include "FreeRTOS.h"    
#include "event_groups.h"
    
#define PLC_PUT_Q 	(2 << 0)
extern EventGroupHandle_t PlcPutEventGroup;

#define HW_UART_START()     xEventGroupSetBits(PlcPutEventGroup,  PLC_PUT_Q)
#define HW_UART_STOP()      xEventGroupClearBits(PlcPutEventGroup, PLC_PUT_Q)
    
extern uint8_t              plc_pkgs_rx[300];

enum PLC_MQ_CMD
{
    MQ_TX = 0,    
    MQ_TX_W_ACK,
    MQ_TX_W_NAK,
    MQ_RX,    
    MQ_PD_STATE,
};

typedef struct
{
	enum        PLC_MQ_CMD cmd;

}PLC_RQ_MSG;


typedef enum {
	UART_WIFI_RECV_HEAD = 0,
	UART_WIFI_RECV_BODY,
	UART_WIFI_RECV_SUC,
	UART_WIFI_RECV_BUTT
}PLC_FRAME_CMD_CODE_t;

typedef enum {
	SUCCESS = 0,
	ERROR,
    START,
    GETLEN,
    PROGRESS,    
    LOST,
}PLC_PACK_STAE_t;

PLC_PACK_STAE_t plc_data_pack(uint8_t *plc_recv_buffer ,uint8_t *plc_frame_rx, uint32_t pack_len, uint32_t *pack_recved_len);

void    PLC_UartOpen(void);
void    PLC_UartFlushRxQ(void);
void    PLC_UartFlushTxQ(void);
TU16     PLC_UartGetRxLen(void);
TU16     PLC_UartGetTxEmpty(void);
TBool    PLC_UartTxFinished(void);
TU8      PLC_UartGetChar(void);
void    PLC_UartPutChar(TU8 nChar);
void    PLC_Set_T_REQ(TU8 nVal);
void    PLC_HwReset(void);
TU32 PLC_UartBufWrite(TU8* pBuf, TU32 nLen);
TS32 PLC_UartRead(TU8* pBuf, TU32 nBufLen);

TU8 PLC_TxRx_StatusPoll(void);

#ifdef __cplusplus
}
#endif

#endif /* __ST_UARTPLC_ST7580_H_ */
/******************* (C) COPYRIGHT 2010 STMicroelectronics *****END OF FILE****/

