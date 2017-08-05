#ifndef _JSON_H_
#define _JSON_H_
#include "string.h"
#include "stdint-gcc.h"
#include "stdio.h"
#include "aes256.h"
#include "uplink_protocol.h"
#include "json_string.h"
#define JSON_DEBUG 			1//1:Turn on  0:Turn off
#if JSON_DEBUG
#define JsDbPrint			printf
#else
#define JsDbPrint
#endif

#define JS_TRACING 			1//1:Turn on  0:Turn off
#if JS_TRACING
#define JsTracePrint		printf
#else
#define JsTracePrint
#endif

#define FUNC		 "\"func\":\""
#define DL     		 "\"dl\":\""
#define TS			 "\"ts\":\""
#define ST			 "\"st\":\""
#define ET			 "\"et\":\""
#define STS			 "\"sts\":\""
//#define CPY			 "\"cpy\":\""
#define QDT			 "\"qdt\":\""
#define DTL			 "\"dtl\":\""
#define QT			 "\"qt\":\""
#define QC			 "\"qc\":\""
enum FUN_TYPE
{
	FUN_DEV_LIST = 1,
	FUN_GET_LOG = 2,
	FUN_GET_DATA = 3,
	FUN_EMERGENCY_FOTA = 4,
	FUN_TIME_STAMP = 5,
};

/***********************************************************
 * mqtt control data structure
 *
 *
 */

typedef struct
{
	char pv[16];
	int enc;
	int sig;
	int func;
	uint32_t sts_s;//2016/06/05/03:04 \0
	uint32_t sts_ms;
	AES_STRUCT *aes;
}PROTO_HEADER;


typedef struct
{
	DEVICE_LIST dl;
	//uint32_t cpy;
	uint32_t  ts;
}UPDATE_DEV_LIST;

typedef struct
{
	uint64_t st;
	uint64_t et;
}GET_HBB_LOG;

typedef struct
{
	uint32_t ts;
}UPDATE_TIME;

typedef struct
{
	unsigned int qdt;
	LONG_ID96 dtl[MAX_SENSOR_COUNT];
	unsigned int num;
	unsigned int qt;
	unsigned int qc;
}GET_DATA;

typedef struct
{
	char msgid[33];
	GET_DATA gd;
}DATA_INFO;

typedef struct
{
	char msgid[33];
	union
	{
		UPDATE_DEV_LIST udl;
		GET_HBB_LOG ghl;
		UPDATE_TIME ut;
		GET_DATA gd;
	};
}RECV_DEV_LIST_BODY;

typedef struct
{
	char * msgid;
	int	rc;
}SEND_DEV_LIST_RESULT_BODY;

typedef struct
{
	uint32_t did;
}JSON_GET_DEV_LIST_BODY;

typedef struct
{
	PROTO_HEADER phead;
	JSON_GET_DEV_LIST_BODY get_dev_list_body;
}JSON_GET_DEV_LIST;

typedef struct
{
	PROTO_HEADER phead;
	SEND_DEV_LIST_RESULT_BODY send_dev_list_result_body;
}JSON_SEND_DEV_LIST_RESULT;

typedef struct
{
	PROTO_HEADER phead;
	RECV_DEV_LIST_BODY recv_dev_list_body;
}JSON_RECV_DEV_LIST;

typedef EVENT_REQ_BODY SEND_EVENT_BODY;  //EVENT_REQ_BODY is define in uplink_protocol.h

typedef struct
{
	PROTO_HEADER phead;
	SEND_EVENT_BODY send_event_body;
}JSON_SEND_EVENT;

typedef struct
{
	char *msgid;
	unsigned char *log_content;
	unsigned int log_len;
	unsigned int btchct;
	unsigned int btchid;
}UPLOADING_LOG_BODY;

typedef struct
{
	PROTO_HEADER phead;
	UPLOADING_LOG_BODY uploading_log_body;
}JSON_UPLOADING_LOG;

typedef struct
{
	char *msgid;
	int rc;
	unsigned int qt;
	char *items;
}SEND_DATA_BODY;

typedef struct
{
	PROTO_HEADER phead;
	SEND_DATA_BODY send_data_body;
}JSON_SEND_DATA;

void json_serialize_get_dev_list(JSON_GET_DEV_LIST *js, char *jsonBuff, int buffLen);

void json_serialize_dev_list_result(JSON_SEND_DEV_LIST_RESULT *js, char *jsonBuff, int buffLen);

void json_serialize_send_event(JSON_SEND_EVENT *js, char *jsonBuff, int buffLen);

void json_serialize_uploading_log(JSON_UPLOADING_LOG *js, char *jsonBuff, int buffLen);

void json_serialize_send_data(JSON_SEND_DATA *js, char *jsonBuff, int buffLen);

int json_deserialize_msg_recv(char *jsonBuff, int buffLen, JSON_RECV_DEV_LIST *js);



#endif
