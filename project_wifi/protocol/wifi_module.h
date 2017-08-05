#ifndef  __WIFI_MODULE_H__
#define __WIFI_MODULE_H__


#include "MQTTFreeRTOS.h"
#include "queue.h"
#include "ip_addr.h"
#include "netbuf.h"
#include "err.h"
#include "netif.h"

#define UART_RX_BUF_SIZE      1024*6

#define  WIFI_KEY_PRESS    		0
#define  WIFI_KEY_NOT_PRESS    	1
#define  WIFI_KEY_CHECK_INTER	300
#define  WIFI_KEY_PRESS_TIMEOUT	7000

#define  WIFI_KEY_PRESS_NULL   	0
#define  WIFI_KEY_PRESS_LONG   	1
#define  WIFI_KEY_PRESS_SHORT  	2

#define  NOT_WIFI_SMART_CONFIG  0
#define  WIFI_SMART_CONFIG 		1

#define  WIFI_NORMAL_MODE   0
#define  WIFI_PROTECT_MODE  1

#define WIFI_UART_CURRENT_TIMEOUT   2160


#define  WIFI_LED_NULL			 	0
#define  WIFI_LED_GREEN 			1
#define  WIFI_LED_RED 				2
#define  WIFI_LED_GREEN_RED 		3
#define  WIFI_LED_GREEN_TOGGLE 		4
#define  WIFI_LED_RED_TOGGLE 		5
#define  WIFI_LED_GREEN_RED_TOGGLE 	6

#define  WIFI_CON_OK  0X05

#define WIFI_HTTP_CMD   1
#define WIFI_MQTT_CMD   2

#define WIFI_CONNECT_STAT_CON      1
#define WIFI_CONNECT_STAT_DIS      0



#define HEAD0  0X7E
#define HEAD1  0X7E
#define HEAD2  0X7E
#define HEAD3  0X7E

#define END0  0X7D
#define END1  0X7D
#define END2  0X7D
#define END3  0X7D



#define MQTT_SOCKET_TCP                  6
#define MQTT_SOCKET_UDP                  17
#define HTTP_SOCKET_TCP                 0
#define HTTP_SOCKET_UDP                 1
#define WIFI_SOCKET_NONE                0xff

#define RET_SOCKET_OK                   0
#define RET_SOCKET_FAIL                -6

#define ACK_OK   0
#define ACK_FAIL 1

#define WIFI_SOCKET_MAX                4
#define WIFI_SOCKET_MIN                1

#define DNS_SOCKET_MAX                2
#define DNS_SOCKET_MIN                0

#define WIFI_SOCKET_NOT_USED            0
#define WIFI_SOCKET_USED                1



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

	UART_CMD_IDLE=0xFFF,
	UART_REQ_CON = 0xA001,
	UART_REQ_CON_ACK=0xA002,

	//get wifi status
	UART_CMD_GET_WIFI_STATUS = 0XCCDD,
	UART_CMD_GET_WIFI_STATUS_ACK = 0XCCEE,
	UART_CMD_SMARTCONFIG = 0XCCAA,
	UART_CMD_SMARTCONFIG_ACK = 0XCCBB,


	UART_CMD_PARSE_URL_IP  =0XCC00,
	UART_CMD_PARSE_URL_IP_ACK  =0XCC01,
	UART_CMD_SOCKET_DISCON =0XCC10,
	UART_CMD_SOCKET_DISCON_ACK =0XCC11,
	UART_CMD_SOCKET_CON =0XCC20,
	UART_CMD_SOCKET_CON_ACK =0XCC21,
	UART_CMD_SOCKET_WRITE =0XCC30,
	UART_CMD_SOCKET_WRITE_ACK =0XCC31,
	UART_CMD_SOCKET_READ_ACK =0XCC40,

	//wifi scan
	UART_CMD_iwlist_scanning =0XDD40,
	UART_CMD_iwlist_scanning_ACK =0XDD41,

	UART_CMD_WIFI_POWER_UP =0XEE00,
	UART_CMD_WIFI_CON_AP   =0XEE11,
	UART_CMD_WIFI_DIS_AP =0XEE22,

	UART_CMD_WIFI_VER_INFO = 0xFF00,
	UART_CMD_WIFI_VER_INFO_ACK = 0xFF01,

	UART_CMD_WIFI_RSSI = 0xFF02,
	UART_CMD_WIFI_RSSI_ACK = 0xFF03,

	UART_CMD_WIFI_STA_INFO = 0xFF04,
	UART_CMD_WIFI_STA_INFO_ACK = 0xFF05


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


typedef struct wifi_socket_type_t
{
    u32_t	socket_index;
    u32_t	socket_index_flag;
    u32_t	socket_proto_type;
}WIFI_SOCKET_TYPE;

typedef struct wifi_socket_t
{
	WIFI_SOCKET_TYPE      wifi_socket_cache[WIFI_SOCKET_MAX];
}WIFI_SOCKET;

typedef struct parse_url_type
{
	 uint8_t ack_code;
	 uint8_t socket_index;
	 ip_addr_t addr;
}PARSE_URL_ACK_T;


typedef struct parse_url_buf_type
{
	 uint8_t socket_index;
	 char url[128];
}PARSE_URL_SEND_T;

typedef struct wifi_socket_close_t
{
	uint8_t ack_code;
	uint8_t socket_index;
}WIFI_DISCON_T;

typedef struct wifi_socket_write_t
{
	uint8_t socket_index;
	uint8_t *send_buf;
}WIFI_WRITE_T;



typedef struct wifi_socket_con_t
{
	uint8_t ack_code;
	uint8_t socket_index;
	struct sockaddr addr;
}WIFI_CON_T;


struct station_config {
	uint8_t ssid[32];
	uint8_t password[64];
	uint8_t bssid_set;	// Note: If bssid_set is 1, station will just connect to the router
                        // with both ssid[] and bssid[] matched. Please check about this.
	uint8_t bssid[6];
};

typedef struct
{
	uint32_t swver;
	char sw_info[92];
} wifi_ver_info;


extern uint32_t wifi_uart_current_status;

#ifdef PRODUCE_TEST
extern signed int uart_wifi_sendData(void *pdata, uint16_t datalen, uint16_t cmdtype);
#else
static s32_t uart_wifi_sendData(void *pdata,uint16_t datalen,uint16_t cmdtype);
#endif
static s32_t uart_wifi_revdata(uint8_t data);
static s32_t parse_uart_wifi_data_packet(const uint8_t *pdata,uint16_t len,uint8_t *userData);
static u16_t wifi_htons(u16_t n);

s32_t wifi_get_rssi(uint32_t timeout_ms);
s32_t wifi_get_ver_info(wifi_ver_info *ver_info,uint32_t timeout_ms);
s32_t wifi_get_config_info(struct station_config *config,uint32_t timeout_ms);


s32_t wifi_check_netconn_status(uint32_t timeout_ms);
s32_t get_wifi_staus(uint16_t cmdtype,wifi_status_msg_t *wifi_staus,uint32_t timeout_ms);
s32_t wifi_enter_smart_config(uint32_t timeout_ms);
s32_t wifi_gethostbyname(const char *name, ip_addr_t *addr);
s32_t wifi_socket(int domain, int type, int protocol);
s32_t wifi_closesocket(int s);
s32_t wifi_connect(int s, const struct sockaddr *name, socklen_t namelen);
s32_t wifi_write(int socket_n, unsigned char* buffer, int len, int timeout_ms);
s32_t wifi_read(int socket_n, unsigned char* buffer, int len, int timeout_ms);


/*--------------------------------------------------------------------------*/
/*
 *                            MACRO DEFINITIONS
 */

typedef unsigned long int uint32;
#define QUEUE_MERMERY_SIZE 1024*6
#define QUEUE_HEAD_SIZE   16

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
