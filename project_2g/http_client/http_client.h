/*
 * http_client.h
 *
 *  Created on: 2016Äê8ÔÂ26ÈÕ
 *      Author: yiqiu.yang
 */

/*HTTP Client header file*/


#ifndef HTTP_CLIENT_H
#define HTTP_CLIENT_H

#include "lwip/netif.h"
#include "lwip/api.h"
#include "fsl_rtc.h"
#include <stdint.h>
#include <stdbool.h>


#ifdef WIFI_MODULE
#include "wifi_module.h"
#endif

#if 0
#define WEB_TEST
#endif

#if 1
//Enable debug
#define DBG    DbgConsole_Printf
#define WARN   DbgConsole_Printf
#define ERR    DbgConsole_Printf

#else
//Disable debug
#define DBG(x, ...)
#define WARN(x, ...)
#define ERR(x, ...)

#endif

#define SEND_BUF_SIZE  (SEND_DATA_SIZE+POST_REQ_HEAD_FORMAT_SIZE)
#define SEND_DATA_SIZE 4500 //send data size can't more then 1000 byte
#define POST_REQ_HEAD_FORMAT_SIZE 500
#define HTTP_DATE rtc_datetime_t

#ifndef WIFI_MODULE
#define ERR_HANDLE()	 	netbuf_delete(inbuf);\
	netconn_close(conn);\
	netconn_delete(conn);
#endif
enum HTTP_CONN_ANALYSE
{
	HTTP_CONN_RETRY,
	HTTP_CONN_URL,
	HTTP_CONN_RETURN_FAIL,
};

enum HTTP_POST
{
	HTTP_SUCC,
	HTTP_FAIL
};

enum HTTP_CONNECT
{
	HTTP_CL = -24,				 /* Can't match content len  */
	HTTP_PARSE = -23,              /* Url Parse error          */
	HTTP_DNS = -22,                /* Could not resolve name   */
	HTTP_NEW = -21,                /* Netconn_new error        */
	HTTP_RC = -20,                 /* Can't match http response code*/
	HTTP_CONTENT = -19,            /* Can't find http content  */
	HTTP_LEN = -18,				 /* The http content length is more then recieve buf*/
	HTTP_LEN_UNEQUAL = -17,
#ifdef CPU_MK24FN256VDC12
	HTTP_PPP = -16,
#endif

	HTTP_IF = -15,			 /* Low-level netif error.   */
	HTTP_ATG = -14,			 /* Illegal argument.        */
	HTTP_CONN = -13,		 /* Not connected.           */
	HTTP_CLSD = -12,		 /* Connection closed.       */
	HTTP_RST = -11,			 /* Connection reset.        */
	HTTP_ABRT = -10,		 /* Connection aborted.      */
	HTTP_ISCONN = -9,		 /* Already connected.       */
	HTTP_USE = -8,			 /* Address in use.          */
	HTTP_WOULDBLOCK = -7,	 /* Operation would block.   */
	HTTP_VAL = -6,			 /* Illegal value.           */
	HTTP_INPROGRESS = -5,	 /* Operation in progress.   */
	HTTP_RTE = -4,			 /* Routing problem.         */
	HTTP_TIMEOUT = -3,		 /* Timeout.                 */
	HTTP_BUF = -2,			 /* Buffer error.            */
	HTTP_MEM = -1,           /* Out of memory error.     */

	HTTP_CONN_OK = 0,             /* Success                  */

};

enum HTTP_METH
{
	HTTP_GET,
	HTTP_POST,
	HTTP_PUT,
	HTTP_DELETE,
	HTTP_HEAD
};

enum HTTP_PURPOSE
{
	SEND_EVENT,
	CHECK_FW_UPDATE,
	DOWNLOAD_IMAGE_BLKS,
	RESP_UPDATE_RESULT,
	GET_SUB_DEV,
};
typedef struct
{
	char *url_ptr;
	uint8_t method;
	uint8_t purpose;
	uint32_t result_code;
	char* send_buf_ptr;
	uint16_t send_buf_length;
	char* recv_buf_ptr;
	uint16_t recv_buf_length;
	uint16_t timeout;
}HTTP_CLIENT_POST_STRUCT;

extern char send_buf[SEND_BUF_SIZE];

int8_t month_string_to_num(char *str);
bool http_match_date_second(const char * date_str, uint32_t *second);
int8_t HTTPClient_connect( HTTP_CLIENT_POST_STRUCT * http_struct);

int8_t HTTPClient_post( HTTP_CLIENT_POST_STRUCT * http_struct);

int8_t HTTPClient_parseURL( char* url, char* scheme, uint8_t maxSchemeLen,
									        char* host, uint8_t maxHostLen, uint16_t* port,
									        char* path, uint8_t maxPathLen); //Parse URL
//int8_t HTTPClient_get( char* url, int8_t purpose, char* data_send, int timeout );

#endif

