/******************** (C) COPYRIGHT 2009 STMicroelectronics ********************/
/**
 * @file    : drv_st7580_win32.c
 * @brief   : low level function implementation of ST7580 driver on STM32.
 * @author  : Alex Li (alex.li@st.com)
 * @version : V1.0
 * @date    : 06/03/2010
*
**********************************************************************************
*              (C) COPYRIGHT 2009 STMicroelectronics                        <br>
* THE PRESENT SOFTWARE WHICH IS FOR GUIDANCE ONLY AIMS AT PROVIDING CUSTOMERS  
* WITH CODING INFORMATION REGARDING THEIR PRODUCTS IN ORDER FOR THEM TO SAVE TIME.
* AS A RESULT, STMICROELECTRONICS SHALL NOT BE HELD LIABLE FOR ANY DIRECT, 
* INDIRECT OR CONSEQUENTIAL DAMAGES WITH RESPECT TO ANY CLAIMS ARISING FROM THE 
* CONTENT OF SUCH SOFTWARE AND/OR THE USE MADE BY CUSTOMERS OF THE CODING?
* INFORMATION CONTAINED HEREIN IN CONNECTION WITH THEIR PRODUCTS.
**********************************************************************************
*/

/* Includes ------------------------------------------------------------------*/
/* FreeRTOS kernel includes. */
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "timers.h"
#include "event_groups.h"

//#include "plc_k24_plat.h"
//#include "st_app_hw_def_stm32.h"
#include "board.h"
#include "util_queue.h"
#include "uartplc_k24.h"


static TPlcQueue   g_tTxQ, g_tRxQ;

//extern void TIMER_Delay(Counter_t nMs);

EventGroupHandle_t PlcPutEventGroup = NULL;
uint8_t             plc_pkgs_rx[300];

//////////////////////////////////////////////////////////////////////////
void PLC_Set_T_REQ(TU8 nVal)
{
    //GPIO_WriteBit(PLC_TREQ_GPIO, PLC_TREQ_PIN, (nVal)?(Bit_SET):(Bit_RESET));
    (nVal)?PLC_TREQ_ON():PLC_TREQ_OFF();
    //(nVal)?PLC_TREQ_OFF():PLC_TREQ_ON();
}
//TU8 PLC_TxRx_StatusPoll(void)
//{
//    if(GPIO_ReadInputDataBit(PLC_SENSE_GPIO, PLC_SENSE_RXPIN))
//        return 3;  
//    if(GPIO_ReadInputDataBit(PLC_SENSE_GPIO, PLC_SENSE_TXPIN))
//        return 2;
//    return 1;
//}

TVoid PLC_HwReset()
{
    //GPIO_WriteBit(PLC_RST_GPIO, PLC_RST_PIN, Bit_RESET);
    TIMER_Delay(100);
    //GPIO_WriteBit(PLC_RST_GPIO, PLC_RST_PIN, Bit_SET);
}

#define HW_UART_START()     xEventGroupSetBits(PlcPutEventGroup,  PLC_PUT_Q)
#define HW_UART_STOP()      xEventGroupClearBits(PlcPutEventGroup, PLC_PUT_Q)
//////////////////////////////////////////////////////////////////////////

#include "board.h"
#include "fsl_uart_freertos.h"
#include "fsl_uart.h"
#include "fsl_debug_console.h"
uart_rtos_handle_t plc_handle;
struct _uart_handle plc_t_handle;
uint8_t plc_background_buffer[512];
uint8_t plc_recv_buffer[1];
TVoid PLC_UartOpen(void)
{
    struct rtos_uart_config uart_config = {
        .baudrate = 57600,
        .parity = kUART_ParityDisabled,
        .stopbits = kUART_OneStopBit,
        .buffer = plc_background_buffer,
        .buffer_size = sizeof(plc_background_buffer),
    };
   
    uart_config.srcclk = CLOCK_GetFreq(PLC_UART_CLKSRC);
    uart_config.base = PLC_UART;

    if (0 > UART_RTOS_Init(&plc_handle, &plc_t_handle, &uart_config))
    {
        PRINTF("Error during UART initialization.\r\n");
        vTaskSuspend(NULL);
    }
        
    PLC_TREQ_GPIO_INIT();
    
    Q_INIT(g_tTxQ);
    Q_INIT(g_tRxQ);
}

void plc_put_msg_task(void *pvParameters)
{
    TU8     nChar;
    
    PlcPutEventGroup = xEventGroupCreate();
    
    while(1)
    {
        /* 事件等待 */
        xEventGroupWaitBits(PlcPutEventGroup,    	/* The event group handle. */
                PLC_PUT_Q,			/* The bit pattern the event group is waiting for. */
                pdFALSE,         		/* BIT will be cleared automatically. */
                pdTRUE,         		/* Don't wait for both bits, either bit unblock task. */
                portMAX_DELAY); 		/* Block indefinitely to wait for the condition to be met. */     
        
        if (Q_GET_L(g_tTxQ) > 0)
        {
            Q_READHEAD(g_tTxQ, nChar);
            if (kStatus_Success != UART_RTOS_Send(&plc_handle, &nChar, 1))
            {
                PRINTF("PLC PUT ERROR\r\n");
                vTaskSuspend(NULL);
            }    
        }
        else
        {
            HW_UART_STOP();
            /* 清除事件 */
            //xEventGroupClearBits(PlcPutEventGroup, PLC_PUT_Q);
        }

    }
}

#include "drv_st7580_fsm.h"

QueueHandle_t plc_pkgs_queue;
QueueHandle_t plc_status_queue;

extern uint32_t UpdatePlcStatus(void);


PLC_PACK_STAE_t plc_data_pack(uint8_t *plc_recv_buffer ,uint8_t *plc_frame_rx, uint32_t pack_len, uint32_t *pack_recved_len)
{
    PLC_PACK_STAE_t     retval;
    static TickType_t   start_tick;    
    static uint32_t     plc_pack_len;    

    if (pack_len <= 2)
    {
        if ((((xTaskGetTickCount() - start_tick) > (TickType_t)10u)) || (plc_pack_len >= pack_len))
        {
            plc_pack_len = 0;
            start_tick = xTaskGetTickCount();
            retval = START;
        }
    }
    else if(pack_len >= 5)
    {
        if (((xTaskGetTickCount() - start_tick) > (TickType_t)1000u) || (plc_pack_len >= pack_len))
        {
            plc_pack_len = 0;
            start_tick = xTaskGetTickCount();
            retval = START;
        }
    }
    plc_frame_rx[plc_pack_len++] = plc_recv_buffer[0];
    *pack_recved_len = plc_pack_len;
    if (*pack_recved_len == 2)
        retval = GETLEN;
    else if(*pack_recved_len > 2)
        retval = PROGRESS;
    
    return retval;
}

void plc_get_msg_task(void *pvParameters)
{
    int                 error;
    PLC_PACK_STAE_t     pack_state;
    size_t              n;
    uint32_t            pack_len;
    uint32_t            crc_len = 2;
    uint32_t            pack_recved_len;
    PLC_RQ_MSG          plc_rq_msg;
    
    plc_status_queue    = xQueueCreate(1, sizeof(uint8_t *));
    plc_pkgs_queue      = xQueueCreate(3, sizeof(uint8_t *));
    PLC_UartOpen();
    
    while(1)
    {
        error = UART_RTOS_Receive(&plc_handle, plc_recv_buffer, sizeof(plc_recv_buffer), &n);
        if (error == kStatus_UART_RxHardwareOverrun)
        {
            PRINTF("RxHardwareOverrun\r\n");
        }
        if (error == kStatus_UART_RxRingBufferOverrun)
        {
            PRINTF("RxRingBufferOverrun\r\n");
        }
        if (n > 0)
        {
            //printf("%02x ", plc_recv_buffer[0]);
            pack_state = plc_data_pack(plc_recv_buffer, plc_pkgs_rx, pack_len, &pack_recved_len);
            if (pack_state == START || pack_state == GETLEN){
                switch(plc_pkgs_rx[0])
                {
                case    '?':
                    pack_len = 2;
                    if (pack_recved_len == 2)
                    {
                        plc_rq_msg.cmd = MQ_PD_STATE;
                        memcpy(pd_status_buf, plc_pkgs_rx, 2);
                        xQueueSend(plc_status_queue, ( void * )&plc_rq_msg, ( TickType_t ) 10U );      
                    }
                    break;
                case    STX:
                case    STX_RE:
                    pack_len = 3 + plc_pkgs_rx[1] + crc_len ;
                    break;
                case    ACK:
                    pack_len = 1;
                    plc_rq_msg.cmd = MQ_TX_W_ACK;
                    xQueueSend(plc_pkgs_queue, ( void * )&plc_rq_msg, ( TickType_t ) 10U );  
                    break;
                case    NAK:
                    pack_len = 1;
                    plc_rq_msg.cmd = MQ_TX_W_NAK;
                    xQueueSend(plc_pkgs_queue, ( void * )&plc_rq_msg, ( TickType_t ) 10U );                     
                    break;
                default:
                    break;
                }
            }
            if ((pack_state == PROGRESS)){ 
                //PRINTF("prlen: %d,pl: %d\r\n", pack_recved_len, pack_len);
                if (pack_recved_len == pack_len){
                    if (plc_pkgs_rx[0] == STX || plc_pkgs_rx[0] == STX_RE){
                        plc_rq_msg.cmd = MQ_RX;
                        xQueueSend(plc_pkgs_queue, ( void * )&plc_rq_msg, ( TickType_t ) 10U );     
                    }
                }
            }
        }
    }
}



void    PLC_UartFlushRxQ()
{
    Q_EMPTY(g_tRxQ);
}

void    PLC_UartFlushTxQ()
{
    Q_EMPTY(g_tTxQ);
}
/*
TS32 PLC_UartGetInQLen(void)
{
    TU16 nRet;
    
    NVIC_DisableIRQ(PLC_INT_CHANNEL);
    
    nRet = (TU16)Q_GET_L(g_tRxQ);
    
    NVIC_EnableIRQ(PLC_INT_CHANNEL);
    
    return nRet;
}*/
TU16     PLC_UartGetRxLen()
{
    TU16 nRet;

    nRet = (TU16)Q_GET_L(g_tRxQ);

    return nRet;
}
TS32 PLC_UartRead(TU8* pBuf, TU32 nBufLen)
{
    TU16    nRxLen = 0, i;

    nRxLen = (TU16)Q_GET_L(g_tRxQ);

    if (nRxLen > nBufLen) nRxLen = nBufLen;

    for (i=0; i<nRxLen; i++)
    {
        Q_READHEAD(g_tRxQ, pBuf[i]);
    }

    return nRxLen;
}
TU32 PLC_UartBufWrite(TU8* pBuf, TU32 nLen)
{
    TU16    nTxEmptyLen = 0, i;

    nTxEmptyLen = (TU16)Q_GET_E(g_tTxQ);

    if (nTxEmptyLen > nLen) nTxEmptyLen = nLen;

    if (nTxEmptyLen > 0)
    {
        for (i=0; i<nTxEmptyLen; i++)
        {
            Q_PUSHTAIL(g_tTxQ, pBuf[i]);
        }
        HW_UART_START();
    }

    return nTxEmptyLen;        
}
TBool     PLC_UartTxFinished()
{
    TU16 nRet;

    nRet = (TU16)Q_GET_L(g_tTxQ);

    if(nRet > 0) return TFalse;
    else return TTrue;
}
TU16     PLC_UartGetTxEmpty()
{
    TU16 nRet;

    nRet = (TU16)Q_GET_E(g_tTxQ);

    return nRet;
}

TU8     PLC_UartGetChar()
{
    TU8 nRet;

    Q_READHEAD(g_tRxQ, nRet);

    return nRet;
}

void    PLC_UartPutChar(TU8 nChar)
{
    Q_PUSHTAIL(g_tTxQ, nChar);

    HW_UART_START();
}

//////////////////////////////////////////////////////////////////////////
