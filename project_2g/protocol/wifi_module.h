#ifndef  __WIFI_MODULE_H__
#define __WIFI_MODULE_H__


#include "MQTTFreeRTOS.h"
#include "queue.h"

#define UART_RX_BUF_SIZE      1024*6
#define UART_TX_BUF_SIZE      1024
#define UART_HTTP_BUF_SIZE    1024*6

#define  WIFI_KEY_PRESS    		0
#define  WIFI_KEY_NOT_PRESS    	1
#define  WIFI_KEY_CHECK_INTER	300
#define  WIFI_KEY_PRESS_TIMEOUT	7000

#define  WIFI_KEY_PRESS_NULL   	0
#define  WIFI_KEY_PRESS_LONG   	1
#define  WIFI_KEY_PRESS_SHORT  	2

#define  NOT_WIFI_SMART_CONFIG  0
#define  WIFI_SMART_CONFIG 		1

#define WIFI_NORMAL_MODE   0
#define WIFI_PROTECT_MODE  1


#define  WIFI_LED_NULL			 	0
#define  WIFI_LED_GREEN 			1
#define  WIFI_LED_RED 				2
#define  WIFI_LED_GREEN_RED 		3
#define  WIFI_LED_GREEN_TOGGLE 		4
#define  WIFI_LED_RED_TOGGLE 		5
#define  WIFI_LED_GREEN_RED_TOGGLE 	6

#define  WIFI_CON_OK  0X05


//conn server state
enum{
	CONN_OK   =        0,
    CONN_FAIL
};
//uart_recv_state
typedef enum {
	UART_WIFI_RECV_HEAD = 0,
	UART_WIFI_RECV_BODY,
	UART_WIFI_RECV_SUC,
	UART_WIFI_RECV_BUTT
}UART_WIFI_RECV_STATE;

//uart recv data state
enum {
	UART_RECV_DATA_SUCCESS = 0,
	UART_RECV_DATA_ERROR,
	UARTRECV_DATA_CONTINUE
};
//cmd type
typedef enum  {

	UART_CMD_IDLE=0,
	UART_REQ_CON = 0xA001,
	UART_REQ_CON_ACK=0xA002,
	UART_CON_SERVER = 0XA101,
	UART_CON_SERVER_ACK=0XA102,
	UART_SEND_DATA = 0XA301,
	UART_SEND_DATA_ACK1=0XA302,
	UART_SEND_DATA_ACK2=0XA303,
	UART_CLOSE_SERVER_CON = 0XA201,
	UART_CLOSE_SERVER_CON_ACK=0XA202,

	//mqtt cmd
	UART_MQTT_REQ_CON_ACK=0xA003,
	UART_MQTT_CON_SERVER = 0XA401,
	UART_MQTT_CON_SERVER_ACK = 0XA402,
	UART_MQTT_SEND_DATA = 0XA501,
	UART_MQTT_SEND_DATA_ACK1=0XA502,
	UART_MQTT_SEND_DATA_ACK2=0XA503,
	UART_MQTT_CLOSE_SERVER_CON = 0XA601,
	UART_MQTT_CLOSE_SERVER_CON_ACK=0XA602,

	//get wifi status
	UART_CMD_GET_WIFI_STATUS = 0XCCDD,
	UART_CMD_GET_WIFI_STATUS_ACK = 0XCCEE,
	UART_CMD_SMARTCONFIG = 0XCCAA,
	UART_CMD_SMARTCONFIG_ACK = 0XCCBB

}UART_CMD_TYPE;

typedef struct
{
	uint16_t send_cmd;
	uint16_t recv_cmd;
}req_ack_type;

//wifi netconn state
enum {
	NETCONN_STATUS_OK = 0,
	NETCONN_STATUS_CLOSE
};
//RET CODE
enum {
	RET_OK = 0,
	RET_FAIL
};


typedef struct
{
	uint16_t cmd;
	uint8_t ret_code;
}wifi_cmd_msg_t;


typedef struct
{
	uint32_t addr;

}wifi_ip_addr_t;

typedef struct
{
	wifi_ip_addr_t  ip;
	wifi_ip_addr_t  gw;
	wifi_ip_addr_t  nw;
}wifi_ip_info_t;


typedef struct
{
	uint8_t  wifi_mode;
	uint8_t  con_status;
	wifi_ip_info_t ip_info;
	uint8_t mac_addr[6];
}wifi_status_msg_t;


//http
uint8_t  wifi_check_netconn_status(uint32_t timeout_ms);
uint8_t  wifi_connect_to_server(char *url,uint32_t timeout_ms);
uint8_t  wifi_netconn_write(const void * pdata,uint16_t datalen,uint16_t cmdtype,uint32_t timeout_ms);
uint8_t  wifi_netconn_recv(void **recvbuf,uint16_t *len,uint32_t timeout_ms);
uint8_t  wifi_netconn_close(uint32_t timeout_ms);
static uint8_t wifi_http_clear_buf();


//mqtt
int  wifi_mqtt_connect_to_server(char *url, uint16_t port,req_ack_type *req_ack,uint32_t timeout_ms);
int  wifi_mqtt_netconn_read(Network* n, unsigned char* buffer, int len, int timeout_ms);
int  wifi_mqtt_netconn_write(Network* n, unsigned char* buffer, int len, int timeout_ms);
int  wifi_mqtt_netconn_close(Network* n);


//uart
static uint8_t uart_wifi_sendData(void *pdata,uint16_t datalen,uint16_t cmdtype);
static uint8_t uart_wifi_revdata(uint8_t data);
static uint8_t wifi_waitfor_ACK(uint16_t cmdtype,uint32_t timeout_ms);
static uint8_t wifi_mqtt_waitfor_ACK(uint16_t cmdtype,QueueHandle_t  queue_type,uint32_t timeout_ms);
static uint16_t crc_calculate(uint8_t *pdata, uint16_t dataLength);
static uint8_t parse_uart_wifi_data_packet(const uint8_t *pdata,uint8_t *userData,uint16_t *index,uint16_t *data_len);

//get wifi status
int get_wifi_staus(uint16_t cmdtype,wifi_status_msg_t *wifi_staus,uint32_t timeout_ms);
int wifi_enter_smart_config(uint32_t timeout_ms);




/*--------------------------------------------------------------------------*/
/*
 *                            MACRO DEFINITIONS
 */

typedef unsigned long int uint32;
#define QUEUE_MERMERY_SIZE 1024*2

/* Initialize the character queue */
#define _CHARQ_INIT(cq,max_size) \
   (cq)->MAX_SIZE = max_size; \
   (cq)->CURRENT_SIZE = 0; \
   (cq)->HEAD         = 0; \
   (cq)->TAIL         = 0;

/*
 * Remove a character to the TAIL of the queue
 * (Normal writing location for the queue)
 */
#define _CHARQ_ENQUEUE(cq,c) \
   if ( (cq)->CURRENT_SIZE < (cq)->MAX_SIZE ){ \
      (cq)->QUEUE[(cq)->TAIL++] = (char)(c); \
      if ( (cq)->TAIL == (cq)->MAX_SIZE ) { \
         (cq)->TAIL = 0; \
      } /* Endif */ \
      ++(cq)->CURRENT_SIZE; \
   } /* Endif */


/*
 * Add a character from the HEAD of the queue
 * (Reading location)
 */
#define _CHARQ_ENQUEUE_HEAD(cq,c) \
   if ( (cq)->CURRENT_SIZE < (cq)->MAX_SIZE ) { \
      if ((cq)->HEAD == 0) { \
        (cq)->HEAD = (cq)->MAX_SIZE; \
      } /* Endif */ \
      --(cq)->HEAD; \
     (cq)->QUEUE[(cq)->HEAD] = c; \
     (cq)->CURRENT_SIZE++; \
   } /* Endif */


/*
 * Remove a character from the HEAD of the queue
 * (Normal reading location for the queue)
 */
#define _CHARQ_DEQUEUE(cq,c) \
   if ( (cq)->CURRENT_SIZE ) { \
      --(cq)->CURRENT_SIZE; \
      c = (cq)->QUEUE[((cq)->HEAD++)]; \
      if ( (cq)->HEAD == (cq)->MAX_SIZE ) { \
         (cq)->HEAD = 0; \
      } /* Endif */ \
   } /* Endif */


/*
 * Remove a character from the TAIL of the queue
 * (Writing location)
 */
#define _CHARQ_DEQUEUE_TAIL(cq,c) \
   if ( (cq)->CURRENT_SIZE ) { \
      --(cq)->CURRENT_SIZE; \
      if ( (cq)->TAIL == 0 ) { \
         (cq)->TAIL = (cq)->MAX_SIZE; \
      } /* Endif */ \
      c = (cq)->QUEUE[--(cq)->TAIL]; \
   } /* Endif */


/* Return the current size of the queue */
#define _CHARQ_SIZE(cq) ((cq)->CURRENT_SIZE)


/* Return whether the queue is empty */
#define _CHARQ_EMPTY(cq) ((cq)->CURRENT_SIZE == 0)


/* Return whether the queue is full */
#define _CHARQ_FULL(q) ((q)->CURRENT_SIZE >= (q)->MAX_SIZE)


/* Return whether the queue is NOT full */
#define _CHARQ_NOT_FULL(q) ((q)->CURRENT_SIZE < (q)->MAX_SIZE)



/*--------------------------------------------------------------------------*/
/*
 *                            DATATYPE DECLARATIONS
 */

/*---------------------------------------------------------------------
 *
 * CHARQ STRUCTURE
 */

/*!
 * \brief This structure used to store a circular character queue.
 *
 * The structure must be the LAST if it is included into another data structure,
 * as the character queue falls off of the end of this structure.
 */
typedef struct charq_struct
{

   /*!
    * \brief The maximum number of characters for the queue, as specified in
    * initialization of the queue.
    */
   uint32  MAX_SIZE;

   /*! \brief The current number of characters in the queue. */
   volatile uint32  CURRENT_SIZE;

   /*! \brief Index of the first character in queue. */
   uint32  HEAD;

   /*! \brief Index of the last character in queue. */
   uint32  TAIL;

   /*! \brief The character queue itself. */
   char      QUEUE[4];

} CHARQ_STRUCT, * CHARQ_STRUCT_PTR;


#endif
