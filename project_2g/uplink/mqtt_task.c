#include "lwip/opt.h"

#if LWIP_NETCONN
#include <stdio.h>
#include <string.h>
#include "lwip/netif.h"
#include "lwip/sys.h"
#include "lwip/arch.h"
#include "lwip/api.h"
#include "lwip/tcpip.h"

#ifndef CPU_MK24FN256VDC12
#include "netif/etharp.h"
#include "ethernetif.h"
#endif
#include "board.h"
#include "rtc_fun.h"
#include "aes256.h"
#include "sha1.h"

#include "fsl_device_registers.h"
#include "pin_mux.h"
#include "clock_config.h"
#include "queue.h"
#include "json.h"
#include "spiflash_config.h"
#include "fota.h"
#include "uart_ppp.h"
#include "log_task.h"
#include "MQTTClient.h"
#include "hb_protocol.h"
#include "mqtt_task.h"
#include "log_task.h"

#ifdef MQTT
/*******************************************************************************
 * topic define
 ******************************************************************************/
#define TOPIC_LENGTH						68
#define SUBSCRIBE_DEV_LIST_TOPIC			"%08X%08X%08X/common"
#define	PUBLISH_DEV_LIST_RESULT_TOPIC		"%08X%08X%08X/dynamic"
#define	PUBLISH_INIT_TOPIC					"controller"
char sub_dev_list_topic[TOPIC_LENGTH],pub_dev_list_result_topic[TOPIC_LENGTH];


/*******************************************************************************
 * Variables
 ******************************************************************************/
extern QueueHandle_t mqtt_send_q_handle;
extern HOMEBOX_VERSION homebox_ver;

int my_netif_status;
char sendbuf[7000], readbuf[2000];
char mqttJsonbuff[6000];
char items_buf[2500];
DATA_INFO  get_data_info;
struct netif fsl_netif0;
/*******************************************************************************
 * Function
 ******************************************************************************/
extern void query_by_sensor_list(DATA_INFO *sensor_query_list);
extern int query_get_sensor_data(DATA_ITEM_STRUCT *data_item);
extern void sync_hbb_info();
extern u32_t update_power_info(DATA_INFO *data_info, DATA_ITEM_STRUCT *data_item);
//void netif_init(void);
void MQTT_messageArrived(MessageData* data);
static void getDevList_to_keyValue(MQTT_SEND_Q_MSG *mq, int enc, int sig, char *strPv, AES_STRUCT *aes, JSON_GET_DEV_LIST * get_dev_list);
static void sendDevListResult_to_keyValue(MQTT_SEND_Q_MSG *mq, int enc, int sig, char *strPv, AES_STRUCT *aes,
										  JSON_SEND_DEV_LIST_RESULT * send_dev_list_result);
static int mqttReconnectRetry(void *v, MQTTPacket_connectData *conData);
int MQTTPublishRetry(MQTTClient *c, const char *topicName, MQTTMessage *message, MQTTPacket_connectData *conData);
/*!
 * @brief The main function containing client thread.
 */

void my_netif_init(void)
{
#if 0
    LWIP_PLATFORM_DIAG(("\r\n************************************************"));
    LWIP_PLATFORM_DIAG((" MQTT Client cfg.....\r\n"));
    LWIP_PLATFORM_DIAG(("************************************************"));
#ifdef CPU_MK24FN256VDC12
    while(!ppp_gprs_info.ppp_negotiate_result)
    	vTaskDelay( 2000 / portTICK_RATE_MS );
#else
    ip_addr_t fsl_netif0_ipaddr, fsl_netif0_netmask, fsl_netif0_gw;
    MPU_Type *base = MPU;

    /* Disable MPU. */
    base->CESR &= ~MPU_CESR_VLD_MASK;

    LWIP_DEBUGF(1, ("TCP/IP initializing...\r\n"));
    tcpip_init(NULL, NULL);
    LWIP_DEBUGF(1, ("TCP/IP initialized.\r\n"));
    IP4_ADDR(&fsl_netif0_ipaddr, configIP_ADDR0, configIP_ADDR1, configIP_ADDR2, configIP_ADDR3);
    IP4_ADDR(&fsl_netif0_netmask, configNET_MASK0, configNET_MASK1, configNET_MASK2, configNET_MASK3);
    IP4_ADDR(&fsl_netif0_gw, configGW_ADDR0, configGW_ADDR1, configGW_ADDR2, configGW_ADDR3);

    netif_add(&fsl_netif0, &fsl_netif0_ipaddr, &fsl_netif0_netmask, &fsl_netif0_gw, NULL, ethernetif_init, tcpip_input);
    netif_set_default(&fsl_netif0);

    LWIP_PLATFORM_DIAG((" IPv4 Address     : %u.%u.%u.%u", ((u8_t *)&fsl_netif0_ipaddr)[0],
                        ((u8_t *)&fsl_netif0_ipaddr)[1], ((u8_t *)&fsl_netif0_ipaddr)[2],
                        ((u8_t *)&fsl_netif0_ipaddr)[3]));
    LWIP_PLATFORM_DIAG((" IPv4 Subnet mask : %u.%u.%u.%u", ((u8_t *)&fsl_netif0_netmask)[0],
                        ((u8_t *)&fsl_netif0_netmask)[1], ((u8_t *)&fsl_netif0_netmask)[2],
                        ((u8_t *)&fsl_netif0_netmask)[3]));
    LWIP_PLATFORM_DIAG((" IPv4 Gateway     : %u.%u.%u.%u", ((u8_t *)&fsl_netif0_gw)[0], ((u8_t *)&fsl_netif0_gw)[1],
                        ((u8_t *)&fsl_netif0_gw)[2], ((u8_t *)&fsl_netif0_gw)[3]));

    LWIP_PLATFORM_DIAG(("************************************************"));

#endif
    netif_set_up(&fsl_netif0);
    return;
#endif
    while(!my_netif_status)
    	vTaskDelay(500);
}

void submit_log()
{
	MQTT_SEND_Q_MSG sub_log;
	sub_log.send_cmd = MQTT_UPLOADING_LOG;
	sub_log.uploading_log.info = (void *)&uploading_log;
	xQueueSend(mqtt_send_q_handle, &sub_log, 3000);
}

int mqttReconnect(void *v)
{
	int rc = -1;
	MQTTClient *c = (MQTTClient *)v;
	c->ipstack->disconnect(c->ipstack);
#ifdef CPU_MK24FN256VDC12
#ifndef WIFI_MODULE
	MqttClientTracePrint("ppp check...\r\n");
    if(ppp_check_status() != ERR_OK)
    	goto exit;
#endif
#endif
	MqttClientTracePrint("NetworkConnect...\r\n");
	if((rc = NetworkConnect(c->ipstack, c->addr, c->port, NET_CONNECT_FIRST)) < 0)
	{
		MqttClientTracePrint("mqttReconnect net err\r\n");
		goto exit;
	}
	c->isconnected = 0;

	MQTT_SEND_Q_MSG	sub_msg;
	MqttTaskDbPrint("send sub cmd....\r\n");
	sub_msg.send_cmd = MQTT_SUB_ALL;
	xQueueSend(mqtt_send_q_handle, &sub_msg, 2000);
exit:
	return rc;
}

static int get_item_string(char *out, DATA_ITEM_STRUCT *item, char *info)
{
	char *info_ptr = info;
	if(out == NULL)
		return -1;
	if(info_ptr == NULL)
		info_ptr = item->info;
	sprintf(out,"{\"did\":\"%08X%08X%08X\",\"status\":\"%lu\",\"info\":%s}",item->longid.idh, item->longid.idm,
			item->longid.id, item->status, info_ptr);
	return 0;
}

static int get_items_string(char *out, char *in, int start_flag)
{
	if(start_flag != 0)//if start_flag == 0 it means the first item is coming
		strcat(out,",");
	strcat(out, in);
	return 0;
}

int get_para_info(DATA_INFO *data_info, DATA_ITEM_STRUCT *item, char *outbuf)
{
	char sosPhone[32];
	uint32_t autoAnswer = 0;
	autoAnswer = ppp_gprs_apn_pap_info.incoming_call_answer_timeout;
	if(sscanf(ppp_gprs_apn_pap_info.sos_call, "%[^\r]", sosPhone) != 1)
		return -1;
	sprintf(outbuf,"{\"urlFota\":\"%s\",\"urlEvent\":\"%s\",\"urlAlert\":\"%s\",\"urlCtrl\":\"%s\","
			"\"heartBeatInterval\":\"%lu\",\"fotaInterval\":\"%lu\",\"sosPhone\":\"%s\",\"autoAnswer\":\"%d\","
			"\"aesKey\":\"%s\",\"aesVector\":\"%s\",\"encVer\":\"%s\"}",hbb_info.config.fotaurl, hbb_info.config.eventurl,
			hbb_info.config.alerturl, hbb_info.config.controlurl, hbb_info.config.heartbeat_interval, hbb_info.config.fota_check_interval,
			sosPhone, autoAnswer, hbb_info.config.aeskey, hbb_info.config.aesivv, hbb_info.config.key_version);

	item->longid.idh = hbb_info.config.longidh;
	item->longid.idm = hbb_info.config.longidm;
	item->longid.id = hbb_info.config.id;

	{
		uint32_t sensor_count = 0;
		for(sensor_count=0; sensor_count<MAX_SENSOR_COUNT; sensor_count++)
		{
			if (sensor_status[sensor_count].send_message.sn_dest != 0)
			{
				PR_INFO("..id:%x..rssi:%d\r\n",sensor_status[sensor_count].send_message.sn_dest,sensor_status[sensor_count].send_message.rssi);
			}
		}
	}
	return 0;
}


static int query_data_items(MQTT_SEND_Q_MSG *msg)
{
	int rc = -1;
	DATA_ITEM_STRUCT data_item;
	DATA_INFO *data_info = (DATA_INFO *)msg->send_data.data_info;
	char temp_buf[500];
	switch(data_info->gd.qt)
	{
		case 1://data
		{
			int items_flag = 0;
			MqttTaskDbPrint("get data of device.\r\n");
			query_by_sensor_list(data_info);
			memset(items_buf, 0, sizeof(items_buf));
			for(int i=0;i<data_info->gd.num;i++)//it's about sensor data
			{
				memcpy(&data_item.longid.idh, &data_info->gd.dtl[i].idh,sizeof(LONG_ID96));
				if((data_item.longid.idh ==  hbb_info.config.longidh)&&
				   (data_item.longid.idm ==  hbb_info.config.longidm)&&
				   (data_item.longid.id ==  hbb_info.config.id))//get homebox data
				{
#ifdef CPU_MK24FN256VDC12
					update_power_info(data_info, &data_item);
#endif
				}
				else
				{
					//xxxx( &data_item);
					if (query_get_sensor_data(&data_item))
					{
						MqttTaskTracePrint("Query sensor fail %08x\r\n", data_item.longid.id);
						continue;
					}
				}
				memset(temp_buf, 0, sizeof(temp_buf));
				get_item_string(temp_buf, &data_item, NULL);
				get_items_string(items_buf, temp_buf, items_flag);
				items_flag++;
			}
			break;
		}
		case 2://parameter
		{
			MqttTaskDbPrint("get data of parameter.\r\n");
			get_para_info(data_info, &data_item, temp_buf);
			get_item_string(items_buf, &data_item, temp_buf);
			break;
		}
		default:
			MqttTaskTracePrint("Query type is undefine in GET DATA!\r\n");
	}
	MqttTaskDbPrint("got it...\r\n");
	msg->send_data.items = items_buf;
	return rc;
}

void MQTT_messageArrived(MessageData* data)
{
	JSON_RECV_DEV_LIST recv_dev_list;

	MqttTaskTracePrint("\r\n--------Recieve from Topic<%.*s>------\r\n",
			data->topicName->lenstring.len, data->topicName->lenstring.data);
	MqttTaskDbPrint("Message arrived on topic %.*s: %.*s\r\n", data->topicName->lenstring.len, data->topicName->lenstring.data,
		data->message->payloadlen, data->message->payload);
	if(json_deserialize_msg_recv(data->message->payload,
			data->message->payloadlen,&recv_dev_list) == JSON_PARSE_ERR)
	{
		MqttTaskTracePrint("MQTT_messageArrived json parse error...\r\n");
		return;
	}
	switch(recv_dev_list.phead.func)
	{
		case FUN_DEV_LIST:
		{
			MQTT_SEND_Q_MSG	result_msg;
			uninstall_sensors(&recv_dev_list.recv_dev_list_body.udl.dl);
			if(recv_dev_list.recv_dev_list_body.udl.dl.num <= MAX_SENSOR_COUNT)
			{
				for(int i=0;i<recv_dev_list.recv_dev_list_body.udl.dl.num;i++)
					install_sensor(&recv_dev_list.recv_dev_list_body.udl.dl.idn[i].lid, recv_dev_list.recv_dev_list_body.udl.dl.idn[i].dt);
				result_msg.send_dev_list_result.rc  = 200;
				sync_hbb_info();
			}
			else
				result_msg.send_dev_list_result.rc  = 300;//sensor list is more than MAX_SENSOR_COUNT

			set_rtc_date(recv_dev_list.phead.sts_s);
			result_msg.send_cmd = MQTT_SEND_DEV_LIST_RESULT;
			result_msg.send_dev_list_result.func = 0;
			strcpy(result_msg.send_dev_list_result.msgid,recv_dev_list.recv_dev_list_body.msgid);
			xQueueSend(mqtt_send_q_handle, &result_msg, 0);
			break;

		}
		case FUN_GET_LOG:
		{
			strcpy(uploading_log.msgid, recv_dev_list.recv_dev_list_body.msgid);
			uploading_log.st = recv_dev_list.recv_dev_list_body.ghl.st;
			uploading_log.et = recv_dev_list.recv_dev_list_body.ghl.et;
			uplink_log();
			//send q
			break;
		}
		case FUN_GET_DATA:
		{
			MQTT_SEND_Q_MSG send_data;
			strcpy(get_data_info.msgid, recv_dev_list.recv_dev_list_body.msgid);
			memcpy(&get_data_info.gd, &recv_dev_list.recv_dev_list_body.gd, sizeof(GET_DATA));
			if(get_data_info.gd.qdt == 1)//it means all device
			{
				//copy all sensor id to gd,expect homebox id
				int dtl_num = 0;
				for (int i=0; i<MAX_SENSOR_COUNT; i++)
				{
					if (sensor_status[i].send_message.en)
					{
						get_data_info.gd.dtl[dtl_num].idh = sensor_status[i].longidh;
						get_data_info.gd.dtl[dtl_num].idm = sensor_status[i].longidm;
						get_data_info.gd.dtl[dtl_num].id = sensor_status[i].send_message.sn_dest;
						dtl_num++;
					}
				}
				get_data_info.gd.dtl[dtl_num].idh = hbb_info.config.longidh;
				get_data_info.gd.dtl[dtl_num].idm = hbb_info.config.longidm;
				get_data_info.gd.dtl[dtl_num].id = hbb_info.config.id;
				get_data_info.gd.num = dtl_num + 1;
			}
			send_data.send_cmd = MQTT_SEND_DATA;
			send_data.send_data.rc = 200;
			send_data.send_data.data_info = (void *)&get_data_info;
 			xQueueSend(mqtt_send_q_handle, (MQTT_SEND_Q_MSG *)&send_data, 0);
			break;
		}
		case FUN_EMERGENCY_FOTA:
		{
			FOTA_RQ_MSG msg;
			msg.cmd = FW_TIMING_CHECK_FOTA;
			xQueueSend(fota_rq_handle, &msg, 2000);
			break;
		}
		case FUN_TIME_STAMP:
		{
			break;
		}
		default:
			MqttTaskTracePrint("func is undefine in (MQTT_messageArrived)");
	}


}

static void getDevList_to_keyValue(MQTT_SEND_Q_MSG *mq, int enc, int sig, char *strPv, AES_STRUCT *aes, JSON_GET_DEV_LIST * get_dev_list)
{
	uint64_t timestamp;
	get_dev_list->phead.enc = enc;
	get_dev_list->phead.func = mq->get_dev_list.func;
	strcpy(get_dev_list->phead.pv, strPv);
	get_current_systime(&timestamp);
	get_dev_list->phead.sts_s = (uint32_t)(timestamp / 1000);// get current timestamp
	get_dev_list->phead.sts_ms = (uint32_t)(timestamp % 1000);// get current timestamp
	get_dev_list->phead.sig = sig;
	get_dev_list->phead.aes = aes;
	get_dev_list->get_dev_list_body.did = hbb_info.config.id;
}

static void sendDevListResult_to_keyValue(MQTT_SEND_Q_MSG *mq, int enc, int sig, char *strPv, AES_STRUCT *aes,
										  JSON_SEND_DEV_LIST_RESULT * send_dev_list_result)
{
	uint64_t timestamp;
	send_dev_list_result->phead.enc = enc;
	send_dev_list_result->phead.func = mq->send_dev_list_result.func;
	strcpy(send_dev_list_result->phead.pv, strPv);
	get_current_systime(&timestamp);
	send_dev_list_result->phead.sts_s = (uint32_t)(timestamp / 1000);// get current timestamp
	send_dev_list_result->phead.sts_ms = (uint32_t)(timestamp % 1000);// get current timestamp
	send_dev_list_result->phead.aes = aes;
	send_dev_list_result->phead.sig = sig;
	send_dev_list_result->send_dev_list_result_body.msgid = mq->send_dev_list_result.msgid;
	send_dev_list_result->send_dev_list_result_body.rc = mq->send_dev_list_result.rc;
}

static void sendEvent_to_keyValue(MQTT_SEND_Q_MSG *mq, int enc, int sig, char *strPv, AES_STRUCT *aes, JSON_SEND_EVENT *send_event )
{
	uint64_t timestamp;
	send_event->phead.enc = enc;
	send_event->phead.sig = sig;
	send_event->phead.func = 0;
	send_event->phead.aes = aes;
	strcpy(send_event->phead.pv, strPv);
	get_current_systime(&timestamp);
	send_event->phead.sts_s = (uint32_t)(timestamp / 1000);// get current timestamp
	send_event->phead.sts_ms = (uint32_t)(timestamp % 1000);// get current timestamp

	send_event->send_event_body.slid = &mq->send_event.slid;
	send_event->send_event_body.dt = mq->send_event.device_type;
	send_event->send_event_body.t = mq->send_event.status;
	send_event->send_event_body.p = mq->send_event.p;
	send_event->send_event_body.ts_s = (uint32_t)(mq->send_event.ts / 1000);
	send_event->send_event_body.ts_ms = (uint32_t)(mq->send_event.ts % 1000);
	send_event->send_event_body.data = mq->send_event.data;
	//send_event->send_event_body.hwv = mq->send_event.hwv;
	get_version_string(mq->send_event.fwv, send_event->send_event_body.fwv, sizeof(send_event->send_event_body.fwv));
	ipaddr_ntoa_r(&netif_default->ip_addr, send_event->send_event_body.src, sizeof(send_event->send_event_body.src));
	get_guid((unsigned char *)mq,sizeof(MQTT_SEND_Q_MSG), (unsigned char *)send_event->send_event_body.msgid);
}

static void uploading_log_keyValue(MQTT_SEND_Q_MSG *mq, int enc, int sig, char *strPv, AES_STRUCT *aes, JSON_UPLOADING_LOG *log)
{
	uint64_t timestamp;
	LOG_UPLOADING_STRUCT *loading = (LOG_UPLOADING_STRUCT *)mq->uploading_log.info;
	log->phead.enc = enc;
	log->phead.sig = sig;
	log->phead.func = FUN_GET_LOG;
	strcpy(log->phead.pv, strPv);
	get_current_systime(&timestamp);
	log->phead.sts_s = (uint32_t)(timestamp / 1000);// get current timestamp
	log->phead.sts_ms = (uint32_t)(timestamp % 1000);// get current timestamp
	log->phead.aes = aes;
	log->uploading_log_body.btchct = loading->batch_count;
	log->uploading_log_body.btchid = loading->batch_id;
	log->uploading_log_body.log_content = loading->log_ptr;
	log->uploading_log_body.log_len = loading->log_len;
	log->uploading_log_body.msgid = loading->msgid;
}

static void send_data_keyValue(MQTT_SEND_Q_MSG *mq, int enc, int sig, char *strPv, AES_STRUCT *aes, JSON_SEND_DATA *data)
{
	uint64_t timestamp;
	DATA_INFO *data_info = (DATA_INFO *)mq->send_data.data_info;
	data->phead.aes = aes;
	data->phead.enc = enc;
	data->phead.sig = sig;
	data->phead.func = FUN_GET_DATA;
	strcpy(data->phead.pv, strPv);
	get_current_systime(&timestamp);
	data->phead.sts_s = (uint32_t)(timestamp / 1000);// get current timestamp
	data->phead.sts_ms = (uint32_t)(timestamp % 1000);// get current timestamp

	data->send_data_body.qt = data_info->gd.qt;
	data->send_data_body.rc = mq->send_data.rc;
	data->send_data_body.msgid = data_info->msgid;
	data->send_data_body.items = mq->send_data.items;
}

void mqtt_task(void *pvParameters)
{
	MQTTClient client;
	Network network;
	MQTT_SEND_Q_MSG mqttSendMsg;
	AES_STRUCT mqtt_aes;
	int rc = 0;
	char *pub_topic;

	char clientID[40];
	MQTTMessage message;
	MQTTPacket_connectData connectData = MQTTPacket_connectData_initializer;
	connectData.MQTTVersion = 3;
	connectData.clientID.cstring = clientID;
	mqtt_aes.ivv = hbb_info.config.aesivv;
	mqtt_aes.key = hbb_info.config.aeskey;
	mqtt_aes.version = hbb_info.config.key_version;

	pvParameters = 0;
	sprintf(connectData.clientID.cstring, "%08X%08X%08Xclient", hbb_info.config.longidh, hbb_info.config.longidm, hbb_info.config.id);
	sprintf(sub_dev_list_topic, SUBSCRIBE_DEV_LIST_TOPIC, hbb_info.config.longidh, hbb_info.config.longidm, hbb_info.config.id);
	sprintf(pub_dev_list_result_topic, PUBLISH_DEV_LIST_RESULT_TOPIC, hbb_info.config.longidh, hbb_info.config.longidm, hbb_info.config.id);

	my_netif_init();
	NetworkInit(&network);
	MQTTClientInit(&client, &network, hbb_info.config.mqtt_address, hbb_info.config.mqtt_port, &connectData, 40000,
					(unsigned char *)sendbuf, sizeof(sendbuf), (unsigned char *)readbuf, sizeof(readbuf));
	client.reconnect = mqttReconnect;//reconnect callback function
	if ((rc = NetworkConnect(&network, client.addr, client.port, NET_CONNECT_FIRST)) != 0)
		MqttTaskTracePrint("Return code from network connect is %d\n", rc);
	else
		MqttTaskDbPrint("net Connected\n");

	if ((rc = MQTTConnect(&client, &connectData, FLAG_LOCK)) != 0)
		MqttTaskTracePrint("Return code from MQTT connect is %d\n", rc);
	else
		MqttTaskDbPrint("MQTT Connected\r\n");

	if ((rc = MQTTSubscribe(&client, sub_dev_list_topic, 2, MQTT_messageArrived)) != 0)
	{
		MqttTaskTracePrint("Return code from MQTT subscribe is %d\n", rc);
		// if subscribe fail we need to disable keep alive flag to retry.
		client.ping_outstanding = 1;
	}
#if defined(MQTT_TASK)
	if ((rc = MQTTStartTask(&client)) != pdPASS)
		MqttTaskTracePrint("Return code from start tasks is %d\n", rc);
#endif
/*******************e.g.**********
	MQTT_SEND_Q_MSG test;
	test.send_cmd = MQTT_UPLOADING_LOG;
	uploading_log.batch_count = 5;
	uploading_log.batch_id = 1;
	uploading_log.log_len = strlen(sdf);
	uploading_log.log_ptr = sdf;
	test.uploading_log.info = (void *)&uploading_log;
	xQueueSend(mqtt_send_q_handle, &test, 3000);
	*/
	while(1)
	{
		xQueueReceive(mqtt_send_q_handle, &mqttSendMsg, portMAX_DELAY);
		switch(mqttSendMsg.send_cmd)
		{
			case MQTT_SEND_DEV_LIST_RESULT:
			{
				JSON_SEND_DEV_LIST_RESULT sendDevListResult;
				sendDevListResult_to_keyValue(&mqttSendMsg, AES256, NOT_SIG, homebox_ver.prot_ver, &mqtt_aes, &sendDevListResult);
				json_serialize_dev_list_result(&sendDevListResult, mqttJsonbuff, sizeof(mqttJsonbuff));
				message.qos = 1;
				message.retained = 0;
				pub_topic =	pub_dev_list_result_topic;
				break;
			}
			case MQTT_GET_DEV_LIST:
			{
				JSON_GET_DEV_LIST getDevList;
				getDevList_to_keyValue(&mqttSendMsg, AES256, NOT_SIG, homebox_ver.prot_ver, &mqtt_aes, &getDevList);
				json_serialize_get_dev_list(&getDevList, mqttJsonbuff, sizeof(mqttJsonbuff));
				message.qos = 1;
				message.retained = 0;
				pub_topic =	PUBLISH_INIT_TOPIC;
				break;
			}
			case MQTT_SUB_ALL:
			{
				MqttClientTracePrint("MQTTConnect...\r\n");
				if ((rc = MQTTConnect(&client, &connectData, FLAG_LOCK)) != 0)
				{
					MqttTaskTracePrint("Return code from MQTT connect is %d\n", rc);
					continue;
				}
				MqttTaskTracePrint("sub all...\r\n");

				if ((rc = MQTTSubscribe(&client, sub_dev_list_topic, 2, MQTT_messageArrived)) != 0)
				{
					MqttTaskTracePrint("Return code from MQTT subscribe is %d\n", rc);
					continue;
				}
				defer_log_uplink();
				client.ping_outstanding = 0;
				continue;
			}
			case UPLINK_SEND_EVENT:
			{
				JSON_SEND_EVENT send_event;
				MqttClientTracePrint("MQTT_SEND_EVNET...\r\n");
				sendEvent_to_keyValue(&mqttSendMsg, AES256, NOT_SIG, homebox_ver.prot_ver, &mqtt_aes, &send_event);
				json_serialize_send_event(&send_event, mqttJsonbuff, sizeof(mqttJsonbuff));
				message.qos = 1;
				message.retained = 0;
				if((mqttSendMsg.send_event.status == ALERT) || (mqttSendMsg.send_event.status == UNALERT))
					pub_topic =	"alert/home";
				else
					pub_topic =	"event/home";
				break;
			}
			case MQTT_UPLOADING_LOG:
			{
				JSON_UPLOADING_LOG log;
				uploading_log_keyValue(&mqttSendMsg, AES256, NOT_SIG, homebox_ver.prot_ver, &mqtt_aes, &log);
				json_serialize_uploading_log(&log, mqttJsonbuff, sizeof(mqttJsonbuff));
				message.qos = 1;
				message.retained = 0;
				pub_topic = pub_dev_list_result_topic;
				break;
			}
			case MQTT_SEND_DATA:
			{
				JSON_SEND_DATA send_data;
				mqttSendMsg.send_data.data_info = (void *)&get_data_info;
				query_data_items(&mqttSendMsg);
				send_data_keyValue(&mqttSendMsg, AES256, NOT_SIG, homebox_ver.prot_ver, &mqtt_aes, &send_data);
				json_serialize_send_data(&send_data, mqttJsonbuff, sizeof(mqttJsonbuff));
				message.qos = 1;
				message.retained = 0;
				pub_topic = pub_dev_list_result_topic;
				break;
			}
			default:
			{
				MqttTaskTracePrint("mqtt send cmd is undefine :%d\r\n",mqttSendMsg.send_cmd);
				continue;
			}
		}
		message.payload = mqttJsonbuff;
		message.payloadlen = strlen(mqttJsonbuff);
		MQTTPublishRetry(&client, pub_topic, &message, &connectData);
//		if ((rc = MQTTPublish(&client, pub_topic, &message)) != 0)
//		{
//			MqttTaskTracePrint("Publish fail,retry...\n");
//		}
	}
	/* do not return */
}

int MQTTPublishRetry(MQTTClient *c, const char *topicName, MQTTMessage *message, MQTTPacket_connectData *conData)
{
	int rc;
	for(int i=0;i<2;i++)
	{
		if ((rc = MQTTPublish(c, topicName, message)) != 0)
		{
			MqttTaskTracePrint("Publish fail,retry...\n");
//			if(mqttReconnectRetry(c, conData) != 0)
//			{
//				MqttTaskTracePrint("mqttReconnectRetry error...\r\n");
//				return rc;
//			}
			continue;
		}
		break;
	}

	return rc;
}

static int mqttReconnectRetry(void *v, MQTTPacket_connectData *conData)
{
	int rc = -1;
	MQTTClient *c = (MQTTClient *)v;
	c->ipstack->disconnect(c->ipstack);

#ifdef CPU_MK24FN256VDC12
#ifndef WIFI_MODULE
	MqttClientTracePrint("ppp check...\r\n");
    if(ppp_check_status() != ERR_OK)
    	goto exit;
#endif
#endif
	MqttClientTracePrint("NetworkConnect...\r\n");
	if((rc = NetworkConnect(c->ipstack, c->addr, c->port, NET_CONNECT_FIRST)) < 0)
	{
		MqttClientTracePrint("mqttReconnect net err\r\n");
		goto exit;
	}
	c->isconnected = 0;
	MqttClientTracePrint("MQTTConnect...\r\n");
	if ((rc = MQTTConnect(c, conData, FLAG_LOCK)) != 0)
	{
		MqttTaskTracePrint("Return code from MQTT connect is %d\n", rc);
		goto exit;
	}
	MqttTaskTracePrint("sub all...\r\n");

	if ((rc = MQTTSubscribe(c, sub_dev_list_topic, 2, MQTT_messageArrived)) != 0)
	{
		MqttTaskTracePrint("Return code from MQTT subscribe is %d\n", rc);
		goto exit;
	}
	c->ping_outstanding = 0;

exit:
	return rc;
}


#endif
#endif /* LWIP_NETCONN */
