/*
 * mqtt_task.h
 *
 *  Created on: 2016Äê11ÔÂ18ÈÕ
 *      Author: yiqiu.yang
 */

#ifndef UPLINK_MQTT_TASK_H_
#define UPLINK_MQTT_TASK_H_
#include "uplink_protocol.h"

#define MQTT_ENABLE		    1//1:Turn on  0:Turn off
#if MQTT_ENABLE
#define MQTT
#endif

#define MQTT_TASK_DEBUG 	1//1:Turn on  0:Turn off
#if MQTT_TASK_DEBUG
#define MqttTaskDbPrint			printf
#else
#define MqttTaskDbPrint
#endif

#define MQTT_TASK_TRACING 	1//1:Turn on  0:Turn off
#if MQTT_TASK_TRACING
#define MqttTaskTracePrint		printf
#else
#define MqttTaskTracePrint
#endif

#define MQTT_SEND_Q_LENGTH		7

typedef struct
{
	LONG_ID96 longid;
	uint32_t status;
	char info[50];
}DATA_ITEM_STRUCT;

typedef struct
{
	int func;
}MQTT_GET_DEV_LIST_STRUCT;

typedef struct
{
	int func;
	int rc;
	char msgid[33];
}MQTT_SEND_DEV_LIST_RESULT_STRUCT;

typedef UPLINK_EVENT_STRUCT MQTT_SEND_EVENT_STRUCT; //UPLINK_EVENT_STRUCT is define in uplink_protocol.h

typedef struct
{
	void *info;
}MQTT_UPLOADING_LOG_STRUCT;

typedef struct
{
	void *data_info;
	char *items;
	int rc;
}MQTT_SEND_DATA_STRUCT;

typedef struct
{
	enum UPLINK_CMD send_cmd;
	union
	{
		MQTT_GET_DEV_LIST_STRUCT get_dev_list;
		MQTT_SEND_DEV_LIST_RESULT_STRUCT send_dev_list_result;
		MQTT_SEND_EVENT_STRUCT send_event;
		MQTT_UPLOADING_LOG_STRUCT uploading_log;
		MQTT_SEND_DATA_STRUCT send_data;
	};

}MQTT_SEND_Q_MSG;

void submit_log(void);

#endif /* UPLINK_MQTT_TASK_H_ */
