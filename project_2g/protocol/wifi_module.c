/* FreeRTOS kernel includes. */
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "timers.h"

/* Freescale includes. */
#include "fsl_device_registers.h"
#include "fsl_debug_console.h"
#include "board.h"

#include "fsl_uart_freertos.h"
#include "fsl_uart.h"


#include "rtc_fun.h"
#include "pin_mux.h"
#include "clock_config.h"
#include "uplink_protocol.h"
#include "message.h"
#include "hb_protocol.h"
#include "lwip/netif.h"
#include "log_task.h"
#include "rtc_fun.h"
#include "uart_ppp.h"
#include "pppif.h"
#include <string.h>
#include <stdlib.h>
#include<stdio.h>
#include"wifi_module.h"
#include"board.h"



//#define WIFI_MODULE

#ifdef WIFI_MODULE


/*******************************************************************************
 * Variables
 ******************************************************************************/

uart_rtos_handle_t wifi_handle;
struct _uart_handle wifi_t_handle;
uint8_t background_buffer[UART_RX_BUF_SIZE];

//uart buf
uint8_t  G_U8_UartWifi_TXBuf[UART_TX_BUF_SIZE] = {0};
uint8_t  G_U8_UartWifi_RXBuf[UART_RX_BUF_SIZE] = {0};
uint16_t G_U16_UartWifi_RXBuf_Count = 0;

//http buf
uint8_t  G_U8_UartWifi_Http_RxBuf[UART_HTTP_BUF_SIZE] = {0};
uint8_t *G_U8_http_Buf_OffSet_addr = G_U8_UartWifi_Http_RxBuf;
uint16_t G_U16_http_total_packet_count = 0;
uint16_t G_U16_http_current_packet_count = 0;


uint32_t wifi_key_press_flag = WIFI_KEY_NOT_PRESS;
uint32_t wifi_key_press_type = WIFI_KEY_PRESS_NULL;
uint32_t wifi_smart_config_flag = NOT_WIFI_SMART_CONFIG;
uint32_t wifi_led_status_type = WIFI_LED_NULL;

uint32_t wifi_work_mode = WIFI_PROTECT_MODE;

//queue for mqtt data bufffer
CHARQ_STRUCT *mqtt_msg_queue = NULL;

//timer to select wifi state
TimerHandle_t 	TimingWifiHandle = NULL;

//msg queue for http and mqtt
extern QueueHandle_t wifi_http_msg_queue;
extern QueueHandle_t wifi_http_uart_ack_queue;
extern QueueHandle_t wifi_mqtt_msg_queue;
extern QueueHandle_t wifi_mqtt_uart_ack_queue;
extern QueueHandle_t  get_wifi_queue;
extern QueueHandle_t wifi_smartconifg_queue;

//sem_mutex for wifi send
extern SemaphoreHandle_t wifi_send_buf_sem_mutex;

UART_WIFI_RECV_STATE dataState = UART_WIFI_RECV_HEAD;

int wifi_mqtt_netconn_read(Network* n, unsigned char* buffer, int len, int timeout_ms)
{

	TickType_t xTicksToWait = timeout_ms / portTICK_PERIOD_MS; /* convert milliseconds to ticks */
	TimeOut_t xTimeOut;
	int readlen = 0;
	wifi_cmd_msg_t msg;

	vTaskSetTimeOutState(&xTimeOut); /* Record the time at which this function was entered. */
	do
	{

		if(0 < _CHARQ_SIZE(mqtt_msg_queue))
		{
			_CHARQ_DEQUEUE(mqtt_msg_queue,buffer[readlen]);
			readlen++;
		}
		else
			xQueueReceive(wifi_mqtt_msg_queue, (void*)&msg, xTicksToWait);

	}
	while(readlen < len &&  xTaskCheckForTimeOut(&xTimeOut, &xTicksToWait) == pdFALSE);

	return (readlen > 0 ? readlen : -1);

}


int  wifi_mqtt_netconn_close(Network* n)
{
	uint8_t ret;
	int timeout_ms = 5000;
	req_ack_type req_ack;
	req_ack.send_cmd = UART_MQTT_CLOSE_SERVER_CON;
	req_ack.recv_cmd = UART_MQTT_CLOSE_SERVER_CON_ACK;
	uart_wifi_sendData(NULL,0,req_ack.send_cmd);
	ret = wifi_mqtt_waitfor_ACK(req_ack.recv_cmd,wifi_mqtt_uart_ack_queue,timeout_ms);
	return ret;
}


static uint8_t uart_wifi_sendData(void *pdata,uint16_t datalen,uint16_t cmdtype)
{

	if(datalen < 0 || datalen> (UART_TX_BUF_SIZE-14))
	return RET_FAIL;
	//take sem mutex
	xSemaphoreTake(wifi_send_buf_sem_mutex,portMAX_DELAY);
	uint16_t sendLen=datalen+14; //head(4) + datalen(2) + CRC(2) + END(4) = 14
	uint16_t crc_vlue = 0;
	wifi_http_clear_buf();
	memset(G_U8_UartWifi_TXBuf,0,UART_TX_BUF_SIZE);
	//head
	G_U8_UartWifi_TXBuf[0] = 0xbe;
	G_U8_UartWifi_TXBuf[1] = 0xbe;
	G_U8_UartWifi_TXBuf[2] = 0xbe;
	G_U8_UartWifi_TXBuf[3] = 0xbe;
	//datalen
	G_U8_UartWifi_TXBuf[4] = ((sendLen>>8) & 0xFF);
	G_U8_UartWifi_TXBuf[5] = (sendLen & 0xFF);
	//cmd type
	G_U8_UartWifi_TXBuf[6] = (cmdtype>>8 & 0xFF);
	G_U8_UartWifi_TXBuf[7] = (cmdtype & 0xFF);
	//body
	if(datalen > 0 && (NULL != pdata))
	memcpy(G_U8_UartWifi_TXBuf+8,pdata,datalen);
	//CRC
	crc_vlue  =  crc_calculate(G_U8_UartWifi_TXBuf+4, sendLen-6-4);
	G_U8_UartWifi_TXBuf[sendLen-6] = (crc_vlue >> 8 & 0xFF);
	G_U8_UartWifi_TXBuf[sendLen-5] = (crc_vlue & 0xFF);
	//end
	G_U8_UartWifi_TXBuf[sendLen-4] = 0xed;
	G_U8_UartWifi_TXBuf[sendLen-3] = 0xed;
	G_U8_UartWifi_TXBuf[sendLen-2] = 0xed;
	G_U8_UartWifi_TXBuf[sendLen-1] = 0xed;
	//send data
	UART_RTOS_Send(&wifi_handle,G_U8_UartWifi_TXBuf, sendLen);
	//give sem_mutex
    xSemaphoreGive(wifi_send_buf_sem_mutex);

	return RET_OK;

}

static uint8_t parse_uart_wifi_data_packet(const uint8_t *pdata,uint8_t *userData,uint16_t *index,uint16_t *data_len)
{
	if(NULL == pdata  || NULL == userData)
			return RET_FAIL;
	uint16_t packetLen;
	packetLen = 256*pdata[0] +  pdata[1];
	if(packetLen > UART_HTTP_BUF_SIZE ||  packetLen < 12)
		return RET_FAIL;
	//for http buf
	if(index != NULL)
	{
		memcpy(userData+(*index),pdata+4,packetLen-14);
		*index = *index + (packetLen-14);
	}
	//for other buf
	else
		memcpy(userData,pdata+4,packetLen-14);

	if(data_len != NULL)
		*data_len =   (packetLen-14);

	return RET_OK;
}


static uint8_t wifi_http_clear_buf()
{
	memset(G_U8_UartWifi_Http_RxBuf,0,UART_HTTP_BUF_SIZE);
	G_U16_http_total_packet_count = 0;
	G_U16_http_current_packet_count = 0;
	G_U8_http_Buf_OffSet_addr = G_U8_UartWifi_Http_RxBuf;
	return 0;
}

static uint16_t crc_calculate(uint8_t *pdata, uint16_t dataLength)
{
	uint8_t  i;
	uint16_t crc=0;
	uint16_t lenth;
	lenth=dataLength;

	while(lenth)
	{
		crc = crc ^ ((uint16_t)(*pdata++) << 8);
		i=8;
		do
		{
			if (crc & 0x8000)
			{
				crc = (crc << 1) ^ 0x1021;
			}
			else
			{
				crc <<= 1;
			}
		} while(--i);
		lenth--;
	}
	return (crc);
}

static uint8_t uart_wifi_revdata(uint8_t data)
{

	uint16_t  uwCRC_receive = 0;
	uint16_t  uwCRCValue = 0;
	uint16_t  cmd_type = 0;
    G_U8_UartWifi_RXBuf[G_U16_UartWifi_RXBuf_Count++] = data;

    if (UART_RX_BUF_SIZE < G_U16_UartWifi_RXBuf_Count)
    {
    	PRINTF("\r\n Begin G_U8_UartWifi_RXBuf is out... \r\n");
        G_U16_UartWifi_RXBuf_Count = 0;
        memset(G_U8_UartWifi_RXBuf, 0, sizeof(G_U8_UartWifi_RXBuf));
        dataState = UART_WIFI_RECV_HEAD;
        return UART_RECV_DATA_ERROR;
    }
	switch(dataState)
	{
	case  UART_WIFI_RECV_HEAD:
        if ((4 <= G_U16_UartWifi_RXBuf_Count) && (UART_RX_BUF_SIZE >= G_U16_UartWifi_RXBuf_Count))
        {
            if ((0xBE == G_U8_UartWifi_RXBuf[G_U16_UartWifi_RXBuf_Count - 1])
                       && (0xBE == G_U8_UartWifi_RXBuf[G_U16_UartWifi_RXBuf_Count - 2])
                       && (0xBE == G_U8_UartWifi_RXBuf[G_U16_UartWifi_RXBuf_Count - 3])
                       && (0xBE == G_U8_UartWifi_RXBuf[G_U16_UartWifi_RXBuf_Count - 4]))
             {

                       G_U16_UartWifi_RXBuf_Count = 0;
                       memset(G_U8_UartWifi_RXBuf, 0, sizeof(G_U8_UartWifi_RXBuf));
                       dataState = UART_WIFI_RECV_BODY;
             }
        }
        else if ((0xED == G_U8_UartWifi_RXBuf[G_U16_UartWifi_RXBuf_Count - 1])
              && (0xED == G_U8_UartWifi_RXBuf[G_U16_UartWifi_RXBuf_Count - 2])
              && (0xED == G_U8_UartWifi_RXBuf[G_U16_UartWifi_RXBuf_Count - 3])
              && (0xED == G_U8_UartWifi_RXBuf[G_U16_UartWifi_RXBuf_Count - 4]))
       {
              G_U16_UartWifi_RXBuf_Count = 0;
              memset(G_U8_UartWifi_RXBuf, 0, sizeof(G_U8_UartWifi_RXBuf));
       }
       else if (UART_RX_BUF_SIZE <= G_U16_UartWifi_RXBuf_Count)
       {
    	 	  PRINTF("\r\n Head G_U8_UartWifi_RXBuf is out... \r\n");
              G_U16_UartWifi_RXBuf_Count = 0;
              memset(G_U8_UartWifi_RXBuf, 0, sizeof(G_U8_UartWifi_RXBuf));
       }
	   break;
	case UART_WIFI_RECV_BODY:
        if ((6 <= G_U16_UartWifi_RXBuf_Count) && (UART_RX_BUF_SIZE >= G_U16_UartWifi_RXBuf_Count))
        {
            if ((0xED == G_U8_UartWifi_RXBuf[G_U16_UartWifi_RXBuf_Count - 1])
                && (0xED == G_U8_UartWifi_RXBuf[G_U16_UartWifi_RXBuf_Count - 2])
                && (0xED == G_U8_UartWifi_RXBuf[G_U16_UartWifi_RXBuf_Count - 3])
                && (0xED == G_U8_UartWifi_RXBuf[G_U16_UartWifi_RXBuf_Count - 4]))
            {

            	cmd_type = 256 * G_U8_UartWifi_RXBuf[2] + G_U8_UartWifi_RXBuf[3];
                //Get CRC
            	uwCRC_receive = 256 * G_U8_UartWifi_RXBuf[G_U16_UartWifi_RXBuf_Count - 6]
                    + G_U8_UartWifi_RXBuf[G_U16_UartWifi_RXBuf_Count - 5];
            	uwCRCValue = crc_calculate(G_U8_UartWifi_RXBuf, (G_U16_UartWifi_RXBuf_Count - 6));
                if(uwCRC_receive == uwCRC_receive)
                {
                	//PRINTF("wifi_receive len = %d\r\n",G_U16_UartWifi_RXBuf_Count-10);
                 	dataState = UART_WIFI_RECV_SUC;
                 	return UART_RECV_DATA_SUCCESS;
                }
                PRINTF("\r\n wifi uart CRC error...\r\n");
                G_U16_UartWifi_RXBuf_Count = 0;
                memset(G_U8_UartWifi_RXBuf, 0, sizeof(G_U8_UartWifi_RXBuf));
                dataState = UART_WIFI_RECV_HEAD;
            	return UART_RECV_DATA_ERROR;

            }
            else if (UART_RX_BUF_SIZE <= G_U16_UartWifi_RXBuf_Count)
            {
             	PRINTF("\r\n  Body G_U8_UartWifi_RXBuf is out... \r\n");
                G_U16_UartWifi_RXBuf_Count = 0;
                memset(G_U8_UartWifi_RXBuf, 0, sizeof(G_U8_UartWifi_RXBuf));
                dataState = UART_WIFI_RECV_HEAD;
            }
        }

		break;
	default:
	    PRINTF("\r\nuart recv default state clear buf cout \r\n");
        G_U16_UartWifi_RXBuf_Count = 0;
        memset(G_U8_UartWifi_RXBuf, 0, sizeof(G_U8_UartWifi_RXBuf));
        dataState = UART_WIFI_RECV_HEAD;
        return UART_RECV_DATA_ERROR;
		break;
	}
	return UARTRECV_DATA_CONTINUE;
}


uint8_t  wifi_check_netconn_status(uint32_t timeout_ms)
{
	uint8_t ret;
	uart_wifi_sendData(NULL,0,UART_REQ_CON);
	ret = wifi_waitfor_ACK(UART_REQ_CON_ACK,timeout_ms);
	return ret;


}


int wifi_enter_smart_config(uint32_t timeout_ms)
{
	wifi_cmd_msg_t  wifi_msg;
	TickType_t xTicksToWait = timeout_ms / portTICK_PERIOD_MS; /* convert milliseconds to ticks */
	//send cmd to get wifi status
	uart_wifi_sendData(NULL,0,UART_CMD_SMARTCONFIG);

    //wait for wifi  smart config
	xQueueReceive(wifi_smartconifg_queue, (void*)&wifi_msg, xTicksToWait);

	return  wifi_msg.ret_code;
}


uint8_t  wifi_connect_to_server(char *url,uint32_t timeout_ms)
{
	if(NULL == url)
		return NETCONN_STATUS_CLOSE;
	uint8_t ret;
	uart_wifi_sendData(url,strlen(url),UART_CON_SERVER);

	ret = wifi_waitfor_ACK(UART_CON_SERVER_ACK,timeout_ms);
	return ret;
}


uint8_t  wifi_netconn_close(uint32_t timeout_ms)
{
	uint8_t ret;
	uart_wifi_sendData(NULL,0,UART_CLOSE_SERVER_CON);
	ret = wifi_waitfor_ACK(UART_CLOSE_SERVER_CON_ACK,timeout_ms);
	return ret;
}


uint8_t  wifi_netconn_write(const void * pdata,uint16_t datalen,uint16_t cmdtype,uint32_t timeout_ms)
{
	uint8_t ret;
	uart_wifi_sendData(pdata,datalen,cmdtype);
	PRINTF("http send datalen = %d\r\n",datalen);
	ret = wifi_waitfor_ACK(UART_SEND_DATA_ACK1,timeout_ms);
	return ret;

}


uint8_t  wifi_netconn_recv(void ** recvbuf,uint16_t *len,uint32_t timeout_ms)
{
	   if(NULL == recvbuf || NULL == len)
			return RET_FAIL;
		wifi_cmd_msg_t msg;

		if(pdTRUE == xQueueReceive(wifi_http_msg_queue, (void*)&msg, timeout_ms))
		{
			if(msg.cmd == UART_SEND_DATA_ACK2)
			{
				PRINTF("\ncmd_type = 0X%X copy wifi data datalen=%d  totalLen=%d\r\n ",msg.cmd,G_U16_http_current_packet_count,G_U16_http_total_packet_count);
				*recvbuf = G_U8_http_Buf_OffSet_addr;
				*len = G_U16_http_current_packet_count;
				G_U8_http_Buf_OffSet_addr =   G_U8_http_Buf_OffSet_addr + G_U16_http_current_packet_count;

				return RET_OK;
			}
		}

		PRINTF("wifi_waitfor_Http_server_ACK TimeOut\r\n");

	return RET_FAIL;
}

static uint8_t wifi_waitfor_ACK(uint16_t cmdtype,uint32_t timeout_ms)
{
	TickType_t xTicksToWait = timeout_ms / portTICK_PERIOD_MS;
	TimeOut_t xTimeOut;
	wifi_cmd_msg_t msg;
	vTaskSetTimeOutState(&xTimeOut);

	do{

		if(pdTRUE == xQueueReceive(wifi_http_uart_ack_queue, (void*)&msg, xTicksToWait))
		{
			if(msg.cmd == cmdtype)
			{
				PRINTF("http ACK success  Ack code  = %d\r\n ",msg.ret_code);
				wifi_http_clear_buf();
				if(msg.ret_code == RET_OK)
					return msg.ret_code;
			}
		}

	}while(xTaskCheckForTimeOut(&xTimeOut, &xTicksToWait) == pdFALSE);
	wifi_http_clear_buf();
	PRINTF("wifi_waitfor_ACK TimeOut\r\n");
	return RET_FAIL;
}

static uint8_t wifi_mqtt_waitfor_ACK(uint16_t cmdtype,QueueHandle_t  queue_type,uint32_t timeout_ms)
{
	TickType_t xTicksToWait = timeout_ms / portTICK_PERIOD_MS; /* convert milliseconds to ticks */
	TimeOut_t xTimeOut;
	wifi_cmd_msg_t msg;

	vTaskSetTimeOutState(&xTimeOut);
	do{
		if(pdTRUE == xQueueReceive(queue_type, (void*)&msg, xTicksToWait))
		{
			if(msg.cmd == cmdtype)
			{
				PRINTF("mqtt ACK success  Ack code  = %d\n ",msg.ret_code);
				return msg.ret_code;
			}
		}

	}while(xTaskCheckForTimeOut(&xTimeOut, &xTicksToWait) == pdFALSE);

	PRINTF("wifi_mqtt_waitfor_ACK  xQueueReceive  TimeOut\r\n");
	return RET_FAIL;

}

int  wifi_mqtt_connect_to_server(char *url, uint16_t port,req_ack_type *req_ack,uint32_t timeout_ms)
{

	if(NULL == url || NULL == req_ack  || timeout_ms < 0)
		return NETCONN_STATUS_CLOSE;

	char buf[128] = {0};
	uint32_t ret = RET_FAIL;
	sprintf(buf,"http://%s:%d/", url, port);
	PRINTF("mqtt con url=%s port =%d\r\n",url,port);
	PRINTF("buf url=%s\r\n",buf);
	uart_wifi_sendData(buf,strlen(buf),req_ack->send_cmd);

	ret = wifi_mqtt_waitfor_ACK(req_ack->recv_cmd,wifi_mqtt_uart_ack_queue,timeout_ms);

	return ret;
}


int  wifi_mqtt_netconn_write(Network* n, unsigned char* buffer, int len, int timeout_ms)
{
	int  ret;
	int i;
	req_ack_type  req_ack;
	req_ack.recv_cmd = UART_MQTT_SEND_DATA_ACK1;
	req_ack.send_cmd = UART_MQTT_SEND_DATA;

	uart_wifi_sendData(buffer,len,req_ack.send_cmd);
	PRINTF("mqtt send data:%d\r\n",len);
	ret = wifi_mqtt_waitfor_ACK(req_ack.recv_cmd,wifi_mqtt_uart_ack_queue,timeout_ms);
	if(RET_OK == ret)
		ret = len;
	else
		ret = -1;
	return ret;
}


int get_wifi_staus(uint16_t cmdtype,wifi_status_msg_t *wifi_staus,uint32_t timeout_ms)
{
	int ret = -1;
	TickType_t xTicksToWait = timeout_ms / portTICK_PERIOD_MS; /* convert milliseconds to ticks */
	//send cmd to get wifi status
	uart_wifi_sendData(NULL,0,UART_CMD_GET_WIFI_STATUS);

    //wait for wifi status
	if(pdTRUE == xQueueReceive(get_wifi_queue, (void*)wifi_staus, xTicksToWait))
			ret = 0;
	return  ret;
}


/*wifi task thread*/
void wifi_task(void *pvParameters)
{
	wifi_cmd_msg_t  wifi_msg;
	uint8_t recv_buffer[1];
    size_t  recved;
    uint32_t i;
    wifi_status_msg_t  wifi_status_msg;
    uint8_t data_parse_buf[1024] = {0};
    uint16_t data_parse_len = 0;

    /*queue for mqtt data*/
    mqtt_msg_queue = (void *)pvPortMalloc(QUEUE_MERMERY_SIZE);
    if(NULL == mqtt_msg_queue)
    	PRINTF("mqtt_msg_queue malloc error\r\n");
    _CHARQ_INIT(mqtt_msg_queue,QUEUE_MERMERY_SIZE);

    struct rtos_uart_config wifi_uart_config = {
        .baudrate = 115200,
        .parity = kUART_ParityDisabled,
        .stopbits = kUART_OneStopBit,
        .buffer = background_buffer,
        .buffer_size = sizeof(background_buffer),
    };
    wifi_uart_config.srcclk = CLOCK_GetFreq(WIFI_UART_CLKSRC);
    wifi_uart_config.base = WIFI_UART;

    NVIC_SetPriority(WIFI_UART_RX_TX_IRQn, 7);

    if (0 > UART_RTOS_Init(&wifi_handle, &wifi_t_handle, &wifi_uart_config))
    {
        PRINTF("Error during UART initialization.\r\n");
        vTaskSuspend(NULL);
    }


    PRINTF("-------wake up wifi task---------------\r\n");

	for(;;)
	{
		if(kStatus_Success == UART_RTOS_Receive(&wifi_handle,recv_buffer,1,&recved))
		{
			    //recv  data
				uart_wifi_revdata(recv_buffer[0]);
				//data handle
				if(UART_WIFI_RECV_SUC == dataState)
				{
					wifi_msg.cmd = 256 * G_U8_UartWifi_RXBuf[2] + G_U8_UartWifi_RXBuf[3];
					//PRINTF("CMD_TYPE = 0X%x\n",wifi_msg.cmd);
					switch(wifi_msg.cmd)
					{
					//HTTP  ACK
					case UART_REQ_CON_ACK:
						parse_uart_wifi_data_packet(G_U8_UartWifi_RXBuf,G_U8_UartWifi_Http_RxBuf,&G_U16_http_total_packet_count,&G_U16_http_current_packet_count);
						wifi_msg.ret_code = G_U8_UartWifi_Http_RxBuf[0];
						xQueueSend(wifi_http_uart_ack_queue,( void* )&wifi_msg,0);
						break;
					case UART_CON_SERVER_ACK:
						parse_uart_wifi_data_packet(G_U8_UartWifi_RXBuf,G_U8_UartWifi_Http_RxBuf,&G_U16_http_total_packet_count,&G_U16_http_current_packet_count);
						wifi_msg.ret_code = G_U8_UartWifi_Http_RxBuf[0];
						xQueueSend(wifi_http_uart_ack_queue,( void* )&wifi_msg,0);
						break;
					case UART_CLOSE_SERVER_CON_ACK:
						parse_uart_wifi_data_packet(G_U8_UartWifi_RXBuf,G_U8_UartWifi_Http_RxBuf,&G_U16_http_total_packet_count,&G_U16_http_current_packet_count);
						wifi_msg.ret_code = G_U8_UartWifi_Http_RxBuf[0];
						xQueueSend(wifi_http_uart_ack_queue,( void* )&wifi_msg,0);
					break;

					case UART_SEND_DATA_ACK1:
						parse_uart_wifi_data_packet(G_U8_UartWifi_RXBuf,G_U8_UartWifi_Http_RxBuf,&G_U16_http_total_packet_count,&G_U16_http_current_packet_count);
						wifi_msg.ret_code = G_U8_UartWifi_Http_RxBuf[0];
						xQueueSend(wifi_http_uart_ack_queue,( void* )&wifi_msg,0);
					break;
					case UART_SEND_DATA_ACK2:
						parse_uart_wifi_data_packet(G_U8_UartWifi_RXBuf,G_U8_UartWifi_Http_RxBuf,&G_U16_http_total_packet_count,&G_U16_http_current_packet_count);
						wifi_msg.ret_code = G_U8_UartWifi_Http_RxBuf[0];
						xQueueSend(wifi_http_msg_queue,( void* )&wifi_msg,0);
					break;

					//MQTT ACK
					case UART_MQTT_REQ_CON_ACK:
						parse_uart_wifi_data_packet(G_U8_UartWifi_RXBuf,data_parse_buf,NULL,NULL);
						wifi_msg.ret_code = data_parse_buf[0];
						xQueueSend(wifi_mqtt_uart_ack_queue,( void* )&wifi_msg,0);
						break;
					case UART_MQTT_CON_SERVER_ACK:
						parse_uart_wifi_data_packet(G_U8_UartWifi_RXBuf,data_parse_buf,NULL,NULL);
						wifi_msg.ret_code = data_parse_buf[0];
						xQueueSend(wifi_mqtt_uart_ack_queue,( void* )&wifi_msg,0);
						break;
					case UART_MQTT_SEND_DATA_ACK1:
						parse_uart_wifi_data_packet(G_U8_UartWifi_RXBuf,data_parse_buf,NULL,NULL);
						wifi_msg.ret_code = data_parse_buf[0];
						xQueueSend(wifi_mqtt_uart_ack_queue,( void* )&wifi_msg,0);
						break;
					//mqtt data from mqtt server
					case UART_MQTT_SEND_DATA_ACK2:
					 	PRINTF("UART_MQTT_SEND_DATA_ACK2\r\n");
					 	parse_uart_wifi_data_packet(G_U8_UartWifi_RXBuf,data_parse_buf,NULL,&data_parse_len);
					 	for(i = 0 ; i < data_parse_len ; i++)
						{
						 	_CHARQ_ENQUEUE(mqtt_msg_queue,data_parse_buf[i]);
						}
					 	PRINTF("wake up wifi_mqtt_msg_queue \r\n");
						wifi_msg.ret_code = 0;
						xQueueSend(wifi_mqtt_msg_queue,( void* )&wifi_msg,0);
						break;
					case UART_CMD_GET_WIFI_STATUS_ACK:
						parse_uart_wifi_data_packet(G_U8_UartWifi_RXBuf,&wifi_status_msg,NULL,NULL);
						//send wifi status
						xQueueSend(get_wifi_queue,(void* )&wifi_status_msg,0);
						break;
					case UART_CMD_SMARTCONFIG_ACK:
						parse_uart_wifi_data_packet(G_U8_UartWifi_RXBuf,data_parse_buf,NULL,NULL);
						wifi_msg.ret_code = data_parse_buf[0];
						xQueueSend(wifi_smartconifg_queue,(void* )&wifi_msg,0);
						break;
					default:
						break;
					}
					//clear buf
				   dataState  = UART_WIFI_RECV_HEAD;
				   G_U16_UartWifi_RXBuf_Count = 0;
				   memset(G_U8_UartWifi_RXBuf, 0, sizeof(G_U8_UartWifi_RXBuf));
				   memset(data_parse_buf, 0, sizeof(data_parse_buf));
				}
		}

	}
	  vTaskSuspend(NULL);
}

/*
*hao.yunran
*2017.3.20
*irq for sos call by sos key
*/
void wifi_key_check_task(void *pvParameters)
{
	uint32_t time = 0;//get_sys_tick();
	MODEM_SOS_INIT();
	Modem_SOS_Irq_Init();
	EnableIRQ(MODEM_SOS_IRQ);
	uint32_t key_value = 0;
	while(1)
	{
		if(wifi_key_press_flag == WIFI_KEY_PRESS)
		{
			wifi_key_press_type = WIFI_KEY_PRESS_LONG;
			time = xTaskGetTickCount();
			while((xTaskGetTickCount() - time) < WIFI_KEY_PRESS_TIMEOUT)
			{	
				key_value = MODEM_SOS_READ();
				if(WIFI_KEY_NOT_PRESS == key_value)
				{	
					PRINTF("..WIFI_KEY_PRESS_SHORT..\r\n");
					wifi_key_press_type = WIFI_KEY_PRESS_SHORT;
					break;
				}
				else
				{
					vTaskDelay(120);
				}
			}
			wifi_key_press_flag = WIFI_KEY_NOT_PRESS;
			if(wifi_key_press_type == WIFI_KEY_PRESS_LONG)
			{
				PRINTF("..WIFI_KEY_PRESS_LONG..\r\n");				
				while(RET_OK != wifi_enter_smart_config(2000));
				wifi_smart_config_flag = WIFI_SMART_CONFIG;

				wifi_led_status_type = WIFI_LED_GREEN_RED_TOGGLE;
				DisableIRQ(MODEM_SOS_IRQ);

			}
			else if(wifi_key_press_type == WIFI_KEY_PRESS_SHORT)
			{
				//short press
				 if(WIFI_PROTECT_MODE == wifi_work_mode)
					 wifi_work_mode = WIFI_NORMAL_MODE;
				 else
					 wifi_work_mode = WIFI_PROTECT_MODE;
			}
		}
		else
		{
			key_value = MODEM_SOS_READ();
			if(WIFI_KEY_NOT_PRESS == key_value)
			{	
				wifi_key_press_flag = WIFI_KEY_NOT_PRESS;
			}
			vTaskDelay(WIFI_KEY_CHECK_INTER);
		}

	}

}

/*
*hao.yunran
*2017.3.22
*irq for sos call by sos key
*/
void wifi_led_status_task(void *pvParameters)
{
	wifi_status_msg_t  wifi_status;
	WIFI_RED_LED_INIT();
	WIFI_GREEN_LED_INIT();
	WIFI_RED_LED_OFF();
	WIFI_GREEN_LED_OFF();
	vTaskDelay(500);
	while(1)
	{

    	get_wifi_staus(UART_CMD_GET_WIFI_STATUS,&wifi_status,1000);
    	if(WIFI_CON_OK == wifi_status.con_status)
    	{
    		if(WIFI_NORMAL_MODE == wifi_work_mode)wifi_led_status_type = WIFI_LED_GREEN;
    		else if(WIFI_PROTECT_MODE == wifi_work_mode)wifi_led_status_type = WIFI_LED_GREEN_TOGGLE;

    		if(WIFI_KEY_PRESS_LONG == wifi_key_press_type)
    				EnableIRQ(MODEM_SOS_IRQ);
    	}
    	else
    	{
    		if(WIFI_KEY_PRESS_LONG == wifi_key_press_type)wifi_led_status_type = WIFI_LED_GREEN_RED_TOGGLE;
    		else wifi_led_status_type = WIFI_LED_NULL;
    	}


		switch(wifi_led_status_type)
		{
			case WIFI_LED_NULL:
				WIFI_RED_LED_ON();
				WIFI_GREEN_LED_ON();
				break;
				
			case WIFI_LED_GREEN:
				WIFI_RED_LED_OFF();
				WIFI_GREEN_LED_ON();

				break;
				
			case WIFI_LED_RED:
				WIFI_RED_LED_ON();
				WIFI_GREEN_LED_OFF();
				break;
				
			case WIFI_LED_GREEN_RED:
				WIFI_RED_LED_ON();
				WIFI_GREEN_LED_ON();
				break;
				
			case WIFI_LED_GREEN_TOGGLE:
				WIFI_RED_LED_OFF();
				WIFI_GREEN_LED_TOGGLE();
				break;
				
			case WIFI_LED_RED_TOGGLE:
				WIFI_RED_LED_TOGGLE();
				WIFI_GREEN_LED_OFF();
				break;
										
			case WIFI_LED_GREEN_RED_TOGGLE:
				WIFI_RED_LED_ON();
				WIFI_GREEN_LED_ON();
				vTaskDelay(500);
				WIFI_RED_LED_OFF();
				WIFI_GREEN_LED_OFF();
				break;
				
			default:
				WIFI_RED_LED_ON();
				WIFI_GREEN_LED_OFF();
				PRINTF("..wifi_led_status_err...\n");
				break;
				
		}
		vTaskDelay(1000);
	}
}


#endif
