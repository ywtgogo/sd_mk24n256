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
#include"uartEncodeDecode.h"
#include"crc.h"



#ifdef WIFI_MODULE

/*******************************************************************************
 * Variables
 ******************************************************************************/

uart_rtos_handle_t wifi_handle;
struct _uart_handle wifi_t_handle;
uint8_t background_buffer[UART_RX_BUF_SIZE];

uint8_t G_U8_UartWifi_RXBuf[UART_RX_BUF_SIZE] = { 0 };
uint8_t G_U8_Wifi_Parse_Buf[UART_RX_BUF_SIZE] = { 0 };
uint16_t G_U16_UartWifi_RXBuf_Count = 0;
uint16_t data_parse_len = 0;

uint32_t wifi_key_press_flag = WIFI_KEY_NOT_PRESS;
uint32_t wifi_key_press_type = WIFI_KEY_PRESS_NULL;
uint32_t wifi_smart_config_flag = NOT_WIFI_SMART_CONFIG;
uint32_t wifi_led_status_type = WIFI_LED_NULL;

uint32_t wifi_work_mode = WIFI_PROTECT_MODE;
uint32_t wifi_uart_current_status = K24_UART_STATUS_LIVE;
uint32_t wifi_uart_current_status_count = 0;
uint32_t wifi_connect_state_flag  =  WIFI_CONNECT_STAT_DIS;

uint32_t wifi_socket_index = 0;
uint16_t cmd_type = UART_CMD_IDLE;

CHARQ_STRUCT *wifi_receive_buf[WIFI_SOCKET_MAX];

//timer to select wifi state
TimerHandle_t TimingWifiHandle = NULL;

extern uint32_t aging_test_flag;
extern QueueHandle_t check_wifi_con_state_queue;
extern QueueHandle_t get_wifi_queue;
extern QueueHandle_t wifi_smartconifg_queue;

extern QueueHandle_t wifi_rssi_queue;
extern QueueHandle_t wifi_ver_info_queue;
extern QueueHandle_t wifi_config_info_queue;

//sem_mutex for wifi send
extern SemaphoreHandle_t wifi_send_buf_sem_mutex;
extern SemaphoreHandle_t wifi_parse_url_sem_mutex;

extern QueueHandle_t wifi_parse_url_queue[DNS_SOCKET_MAX];
extern QueueHandle_t wifi_socket_discon_queue[WIFI_SOCKET_MAX];
extern QueueHandle_t wifi_socket_con_queue[WIFI_SOCKET_MAX];
extern QueueHandle_t wifi_socket_write_queue[WIFI_SOCKET_MAX];
extern QueueHandle_t wifi_socket_read_queue[WIFI_SOCKET_MAX];

UART_WIFI_RECV_STATE dataState = UART_WIFI_RECV_HEAD;

WIFI_SOCKET wifi_socket_buf = { .wifi_socket_cache[0].socket_index = 1,
		.wifi_socket_cache[0].socket_index_flag = WIFI_SOCKET_NOT_USED,
		.wifi_socket_cache[0].socket_proto_type = WIFI_SOCKET_NONE,
		.wifi_socket_cache[1].socket_index = 2,
		.wifi_socket_cache[1].socket_index_flag = WIFI_SOCKET_NOT_USED,
		.wifi_socket_cache[1].socket_proto_type = WIFI_SOCKET_NONE,
		.wifi_socket_cache[2].socket_index = 3,
		.wifi_socket_cache[2].socket_index_flag = WIFI_SOCKET_NOT_USED,
		.wifi_socket_cache[2].socket_proto_type = WIFI_SOCKET_NONE,
		.wifi_socket_cache[3].socket_index = 4,
		.wifi_socket_cache[3].socket_index_flag = WIFI_SOCKET_NOT_USED,
		.wifi_socket_cache[3].socket_proto_type = WIFI_SOCKET_NONE,

};

WIFI_SOCKET wifi_parse_url_buf = { .wifi_socket_cache[0].socket_index = 0,
		.wifi_socket_cache[0].socket_index_flag = WIFI_SOCKET_NOT_USED,
		.wifi_socket_cache[0].socket_proto_type = WIFI_SOCKET_NONE,
		.wifi_socket_cache[1].socket_index = 1,
		.wifi_socket_cache[1].socket_index_flag = WIFI_SOCKET_NOT_USED,
		.wifi_socket_cache[1].socket_proto_type = WIFI_SOCKET_NONE };


#ifdef PRODUCE_TEST
signed int uart_wifi_sendData(void *pdata, uint16_t datalen, uint16_t cmdtype)
#else
static s32_t uart_wifi_sendData(void *pdata	, uint16_t datalen, uint16_t cmdtype)
#endif
{
	if (datalen < 0 )
		return RET_FAIL;
	xSemaphoreTake(wifi_send_buf_sem_mutex, portMAX_DELAY);
	unsigned long crc32Value = 0;
	uint16_t iencodeLen = datalen + 8;
	uint16_t oEncodeLen = 0;
    uint16_t uarSendLen = 0;

    unsigned char * iEncodeBuf = (unsigned char *) pvPortMalloc(iencodeLen);
    unsigned char * oEncodeBuf = (unsigned char *) pvPortMalloc(iencodeLen*2);
    if(NULL == iEncodeBuf || NULL == oEncodeBuf)
    {
    	if(NULL != iEncodeBuf)
    		vPortFree(iEncodeBuf);
    	if(NULL != oEncodeBuf)
    		vPortFree(oEncodeBuf);
    	PRINTF("NO1 RAM for uart_wifi_sendData()...\r\n");
    	return RET_FAIL;
    }
	memset(oEncodeBuf, 0, iencodeLen*2);
	memset(iEncodeBuf, 0,iencodeLen);
	iEncodeBuf[0] = (iencodeLen & 0xFF);
	iEncodeBuf[1] = ((iencodeLen >> 8) & 0xFF);
	iEncodeBuf[2] = (cmdtype & 0xFF);
	iEncodeBuf[3] = ((cmdtype >> 8) & 0xFF);
	if(NULL != pdata)
		memcpy(iEncodeBuf + 4, pdata, datalen);
	crc32Value = Crc32_Calc(iEncodeBuf,iencodeLen-4);

	iEncodeBuf[iencodeLen - 4] = (crc32Value & 0xFF);
	iEncodeBuf[iencodeLen - 3] = ((crc32Value >> 8)& 0xFF);
	iEncodeBuf[iencodeLen - 2] = ((crc32Value >> 16) & 0xFF);
	iEncodeBuf[iencodeLen - 1] = ((crc32Value >> 24)& 0xFF);

	oEncodeBuf[0] = HEAD0;
	oEncodeBuf[1] = HEAD1;
	oEncodeBuf[2] = HEAD2;
	oEncodeBuf[3] = HEAD3;
	oEncodeLen = uartEncode(iEncodeBuf,iencodeLen,oEncodeBuf + 4);
	uarSendLen = oEncodeLen + 8;

	oEncodeBuf[uarSendLen - 4] = END0;
	oEncodeBuf[uarSendLen - 3] = END1;
	oEncodeBuf[uarSendLen - 2] = END2;
	oEncodeBuf[uarSendLen - 1] = END3;
	vTaskDelay(50);  //wait for esp8266 send callback send complete
	//send data
	UART_RTOS_Send(&wifi_handle, oEncodeBuf, uarSendLen);

	vPortFree(iEncodeBuf);
	vPortFree(oEncodeBuf);
	xSemaphoreGive(wifi_send_buf_sem_mutex);

	return RET_OK;
}

static s32_t parse_uart_wifi_data_packet(const uint8_t *pdata,uint16_t len,uint8_t *userData)
{
	if(len < 0 || NULL == pdata || NULL == userData)
		return RET_FAIL;
	unsigned long uwCRC_receive = 0;
	unsigned long uwCRCValue = 0;
	int i;
	int ret = RET_FAIL;
	int decodeLen = 0;
	unsigned char * decodeBuf = (unsigned char *)pvPortMalloc(len);
	if(NULL == decodeBuf)
	{
		cmd_type = UART_CMD_IDLE;
		PRINTF("NO RAM for parse_uart_wifi_data_packet()...\r\n");
		return RET_FAIL;
	}
	memset(decodeBuf,0,len);
	decodeLen =  uartDecode(pdata,len,decodeBuf);
	data_parse_len = decodeLen - 8;
	 uwCRC_receive =    decodeBuf[decodeLen - 4] |
						    ((decodeBuf[decodeLen - 3] & 0x000000FF) << 8)  |
							((decodeBuf[decodeLen - 2] & 0x000000FF) << 16) |
							((decodeBuf[decodeLen - 1] & 0x000000FF) << 24) ;

	uwCRCValue = Crc32_Calc(decodeBuf,(decodeLen - 4));
	if(uwCRCValue == uwCRC_receive)
	{
		cmd_type = decodeBuf[2] + 256 *  decodeBuf[3];
		memcpy(userData,decodeBuf+4,data_parse_len);
		ret  = RET_OK;
	}
	else
	{
		PRINTF("\r\n wifi uart CRC32 error。。。。。。。。。。。。。。\r\n");
		PRINTF("uwCRC_receive:%x -- uwCRCValue:%x\r\n",uwCRC_receive,uwCRCValue);
		cmd_type = UART_CMD_IDLE;
	}

	vPortFree(decodeBuf);
	return ret;
}

static u16_t wifi_htons(u16_t n) {
	return ((n & 0xff) << 8) | ((n & 0xff00) >> 8);
}



static s32_t uart_wifi_revdata(uint8_t data) {
	G_U8_UartWifi_RXBuf[G_U16_UartWifi_RXBuf_Count++] = data;

	if (UART_RX_BUF_SIZE < G_U16_UartWifi_RXBuf_Count) {
		PRINTF("\r\n Begin G_U8_UartWifi_RXBuf is out... \r\n");
		G_U16_UartWifi_RXBuf_Count = 0;
		memset(G_U8_UartWifi_RXBuf, 0, sizeof(G_U8_UartWifi_RXBuf));
		dataState = UART_WIFI_RECV_HEAD;
		return UART_RECV_DATA_ERROR;
	}
	switch (dataState) {
	case UART_WIFI_RECV_HEAD:
		if ((4 <= G_U16_UartWifi_RXBuf_Count)
				&& (UART_RX_BUF_SIZE >= G_U16_UartWifi_RXBuf_Count)) {
			if ((HEAD3 == G_U8_UartWifi_RXBuf[G_U16_UartWifi_RXBuf_Count - 1])
					&& (HEAD2
							== G_U8_UartWifi_RXBuf[G_U16_UartWifi_RXBuf_Count
									- 2])
					&& (HEAD1
							== G_U8_UartWifi_RXBuf[G_U16_UartWifi_RXBuf_Count
									- 3])
					&& (HEAD0
							== G_U8_UartWifi_RXBuf[G_U16_UartWifi_RXBuf_Count
									- 4])) {

				G_U16_UartWifi_RXBuf_Count = 0;
				memset(G_U8_UartWifi_RXBuf, 0, sizeof(G_U8_UartWifi_RXBuf));
				dataState = UART_WIFI_RECV_BODY;
			}
		} else if ((END3 == G_U8_UartWifi_RXBuf[G_U16_UartWifi_RXBuf_Count - 1])
				&& (END2 == G_U8_UartWifi_RXBuf[G_U16_UartWifi_RXBuf_Count - 2])
				&& (END1 == G_U8_UartWifi_RXBuf[G_U16_UartWifi_RXBuf_Count - 3])
				&& (END0 == G_U8_UartWifi_RXBuf[G_U16_UartWifi_RXBuf_Count - 4])) {

			G_U16_UartWifi_RXBuf_Count = 0;
			memset(G_U8_UartWifi_RXBuf, 0, sizeof(G_U8_UartWifi_RXBuf));

		} else if (UART_RX_BUF_SIZE <= G_U16_UartWifi_RXBuf_Count) {
			PRINTF("\r\n Head G_U8_UartWifi_RXBuf is out... \r\n");
			G_U16_UartWifi_RXBuf_Count = 0;
			memset(G_U8_UartWifi_RXBuf, 0, sizeof(G_U8_UartWifi_RXBuf));
		}
		break;
	case UART_WIFI_RECV_BODY:
		if ((6 <= G_U16_UartWifi_RXBuf_Count)&& (UART_RX_BUF_SIZE >= G_U16_UartWifi_RXBuf_Count)) {
			if ((END3 == G_U8_UartWifi_RXBuf[G_U16_UartWifi_RXBuf_Count - 1])
					&& (END2
							== G_U8_UartWifi_RXBuf[G_U16_UartWifi_RXBuf_Count
									- 2])
					&& (END1
							== G_U8_UartWifi_RXBuf[G_U16_UartWifi_RXBuf_Count
									- 3])
					&& (END0
							== G_U8_UartWifi_RXBuf[G_U16_UartWifi_RXBuf_Count
									- 4])) {
					dataState = UART_WIFI_RECV_SUC;
			} else if (UART_RX_BUF_SIZE <= G_U16_UartWifi_RXBuf_Count) {

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



s32_t wifi_get_rssi(uint32_t timeout_ms)
{
	uart_wifi_sendData(NULL, 0, UART_CMD_WIFI_RSSI);
	TickType_t xTicksToWait = timeout_ms / portTICK_PERIOD_MS;
    char rssi_level;
	if (pdTRUE
			== xQueueReceive(wifi_rssi_queue, (void* )&rssi_level,
					xTicksToWait)) {
			return rssi_level;
	}
	return -1;
}
s32_t wifi_get_ver_info(wifi_ver_info *ver_info,uint32_t timeout_ms)
{
	uart_wifi_sendData(NULL, 0, UART_CMD_WIFI_VER_INFO);
	TickType_t xTicksToWait = timeout_ms / portTICK_PERIOD_MS;
	if( pdTRUE == xQueueReceive(wifi_ver_info_queue, (void* )ver_info,xTicksToWait))
		return 0;
	else
		return -1;
}


s32_t wifi_get_config_info(struct station_config *config,uint32_t timeout_ms)
{
	uart_wifi_sendData(NULL, 0, UART_CMD_WIFI_STA_INFO);
	TickType_t xTicksToWait = timeout_ms / portTICK_PERIOD_MS;
	if( pdTRUE == xQueueReceive(wifi_config_info_queue, (void* )config,xTicksToWait))
		return 0;
	else
		return -1;
}

s32_t wifi_check_netconn_status(uint32_t timeout_ms) {

	uart_wifi_sendData(NULL, 0, UART_REQ_CON);
	TickType_t xTicksToWait = timeout_ms / portTICK_PERIOD_MS;
	TimeOut_t xTimeOut;
	wifi_cmd_msg_t msg;
	vTaskSetTimeOutState(&xTimeOut);

	do {

		if (pdTRUE
				== xQueueReceive(check_wifi_con_state_queue, (void* )&msg,
						xTicksToWait)) {
			if (RET_OK == msg.ret_code)
				return RET_OK;
		}

	} while (xTaskCheckForTimeOut(&xTimeOut, &xTicksToWait) == pdFALSE);

	return RET_FAIL;
}

s32_t wifi_enter_smart_config(uint32_t timeout_ms) {
	wifi_cmd_msg_t wifi_msg;
	TickType_t xTicksToWait = timeout_ms / portTICK_PERIOD_MS; /* convert milliseconds to ticks */
	//send cmd to get wifi status
	uart_wifi_sendData(NULL, 0, UART_CMD_SMARTCONFIG);

	//wait for wifi  smart config
	xQueueReceive(wifi_smartconifg_queue, (void* )&wifi_msg, xTicksToWait);

	return wifi_msg.ret_code;
}

s32_t get_wifi_staus(uint16_t cmdtype, wifi_status_msg_t *wifi_staus,
		uint32_t timeout_ms) {
	int ret = -1;
	TickType_t xTicksToWait = timeout_ms / portTICK_PERIOD_MS; /* convert milliseconds to ticks */
	//send cmd to get wifi status
	uart_wifi_sendData(NULL, 0, UART_CMD_GET_WIFI_STATUS);

	//wait for wifi status
	if (pdTRUE == xQueueReceive(get_wifi_queue, (void* )wifi_staus, xTicksToWait))
	{
		wifi_uart_current_status_count = 0;
		wifi_uart_current_status = K24_UART_STATUS_LIVE;
		return RET_OK;
	}

	if(wifi_uart_current_status_count < WIFI_UART_CURRENT_TIMEOUT)
	{
		wifi_uart_current_status_count++;
	}
	else
	{
		wifi_uart_current_status = K24_UART_STATUS_DEAD;
		PRINTF("WIFI MODULE IS DEAD ,SBOX WILL BE RESTART....................\r\n");
	}

	return RET_FAIL;
}

s32_t wifi_gethostbyname(const char *name, ip_addr_t *addr) {
	PRINTF("wifi_gethostbyname()...\r\n");
	if (NULL == name || NULL == addr) {
		PRINTF("WIFI DNS Fail...name or addr is NULL\n");
		return RET_SOCKET_FAIL;
	}

	xSemaphoreTake(wifi_parse_url_sem_mutex, portMAX_DELAY);
	u32_t i = 0;
	uint8_t *temp;
	PARSE_URL_ACK_T url_ack;
	uint32_t socket_index = 0;

	for (i = DNS_SOCKET_MIN; i < DNS_SOCKET_MAX; i++) {
		if (wifi_parse_url_buf.wifi_socket_cache[i].socket_index_flag
				== WIFI_SOCKET_NOT_USED) {
			wifi_parse_url_buf.wifi_socket_cache[i].socket_index_flag =
					WIFI_SOCKET_USED;
			socket_index = wifi_parse_url_buf.wifi_socket_cache[i].socket_index;
			break;
		}
	}

	if (socket_index < DNS_SOCKET_MIN || socket_index >= DNS_SOCKET_MAX) {
		PRINTF("WIFI DNS Fail...no socket for DNS..\r\n");
		return RET_SOCKET_FAIL;
	}

	PARSE_URL_SEND_T buf;
	memset(&buf, 0, sizeof(PARSE_URL_SEND_T));
	buf.socket_index = socket_index;
	memcpy(buf.url, name, strlen(name));

	uart_wifi_sendData(&buf, sizeof(buf), UART_CMD_PARSE_URL_IP);

	xSemaphoreGive(wifi_parse_url_sem_mutex);

	if (pdTRUE== xQueueReceive(wifi_parse_url_queue[socket_index],
					(void* )&url_ack, 25000)) {

		wifi_parse_url_buf.wifi_socket_cache[socket_index].socket_index_flag = WIFI_SOCKET_NOT_USED;
		if((ACK_OK == url_ack.ack_code) && (url_ack.socket_index == socket_index))
		{
			memcpy(addr, &url_ack.addr, sizeof(ip_addr_t));
			temp = (uint8_t*) &url_ack.addr;
			PRINTF("WIFI DNS Success..socket id:%d server ip %d.%d.%d.%d\r\n",
					socket_index, temp[0], temp[1], temp[2], temp[3]);
			return RET_SOCKET_OK;
		}
		return RET_SOCKET_FAIL;
	} else {
		wifi_parse_url_buf.wifi_socket_cache[socket_index].socket_index_flag =
				WIFI_SOCKET_NOT_USED;
		PRINTF("WIFI DNS Fail...TimeOUT..socket=%d\r\n", socket_index);
		return RET_SOCKET_FAIL;
	}
}

s32_t wifi_socket(int domain, int type, int protocol) {
	if (WIFI_HTTP_CMD == type)
	{
		if(wifi_socket_buf.wifi_socket_cache[0].socket_index_flag == WIFI_SOCKET_NOT_USED)
		{
			 wifi_socket_buf.wifi_socket_cache[0].socket_index_flag = WIFI_SOCKET_USED;
			 return WIFI_HTTP_CMD;
		}
	}
	else if (WIFI_MQTT_CMD == type)
	{
		if(wifi_socket_buf.wifi_socket_cache[1].socket_index_flag == WIFI_SOCKET_NOT_USED)
		{
			 wifi_socket_buf.wifi_socket_cache[1].socket_index_flag = WIFI_SOCKET_USED;
				return WIFI_MQTT_CMD;
		}

	}
	return -1;
}

s32_t wifi_closesocket(int s) {

	PRINTF("wifi_closesocket(%d)\r\n", s);
	if (s < WIFI_SOCKET_MIN || s > WIFI_SOCKET_MAX)
		return RET_SOCKET_FAIL;

	//Init queue
	_CHARQ_INIT(wifi_receive_buf[s-1], QUEUE_MERMERY_SIZE);

	uint8_t index = s;
	WIFI_DISCON_T dis_ack;
	uart_wifi_sendData(&index, sizeof(index), UART_CMD_SOCKET_DISCON);
	if (pdTRUE
			== xQueueReceive(wifi_socket_discon_queue[index - 1],
					(void* )&dis_ack, 2000)) {
		if ((index == dis_ack.socket_index) && ACK_OK == dis_ack.ack_code) {
			wifi_socket_buf.wifi_socket_cache[index - 1].socket_index_flag =
					WIFI_SOCKET_NOT_USED;
			return RET_SOCKET_OK;
		} else {
			wifi_socket_buf.wifi_socket_cache[index - 1].socket_index_flag =
					WIFI_SOCKET_NOT_USED;
			return RET_SOCKET_FAIL;
		}
	}

	PRINTF(" wifi_closesocket FAIL..TimeOut.index=%d\r\n", s);
	wifi_socket_buf.wifi_socket_cache[index - 1].socket_index_flag =
			WIFI_SOCKET_NOT_USED;
	return RET_SOCKET_FAIL;
}

s32_t wifi_connect(int s, const struct sockaddr *name, socklen_t namelen) {

	struct sockaddr_in s_addr;
	int socket_id = s;
	int index = s;
	if ((socket_id < WIFI_SOCKET_MIN || socket_id > WIFI_SOCKET_MAX)
			|| (NULL == name) || (namelen < 0)) {
		PRINTF(" wifi_connect FAIL..parameter error..\r\n");
		return RET_SOCKET_FAIL;
	}
	socket_id = socket_id - 1;

	if (WIFI_SOCKET_NOT_USED
			== wifi_socket_buf.wifi_socket_cache[socket_id].socket_index_flag) {
		PRINTF(" wifi_connect FAIL..socket %d can not use..\r\n", s);
		return RET_SOCKET_FAIL;
	}

	memcpy(&s_addr, name, sizeof(s_addr));
	s_addr.sin_port = wifi_htons(s_addr.sin_port);
	unsigned char * temp = (unsigned char *) &s_addr.sin_addr;

	PRINTF("connetc server ip %d.%d.%d.%d port:%d\r\n", temp[0], temp[1],
			temp[2], temp[3], s_addr.sin_port);

	WIFI_CON_T wifi_con;
	wifi_con.socket_index = s;

	memcpy(&wifi_con.addr, &s_addr, sizeof(wifi_con.addr));

	uart_wifi_sendData(&wifi_con, sizeof(WIFI_CON_T), UART_CMD_SOCKET_CON);

	//wait for connetc ack
	if (pdTRUE
			== xQueueReceive(wifi_socket_con_queue[socket_id],
					(void* )&wifi_con, 25000)) {
		if ((index == wifi_con.socket_index) && ACK_OK == wifi_con.ack_code) {
			PRINTF(" wifi_con Success...socket id:%d\r\n", index);
			return RET_SOCKET_OK;
		} else {
			PRINTF(" wifi_con FAIL..ack error. socket id:%d\r\n", s);
			return RET_SOCKET_FAIL;
		}
	}

	PRINTF(" wifi_con FAIL..TimeOut.socket id:%d\r\n", index);
	return RET_SOCKET_FAIL;

}

s32_t wifi_write(int socket_n, unsigned char* buffer, int len, int timeout_ms) {
	PRINTF("wifi_write(%d,,)..\r\n", socket_n);
	if ((socket_n < WIFI_SOCKET_MIN || socket_n > WIFI_SOCKET_MAX)
			|| (NULL == buffer) || (timeout_ms < 0)) {
		PRINTF(" wifi_write FAIL..parameter error..\r\n");
		return 0;
	}

	WIFI_DISCON_T wite_ack;
	int index = socket_n;

	uint8_t *pSendData = (uint8_t*) pvPortMalloc(len + 1);
	if(NULL == pSendData)
	{
		PRINTF("NO RAM for wifi_write()...\r\n");
		return 0;
	}
	memset(pSendData, 0, len + 1);
	pSendData[0] = socket_n;
	memcpy(pSendData + 1, buffer, len);
	PRINTF("\r\n write len:%d\r\n", len);
	uart_wifi_sendData(pSendData, len + 1, UART_CMD_SOCKET_WRITE);

	vPortFree(pSendData);
	index = index - 1;
	if (pdTRUE
			== xQueueReceive(wifi_socket_write_queue[index], (void* )&wite_ack,
					timeout_ms)) {
		if (ACK_OK == wite_ack.ack_code && socket_n == wite_ack.socket_index) {
			PRINTF(" wifi_write Success...socket id:%d\r\n", socket_n);
			return len;
		} else {
			PRINTF(" wifi_write FAIL..ack error.socket id:%d\r\n", socket_n);
			return 0;
		}
	}

	PRINTF(" wifi_write FAIL..TimeOut.socket id:%d\r\n", socket_n);
	return 0;
}

s32_t wifi_read(int socket_n, unsigned char* buffer, int len, int timeout_ms) {

	if ((socket_n < WIFI_SOCKET_MIN || socket_n > WIFI_SOCKET_MAX)
			|| (NULL == buffer) || len < 0 || (timeout_ms < 0)) {
		PRINTF(" wifi_read FAIL..parameter error..\r\n");
		return RET_SOCKET_FAIL;
	}
	TickType_t xTicksToWait = timeout_ms / portTICK_PERIOD_MS; /* convert milliseconds to ticks */
	TimeOut_t xTimeOut;
	int readlen = 0;
	WIFI_DISCON_T read_ack;

	vTaskSetTimeOutState(&xTimeOut); /* Record the time at which this function was entered. */
	do {

		if (0 < _CHARQ_SIZE(wifi_receive_buf[socket_n - 1])) {

			_CHARQ_DEQUEUE(wifi_receive_buf[socket_n - 1], buffer[readlen]);
			readlen++;

		} else
			xQueueReceive(wifi_socket_read_queue[socket_n - 1],
					(void* )&read_ack, xTicksToWait);

	} while (readlen < len
			&& xTaskCheckForTimeOut(&xTimeOut, &xTicksToWait) == pdFALSE);


	return (readlen > 0 ? readlen : 0);
}



/*wifi task thread*/
void wifi_task(void *pvParameters) {
	wifi_cmd_msg_t wifi_msg;
	uint8_t recv_buffer[1];
	uint8_t ack_code;
	size_t recved;
	uint32_t i;
	wifi_ver_info ver_info;
	struct station_config config;
	uint8_t rssi_level;
	wifi_status_msg_t wifi_status_msg;

	WIFI_DISCON_T dis_ack;
	WIFI_CON_T wifi_con;
	uint8_t index;

	PARSE_URL_ACK_T url_ack;

	for (int i = 0; i < WIFI_SOCKET_MAX; i++) {
		wifi_receive_buf[i] = (void *) pvPortMalloc(QUEUE_MERMERY_SIZE + QUEUE_HEAD_SIZE);
		if (NULL == wifi_receive_buf[i])
			PRINTF("buf queue  malloc error wifi_receive_buf[%d]\r\n", i);
		_CHARQ_INIT(wifi_receive_buf[i], QUEUE_MERMERY_SIZE);

	}

	struct rtos_uart_config wifi_uart_config = { .baudrate = 115200, .parity =
			kUART_ParityDisabled, .stopbits = kUART_OneStopBit, .buffer =
			background_buffer, .buffer_size = sizeof(background_buffer), };
	wifi_uart_config.srcclk = CLOCK_GetFreq(WIFI_UART_CLKSRC);
	wifi_uart_config.base = WIFI_UART;

	NVIC_SetPriority(WIFI_UART_RX_TX_IRQn, 7);

	if (0 > UART_RTOS_Init(&wifi_handle, &wifi_t_handle, &wifi_uart_config)) {
		PRINTF("Error during UART initialization.\r\n");
		vTaskSuspend(NULL);
	}

	PRINTF("-------wake up wifi task---------------\r\n");

	for (;;) {
		if (kStatus_Success== UART_RTOS_Receive(&wifi_handle, recv_buffer, 1, &recved)) {
			uart_wifi_revdata(recv_buffer[0]);
			if (UART_WIFI_RECV_SUC == dataState) {
				parse_uart_wifi_data_packet(G_U8_UartWifi_RXBuf,(G_U16_UartWifi_RXBuf_Count-4),G_U8_Wifi_Parse_Buf);
				switch (cmd_type) {
				case UART_REQ_CON_ACK:
					    PRINTF("UART_REQ_CON_ACK...\r\n");
						wifi_msg.ret_code = G_U8_Wifi_Parse_Buf[0];
						xQueueSend(check_wifi_con_state_queue, (void* )&wifi_msg,0);
					break;
				case UART_CMD_GET_WIFI_STATUS_ACK:
					memcpy(&wifi_status_msg,G_U8_Wifi_Parse_Buf,sizeof(wifi_status_msg));
					//send wifi status
					xQueueSend(get_wifi_queue, (void* )&wifi_status_msg, 0);
					break;
				case UART_CMD_SMARTCONFIG_ACK:
				    PRINTF("UART_CMD_SMARTCONFIG_ACK...\r\n");
					wifi_msg.ret_code = G_U8_Wifi_Parse_Buf[0];
					xQueueSend(wifi_smartconifg_queue, (void* )&wifi_msg, 0);
					break;
				case UART_CMD_PARSE_URL_IP_ACK:
				    PRINTF("UART_CMD_PARSE_URL_IP_ACK...\r\n");
					memcpy(&url_ack, G_U8_Wifi_Parse_Buf, sizeof(url_ack));
					if (url_ack.socket_index >= DNS_SOCKET_MIN
							&& url_ack.socket_index < DNS_SOCKET_MAX) {
						xQueueSend(wifi_parse_url_queue[url_ack.socket_index],
								(void* )&url_ack, 0);
					}
					break;
				case UART_CMD_SOCKET_DISCON_ACK:
				    PRINTF("UART_CMD_SOCKET_DISCON_ACK...\r\n");
					memcpy(&dis_ack, G_U8_Wifi_Parse_Buf, sizeof(dis_ack));
					if (dis_ack.socket_index >= WIFI_SOCKET_MIN
							&& dis_ack.socket_index <= WIFI_SOCKET_MAX)
						xQueueSend(
								wifi_socket_discon_queue[dis_ack.socket_index
										- 1], (void* )&dis_ack, 0);

					break;
				case UART_CMD_SOCKET_CON_ACK:
				    PRINTF("UART_CMD_SOCKET_CON_ACK...\r\n");
					memcpy(&wifi_con, G_U8_Wifi_Parse_Buf, sizeof(wifi_con));
					if (wifi_con.socket_index >= WIFI_SOCKET_MIN
							&& wifi_con.socket_index <= WIFI_SOCKET_MAX) {
						index = (wifi_con.socket_index - 1);
						xQueueSend(wifi_socket_con_queue[index],
								(void* )&wifi_con, 0);
					}
					break;
				case UART_CMD_SOCKET_READ_ACK:
					memcpy(&dis_ack, G_U8_Wifi_Parse_Buf, sizeof(dis_ack));
				    PRINTF("UART_CMD_SOCKET_READ_ACK:%d...\r\n",dis_ack.socket_index);
					if (dis_ack.socket_index >= WIFI_SOCKET_MIN
							&& dis_ack.socket_index <= WIFI_SOCKET_MAX) {
						for (i = 2; i < data_parse_len; i++) {

							if(_CHARQ_NOT_FULL(wifi_receive_buf[dis_ack.socket_index - 1]))
							{
								_CHARQ_ENQUEUE(
										wifi_receive_buf[dis_ack.socket_index - 1],
										G_U8_Wifi_Parse_Buf[i]);
							}

						}
						xQueueSend(
								wifi_socket_read_queue[dis_ack.socket_index - 1],
								(void* )&dis_ack, 0);
					}
				case UART_CMD_SOCKET_WRITE_ACK:

					memcpy(&dis_ack, G_U8_Wifi_Parse_Buf, sizeof(dis_ack));
					PRINTF("UART_CMD_SOCKET_WRITE_ACK:%d...\r\n",dis_ack.socket_index);
					if (dis_ack.socket_index >= WIFI_SOCKET_MIN
							&& dis_ack.socket_index <= WIFI_SOCKET_MAX) {

						index = (dis_ack.socket_index - 1);
						xQueueSend(wifi_socket_write_queue[index],
								(void* )&dis_ack, 0);
					}
					break;
				case UART_CMD_WIFI_POWER_UP:
					PRINTF("WIFI MODULE POWER UP.....\r\n");
					PR_INFO("..wifi module reset..\r\n");
					wifi_connect_state_flag = WIFI_CONNECT_STAT_DIS;
					break;
				case UART_CMD_WIFI_CON_AP:
					PRINTF("WIFI MODULE Connect successed.....\r\n");
					wifi_connect_state_flag = WIFI_CONNECT_STAT_CON;
					PR_INFO("..wifi module connect suc..\r\n");
					break;
				case UART_CMD_WIFI_DIS_AP:
					if( WIFI_CONNECT_STAT_CON == wifi_connect_state_flag )
					{
						wifi_connect_state_flag = WIFI_CONNECT_STAT_DIS;
						PR_INFO("..wifi module disconnect ..\r\n");
						PRINTF("WIFI MODULE Disconnect.....\r\n");
					}
					break;
				case UART_CMD_WIFI_VER_INFO_ACK:
					memcpy(&ver_info, G_U8_Wifi_Parse_Buf, sizeof(ver_info));
					xQueueSend(wifi_ver_info_queue,
												(void* )&ver_info, 0);
					break;

				case UART_CMD_WIFI_RSSI_ACK:
					rssi_level = G_U8_Wifi_Parse_Buf[0];
					xQueueSend(wifi_rssi_queue,(void* )&rssi_level, 0);
					break;

				case UART_CMD_WIFI_STA_INFO_ACK:
					memcpy(&config, G_U8_Wifi_Parse_Buf, sizeof(config));
					xQueueSend(wifi_config_info_queue,
												(void* )&config, 0);
					break;
				default:
					break;
				}
				dataState = UART_WIFI_RECV_HEAD;
				G_U16_UartWifi_RXBuf_Count = 0;
				data_parse_len = 0;
				cmd_type = UART_CMD_IDLE;
				memset(G_U8_UartWifi_RXBuf, 0, sizeof(G_U8_UartWifi_RXBuf));
				memset(G_U8_Wifi_Parse_Buf, 0, sizeof(G_U8_Wifi_Parse_Buf));
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
void wifi_key_check_task(void *pvParameters) {
	uint32_t time = 0; //get_sys_tick();
	uint32_t key_value = 0;
	s32_t ret = 0xf;
	wifi_status_msg_t wifi_status;
	MODEM_SOS_INIT();
	Modem_SOS_Irq_Init();
	EnableIRQ(MODEM_SOS_IRQ);
	
	while (1) {
		if (wifi_key_press_flag == WIFI_KEY_PRESS) {
			wifi_key_press_type = WIFI_KEY_PRESS_LONG;
			time = xTaskGetTickCount();
			while ((xTaskGetTickCount() - time) < WIFI_KEY_PRESS_TIMEOUT) {
				key_value = MODEM_SOS_READ();
				if (WIFI_KEY_NOT_PRESS == key_value) {
					PRINTF("..WIFI_KEY_PRESS_SHORT..\r\n");
					wifi_key_press_type = WIFI_KEY_PRESS_SHORT;
					break;
				} else {
					vTaskDelay(120);
				}
			}

			if (wifi_key_press_type == WIFI_KEY_PRESS_LONG) {
				PRINTF("..WIFI_KEY_PRESS_LONG..\r\n");
				ret = wifi_enter_smart_config(2000);
				if(ret == RET_OK)
				{
					wifi_smart_config_flag = WIFI_SMART_CONFIG;			
					wifi_led_status_type = WIFI_LED_GREEN_RED_TOGGLE;
					GPIO_ClearPinsInterruptFlags(MODEM_SOS_GPIO, 1U << MODEM_SOS_GPIO_PIN);
					DisableIRQ(MODEM_SOS_IRQ);
					PRINTF("..WIFI_SMART_CONFIG..\r\n");
				}
				else
				{
					PRINTF("..WIFI_SMART_CONFIG fail..\r\n");
				}
			} else if (wifi_key_press_type == WIFI_KEY_PRESS_SHORT) {
				//short press
				if (WIFI_PROTECT_MODE == wifi_work_mode)
					wifi_work_mode = WIFI_NORMAL_MODE;
				else
					wifi_work_mode = WIFI_PROTECT_MODE;
			}
			wifi_key_press_flag = WIFI_KEY_NOT_PRESS;
		}
		else {
			key_value = MODEM_SOS_READ();
			if (WIFI_KEY_NOT_PRESS == key_value) {
				wifi_key_press_flag = WIFI_KEY_NOT_PRESS;
			}
			vTaskDelay(WIFI_KEY_CHECK_INTER);
		}
		
		if(wifi_smart_config_flag == WIFI_SMART_CONFIG)
		{
			vTaskDelay(1000);

			get_wifi_staus(UART_CMD_GET_WIFI_STATUS, &wifi_status, 1000);		
			if (WIFI_CON_OK == wifi_status.con_status) 
			{
				wifi_smart_config_flag = NOT_WIFI_SMART_CONFIG;
				if (WIFI_NORMAL_MODE == wifi_work_mode)
				{
					wifi_led_status_type = WIFI_LED_GREEN;
				}
				else if (WIFI_PROTECT_MODE == wifi_work_mode)
				{
					wifi_led_status_type = WIFI_LED_GREEN_TOGGLE;
				}
				
				if (WIFI_KEY_PRESS_LONG == wifi_key_press_type)
				{
					PRINTF("..enable press..\r\n");
					GPIO_ClearPinsInterruptFlags(MODEM_SOS_GPIO, 1U << MODEM_SOS_GPIO_PIN);
					EnableIRQ(MODEM_SOS_IRQ);
				}
			}

		}
		else
		{
			vTaskDelay(1000);
			get_wifi_staus(UART_CMD_GET_WIFI_STATUS, &wifi_status, 1000);
			if (WIFI_CON_OK == wifi_status.con_status) {			
				if (WIFI_NORMAL_MODE == wifi_work_mode)
				{
					wifi_led_status_type = WIFI_LED_GREEN;
				}
				else if (WIFI_PROTECT_MODE == wifi_work_mode)
				{
					wifi_led_status_type = WIFI_LED_GREEN_TOGGLE;
				}
			} 
			else {	
				if((wifi_key_press_flag == WIFI_KEY_NOT_PRESS)
					&&(wifi_smart_config_flag != WIFI_SMART_CONFIG))
				{
					wifi_led_status_type = WIFI_LED_NULL;
				}
			}
			if(aging_test_flag != 0)
			{
				wifi_led_status_type = WIFI_LED_RED_TOGGLE;
			}
		}
	}
}

/*
 *hao.yunran
 *2017.3.22
 *irq for sos call by sos key
 */
void wifi_led_status_task(void *pvParameters) {
	WIFI_RED_LED_INIT();
	WIFI_GREEN_LED_INIT();
	WIFI_RED_LED_OFF();
	WIFI_GREEN_LED_OFF();
	vTaskDelay(500);
	while (1) {
		switch (wifi_led_status_type) {
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
			WIFI_GREEN_LED_ON();
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
			PRINTF("..wifi_led_status_err...\r\n");
			break;

		}
		vTaskDelay(1000);
	}
}

#endif
