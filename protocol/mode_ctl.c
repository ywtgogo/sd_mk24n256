/*
 * mode_ctl.c
 *
 *  Created on: 2015/7/18
 *      Author: zhanghong
 */
#include <string.h>
#include <stdint.h>
#include "fsl_common.h"
#include "fsl_debug_console.h"

/* Kernel includes. */
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "timers.h"
#include "semphr.h"
#include "hb_protocol.h"
#include "message.h"
#include "uplink_protocol.h"
#include "protocol_2_17.h"
#include "aes256.h"
#include "sha1.h"
#include "lwip/err.h"
#include "lwip/sys.h"
#include "rtc_fun.h"
#include "version.h"
#include "uart_ppp.h"
#include "hbb_config.h"
#include "version.h"
#include "uplink_protocol.h"
#include "log_task.h"
#include "aes256.h"
#include "supervisor.h"

#ifdef PLC
#include "uart_plc.h"
#include "plc_if.h"
#endif
#include "mqtt_task.h"
#ifdef MQTT
#include "json.h"
extern QueueHandle_t mqtt_send_q_handle;
#endif
extern uint32_t __etext[];

__attribute__((section(".default_config")))
#ifdef BRIDGE
HBB_STATUS_STORAGE_BLOCK hbb_info = {
		.config = {
				BRIDGE_CONFIG
			},
};
#else
HBB_STATUS_STORAGE_BLOCK hbb_info = {
		.hb_tick = 0,
		.msg_sent = 0,
		.uplink_sent = 0,
		.status = 0,
		.pversion =  0x03000001,//"3.0.0.1"
		.nt = NET_TYPE,
		.man = "SANDLUCAS",
		.os = "ld",
		.config = {
				HBB_DEFCONFIG
		},
};
#endif


uint8_t rf_aeskey[16] = { 0x2b, 0x7e, 0x15, 0x16, 0x28, 0xae, 0xd2, 0xa6, 0xab, 0xf7, 0x15, 0x88, 0x09, 0xcf, 0x4f, 0x3c };
AES_KEY dec_key;
AES_KEY enc_key;

uint32_t sensor_count = 0;

uint8_t flag = 0 ;

SemaphoreHandle_t rf_message_sem;
SemaphoreHandle_t hb_task_sem;

void sync_sensor_list()
{
	int sensor_cnt = 0;
	volatile SENSOR_STATUS_STORAGE_BLOCK *sensorptr;
	volatile plc_bridge_node *bridgeptr;

	for(int i=0; i<MAX_SENSOR_COUNT; i++)
	{
	 if(sensor_status[i].send_message.en == 1)
		 sensor_cnt++;
	}

	if (sensor_cnt == 0)
		return;

	taskENTER_CRITICAL();
	FLASH_RTOS_Erase_Sector(&pflash_handle, HBB_SENSOR_LIST_CONFIG, 1);
	FLASH_RTOS_Program(&pflash_handle, HBB_SENSOR_LIST_CONFIG, (uint32_t *)sensor_status, (sizeof(sensor_status) + 7) & ~7);
#ifdef PLC
	FLASH_RTOS_Program(&pflash_handle, HBB_SENSOR_LIST_CONFIG + 1024 * 3, (uint32_t *)bridge_list, (sizeof(&bridge_list) + 7) & ~7);
#endif
	taskEXIT_CRITICAL();

	sensorptr = (volatile SENSOR_STATUS_STORAGE_BLOCK *)HBB_SENSOR_LIST_CONFIG;

	PRINTF("dump flash sensor list\r\n");
	for(int i=0; i<MAX_SENSOR_COUNT; i++)
	{
	 if(sensorptr[i].send_message.en  == 1)
		 PRINTF("sensor %x \r\n", sensorptr[i].send_message.sn_dest);
	}
#ifdef PLC
	bridgeptr = (volatile plc_bridge_node *)(HBB_SENSOR_LIST_CONFIG + 1024 * 3);
	for(int i=0; i<MAX_BRIDGE_COUNT; i++)
	{
		 PRINTF("sensor %x \r\n", bridgeptr[i].id);
	}
#endif
}

void get_sensor_list()
{
	   LONG_ID96 lid;

       SENSOR_STATUS_STORAGE_BLOCK *flash_sensor_status = (SENSOR_STATUS_STORAGE_BLOCK *)HBB_SENSOR_LIST_CONFIG;
       plc_bridge_node *bridgeptr = (plc_bridge_node *)(HBB_SENSOR_LIST_CONFIG + 1024 * 3);

       if (READ32((uint32_t*)(HBB_SENSOR_LIST_CONFIG)) == 0xffffffff){
               return;
       }

       PRINTF("get flash sensor list\r\n");
       PRINTF("sensor list+++++++++++++++++++++\r\n");
       for(int i=0; i<MAX_SENSOR_COUNT; i++)
       {
    	   if ((flash_sensor_status[i].send_message.sn_dest == 0)
                  || (flash_sensor_status[i].send_message.sn_dest == 0xFFFFFFFF))
    		   break;
    	   lid.id = flash_sensor_status[i].send_message.sn_dest;
    	   lid.idh = flash_sensor_status[i].longidh;
    	   lid.idm = flash_sensor_status[i].longidm;

           install_sensor(&lid, flash_sensor_status[i].device_type);
       }

#ifdef PLC
       for(int i=0; i<MAX_BRIDGE_COUNT; i++)
       {
    	   if ((bridgeptr[i].id == 0)
                  || (bridgeptr[i].id == 0xFFFFFFFF))
    		   break;
    	   lid.id = bridgeptr[i].id;
    	   lid.idh = bridgeptr[i].longidh;
    	   lid.idm = bridgeptr[i].longidm;

           install_sensor(&lid, bridgeptr[i].dt);
       }
#endif
}

void sync_hbb_info()
{
	taskENTER_CRITICAL();
	FLASH_RTOS_Erase_Sector(&pflash_handle, HBB_INFO_CONFIG, 1);
	PRINTF("hbb config %x %d\r\n", &hbb_info.config, (sizeof(hbb_config) + 7) & ~7);
	FLASH_RTOS_Program(&pflash_handle, HBB_INFO_CONFIG, (uint32_t *)&hbb_info.config, ((sizeof(hbb_config) + 7) & ~7));
	FLASH_RTOS_Program(&pflash_handle, HBB_INFO_CONFIG + 2048, (uint32_t *)&ppp_gprs_apn_pap_info, ((sizeof(ppp_gprs_apn_pap_info) + 7) & ~7));
	taskEXIT_CRITICAL();
}

void get_hbb_info()
{
	volatile hbb_config *config;

	config = (volatile hbb_config *)HBB_INFO_CONFIG;

	if (strncmp(config->alerturl, "http", 4))
		return;
	memset(&hbb_info.config, 0, sizeof(hbb_config));
	memcpy((void *)&hbb_info.config, (void *)HBB_INFO_CONFIG,  sizeof(hbb_config));
	memcpy((void *)&ppp_gprs_apn_pap_info, (void *)(HBB_INFO_CONFIG + 2048), sizeof(ppp_gprs_apn_pap_info));
#ifndef PRODUCE_TEST
	PRINTF("load hb info from flash\r\n");
	PRINTF("id  %x\r\n", hbb_info.config.id);
	PRINTF("heartbeat_interval   %d\r\n", hbb_info.config.heartbeat_interval);
	PRINTF("fota_check_interval   %d\r\n", hbb_info.config.fota_check_interval);
	PRINTF("alerturl  %s\r\n", hbb_info.config.alerturl);
	PRINTF("controlurl  %s\r\n", hbb_info.config.controlurl);
	PRINTF("eventurl  %s\r\n", hbb_info.config.eventurl);
	PRINTF("fotaurl  %s\r\n", hbb_info.config.fotaurl);
//	PRINTF("mqtt_address:  %s\r\n", hbb_info.config.mqtt_address);
//	PRINTF("mqtt_port:  %d\r\n", hbb_info.config.mqtt_port);
#endif
}

void get_default_hbbconfig(hbb_config *outconfig)
{
	hbb_config *pconfig;
	uint32_t address;

	address = (uint32_t)__etext + offsetof(HBB_STATUS_STORAGE_BLOCK, config);
	pconfig = (hbb_config *)address;

	PRINTF("get_old_hbbconfig\r\n");
	PRINTF("id  %x\r\n", pconfig->id);
	PRINTF("master  %x\r\n", pconfig->master);
	PRINTF("heartbeat_interval  %d\r\n", pconfig->heartbeat_interval);
	PRINTF("fota_check_interval  %d\r\n", pconfig->fota_check_interval);
	PRINTF("eventurl  %s\r\n", pconfig->eventurl);

	memcpy(outconfig, pconfig, sizeof(pconfig));
}

void sync_gprs_info()
{
	taskENTER_CRITICAL();
	FLASH_RTOS_Erase_Sector(&pflash_handle, HBB_INFO_CONFIG, 1);
	FLASH_RTOS_Program(&pflash_handle, HBB_INFO_CONFIG, (uint32_t *)&hbb_info.config, ((sizeof(hbb_config) + 7) & ~7));
	FLASH_RTOS_Program(&pflash_handle, HBB_INFO_CONFIG + 2048, (uint32_t *)&ppp_gprs_apn_pap_info, ((sizeof(ppp_gprs_apn_pap_info) + 7) & ~7));
	taskEXIT_CRITICAL();
}

void get_gprs_info()
{
	memcpy((void *)&ppp_gprs_apn_pap_info, (void *)(HBB_INFO_CONFIG + 2048),  sizeof(ppp_gprs_apn_pap_info));
}

int check_sensor_last_tick(SENSOR_STATUS_STORAGE_BLOCK *sensor, uint32_t ms)
{
	TickType_t		now, elapse;

	if (sensor->last_ack_tick == 0)
		return 0;
	now = xTaskGetTickCount();
	elapse = (now - sensor->last_ack_tick) * portTICK_PERIOD_MS;

	if (elapse <= ms)
		return 1;
	else
		return 0;
}

int check_sensor_last_alarm(SENSOR_STATUS_STORAGE_BLOCK *sensor, uint32_t ms)
{
	TickType_t		now, elapse;
	if (sensor->last_alarm_tick == 0)
		return 0;
	now = xTaskGetTickCount();
	elapse = (now - sensor->last_alarm_tick) * portTICK_PERIOD_MS;

	if (elapse <= ms)
		return 1;
	else
		return 0;
}

int query_get_sensor_data(DATA_ITEM_STRUCT *data_item)
{

	uint32_t sid = data_item->longid.id;
	int idx = sensor_get_index_id(sid);

	if (idx == -1)
	{
		PRINTF("invalid sensor id\r\n");
		return -1;
	}

	if (sensor_status[idx].send_message.en && (sensor_status[idx].send_message.sn_mitt == hbb_info.config.id))
	{
		if (check_sensor_last_tick(&sensor_status[idx], 30000))
		{
			if (sensor_status[idx].send_message.alarm_status)
				data_item->status = ALERT;
			else
				data_item->status = INFO;

			if(data_item->info)
				sprintf(data_item->info, "\"10401/vt:%u\"", (unsigned int)GET_SENSOR_VBATT(&sensor_status[idx]));

			PRINTF("%s get sensor status %d data %s\r\n", __func__, data_item->status, data_item->info);

			return 0;
		}

		PRINTF("%s get sensor status fail\r\n", __func__);
	}

	return -1;
}

#ifdef MQTT
void query_by_sensor_list(DATA_INFO *sensor_query_list)
{
	TickType_t		xLastWakeTime;
	RF_MSG rf_msg_query;
	err_t err = 0;
	int i;
	uint32_t sid;
	int idx;
	int listnum = sensor_query_list->gd.num;

	rf_msg_query.cmd = RF_CMD_GENERIC_REQUEST;
	for (int j=0; j<3; j++)
	{

		xLastWakeTime = xTaskGetTickCount();

		for (i=0; i<listnum; i++)
		{
			sid = sensor_query_list->gd.dtl[i].id;
			idx = sensor_get_index_id(sid);
			if (idx == -1)
			{
				PRINTF("invalid sensor id\r\n");
				continue;
			}
			if (sensor_status[idx].send_message.en && (sensor_status[idx].send_message.sn_mitt == hbb_info.config.id))
			{
				if (!check_sensor_last_tick(&sensor_status[idx], 30000))
				{
					PRINTF("query sensor %x\r\n", sensor_status[idx].send_message.sn_dest);
					err = hb_do_rf_send_callback(sensor_status[idx].send_message.sn_dest, &rf_msg_query);
				}
			}
		}
		if (err)
			vTaskDelayUntil( &xLastWakeTime, 1000);
	}

	for (i=0; i<listnum; i++)
	{
		sid = sensor_query_list->gd.dtl[i].id;
		idx = sensor_get_index_id(sid);
		PRINTF("sid %x tick %d\r\n", sid, sensor_status[idx].last_ack_tick);
	}
	PRINTF("current tick %d\r\n", xTaskGetTickCount());
}

void query_sensor_all(void)
{
	TickType_t		xLastWakeTime;
	RF_MSG rf_msg_query;
	err_t err = 0;
	int i;
	uint32_t sid;

	rf_msg_query.cmd = RF_CMD_GENERIC_REQUEST;
	for (int j=0; j<3; j++)
	{
		xLastWakeTime = xTaskGetTickCount();

		for (int i=0; i<MAX_SENSOR_COUNT; i++)
		{
			if (sensor_status[i].send_message.en && (sensor_status[i].send_message.sn_mitt == hbb_info.config.id))
			{
				if (!check_sensor_last_tick(&sensor_status[i], 30000))
				{
					err = hb_do_rf_send_callback(sensor_status[i].send_message.sn_dest, &rf_msg_query);
				}

			}
		}
		if (err)
			vTaskDelayUntil( &xLastWakeTime, 10000 );

	}

	for (i=0; i<32; i++)
	{
		if(!sensor_status[i].send_message.en)
			continue;
		sid = sensor_status[i].send_message.sn_dest;
		PRINTF("sid %x tick %d\r\n", sid, sensor_status[i].last_ack_tick);
		if (check_sensor_last_tick(&sensor_status[i], 30000))
			PRINTF("updated\r\n");
	}
	PRINTF("current tick %d\r\n", xTaskGetTickCount());
}
#endif
void check_sensor_status(void)
{
	TickType_t		xLastWakeTime;
	RF_MSG rf_msg_query;
	err_t err = 0;

	rf_msg_query.cmd = RF_CMD_GENERIC_REQUEST;
	for (int j=0; j<6; j++)
	{

		xLastWakeTime = xTaskGetTickCount();

		for (int i=0; i<MAX_SENSOR_COUNT; i++)
		{
			if (sensor_status[i].send_message.en && (sensor_status[i].send_message.sn_mitt == hbb_info.config.id))
			{
				sensor_status[i].send_message.query_acked = 0;
				if (DEVICE_TYPE_Module(sensor_status[i].device_type) == 3)
					continue;
				if (!check_sensor_last_tick(&sensor_status[i], 60000))
				{
					err = hb_do_rf_send_callback(sensor_status[i].send_message.sn_dest, &rf_msg_query);
				}
				else
				{
					sensor_status[i].send_message.query_acked = 1;
				}
			}
		}
		if (err)
			vTaskDelayUntil( &xLastWakeTime, 10000 );
	}

}

void fota_check_sensor_status(uint32_t device_type)
{
	RF_MSG rf_msg_query;
	//err_t err = 0;

	rf_msg_query.cmd = RF_CMD_GENERIC_REQUEST;
	for (int i=0; i<MAX_SENSOR_COUNT; i++)
	{
		if (device_type != sensor_status[i].device_type) continue;
		for (int j=0; j<3; j++)
		{
			if (sensor_status[i].send_message.en && (sensor_status[i].send_message.sn_mitt == hbb_info.config.id))
			{
				sensor_status[i].send_message.query_acked = 0;
				if (hb_do_rf_send_callback(sensor_status[i].send_message.sn_dest, &rf_msg_query) != 0 ){
					vTaskDelay(1000);
				}else {
					sensor_status[i].send_message.query_acked = 1;
					break;
				}
			}
		}
	}
}

int update_plc_route = 0;

void hb_task(void *pvParameters )
{
	int sendlist_retry = 0;
	int query_plc_retry = 0;
	LONG_ID96 lid;

	for (;;)
	{
    	xSemaphoreTake(hb_task_sem, portMAX_DELAY);
    	check_sensor_status();
#ifdef PLC
    	if (update_plc_route == 0)
    	{
    		int res;

    		++plc_local_seq;
retry_sendlist:
    		sent_sensor_list_to_bridge(bridge_list[0].id,plc_local_seq);
    		res = xSemaphoreTake(plc_slist_back_sem, 10000);
    		if(pdPASS != res)
    		{
    			PRINTF("PLC ERR : wait sensor list time out!\r\n");
    			if(sendlist_retry < 5)
    			{
    	    		sendlist_retry++;
    				goto retry_sendlist;
    			}
    		}
    		else
    		{
    			deploy_sensor_list_to_bridge(bridge_list[0].id,++plc_local_seq);
    			update_plc_route = 1;
    			lid.id = bridge_list[0].id;
    			lid.idh = bridge_list[0].longidh;
    			lid.idm = bridge_list[0].longidm;

    			send_uplink_event_core(&lid, bridge_list[0].dt, INFO, bridge_list[0].sw, bridge_list[0].hw, 1, NULL);
    			hbb_info.uplink_sent++;
    		}
    	}
    	else
    	{
    		int res;

    		++plc_local_seq;
retry_query_plc:
			PRINTF("query bridge\r\n");
        	query_bridge_sensor(bridge_list[0].id,plc_local_seq);
        	res = xSemaphoreTake(plc_sensor_query_back_sem, 10000);
			if(pdPASS != res)
			{
				PRINTF("PLC ERR : wait sensor list time out!\r\n");
				if(query_plc_retry < 5)
				{
					query_plc_retry++;
					goto retry_query_plc;
				}

			}
			else
			{
				lid.id = bridge_list[0].id;
				lid.idh = bridge_list[0].longidh;
				lid.idm = bridge_list[0].longidm;

				send_uplink_event_core(&lid, bridge_list[0].dt, INFO, bridge_list[0].sw, bridge_list[0].hw, 1, NULL);
			    hbb_info.uplink_sent++;
			}
    	}
#endif
    	hbb_send_uplink_event(hbb_info.config.id, INFO);

		 for(int i=0; i<MAX_SENSOR_COUNT; i++)
		 {
			 if(sensor_status[i].send_message.en  && (sensor_status[i].send_message.sn_mitt == hbb_info.config.id)
					  && check_sensor_last_tick(&sensor_status[i], hbb_info.config.heartbeat_interval))
			 {
				 send_uplink_event(sensor_status[i].send_message.sn_dest, INFO);
			 }
			 else
			 {
				 if (sensor_status[i].send_message.query_acked == 1)
					 send_uplink_event(sensor_status[i].send_message.sn_dest, INFO);
			 }
		}

	}
}

void hbCallback( TimerHandle_t xTimer )
{
	PRINTF("hb timer\r\n");
	xSemaphoreGive(hb_task_sem);
}

int update_hb_heartbeat(uint32_t newval)
{
	if ((newval < 60000) && (newval > SENSOR_POLLING_INTERVAL))
		return -1;
	if( xSemaphoreTake( hbb_info.xSemaphore, 1000) == pdTRUE)
	{
		hbb_info.config.heartbeat_interval = newval;
		xSemaphoreGive(hbb_info.xSemaphore);
		sync_hbb_info();
		xTimerStop(hbb_info.timer, 0);
		if( xTimerChangePeriod( hbb_info.timer, hbb_info.config.heartbeat_interval, 0) != pdPASS )
		{
			PRINTF("change alarm timer failed\r\n");
			return -1;
		}
		if (xTimerStart(hbb_info.timer, 0) != pdPASS)
		{
			PRINTF("start hb timer failed\r\n");
			return -1;
		}
	} else {
		PRINTF("%s new %d failed\r\n", __func__, newval);
		return -1;
	}

	PRINTF("%s new %d\r\n", __func__, newval);

	return 0;
}

int update_hbb_id(LONG_ID96 *longid)
{
	if( xSemaphoreTake( hbb_info.xSemaphore, 1000) == pdTRUE)
	{
		hbb_info.config.longidh = longid->idh;
		hbb_info.config.longidm = longid->idm;
		hbb_info.config.id = longid->id;
		sync_hbb_info();
		xSemaphoreGive(hbb_info.xSemaphore);
		for(int i=0; i<MAX_SENSOR_COUNT; i++)
			sensor_status[i].send_message.sn_mitt = hbb_info.config.id;
	} else {
		PRINTF("%s id %d failed\r\n", __func__, hbb_info.config.id);
		return -1;
	}

	PRINTF("%s id %d\r\n", __func__, hbb_info.config.id);

	return 0;
}

int update_hb_msg_audio_switch(uint32_t swtich_value)
{

	if( xSemaphoreTake( hbb_info.xSemaphore, 1000) == pdTRUE)
	{
		hbb_info.config.msg_audio_switch = swtich_value;
		sync_hbb_info();
		xSemaphoreGive(hbb_info.xSemaphore);

	} else {
		PRINTF("%s id %d failed\r\n", __func__, hbb_info.config.msg_audio_switch);
		return -1;
	}

	PRINTF("%s id %d\r\n", __func__,hbb_info.config.msg_audio_switch);

	return 0;
}

uint32_t get_hb_msg_audio_switch()
{
	return hbb_info.config.msg_audio_switch;
}



int update_hbb_idd(LONG_ID96 *longid)
{


	return 0;
}

int update_hbb_aeskey(unsigned char *newivv, unsigned char *newkey, char *keyver)
{
	if( xSemaphoreTake( hbb_info.xSemaphore, 1000) == pdTRUE)
	{
		memset(hbb_info.config.aesivv, 0, 20);
		memcpy(hbb_info.config.aesivv, newivv, 20);
		memset(hbb_info.config.aeskey, 0, 40);
		memcpy(hbb_info.config.aeskey, newkey, 40);
		memset(hbb_info.config.key_version, 0, 4);
		strncpy(hbb_info.config.key_version, keyver, 4);
		xSemaphoreGive(hbb_info.xSemaphore);
		sync_hbb_info();
	} else {
		PRINTF("%s failed\r\n", __func__);
		return -1;
	}

	PRINTF("%s success\r\n", __func__);

	return 0;
}

int update_hbb_alerturl(char *url, uint32_t len)
{
	if( xSemaphoreTake( hbb_info.xSemaphore, 1000) == pdTRUE)
	{
		memset(hbb_info.config.alerturl, 0, 128);
		strncpy(hbb_info.config.alerturl, url, len);
		xSemaphoreGive(hbb_info.xSemaphore);
		sync_hbb_info();
	} else {
		PRINTF("%s failed\r\n", __func__);
		return -1;
	}

	PRINTF("%s success\r\n", __func__);

	return 0;
}

int update_hbb_eventurl(char *url, uint32_t len)
{
	if( xSemaphoreTake( hbb_info.xSemaphore, 1000) == pdTRUE)
	{
		memset(hbb_info.config.eventurl, 0, 128);
		strncpy(hbb_info.config.eventurl, url, len);
		xSemaphoreGive(hbb_info.xSemaphore);
		PRINTF("new event url %s\r\n", hbb_info.config.eventurl);
		sync_hbb_info();
	} else {
		PRINTF("%s failed\r\n", __func__);
		return -1;
	}

	PRINTF("%s success\r\n", __func__);

	return 0;
}

int update_hbb_controlturl(char *url, uint32_t len)
{
	if( xSemaphoreTake( hbb_info.xSemaphore, 1000) == pdTRUE)
	{
		memset(hbb_info.config.controlurl, 0, 128);
		strncpy(hbb_info.config.controlurl, url, len);
		xSemaphoreGive(hbb_info.xSemaphore);
		sync_hbb_info();
	} else {
		PRINTF("%s failed\r\n", __func__);
		return -1;
	}

	PRINTF("%s success\r\n", __func__);

	return 0;
}

int update_hbb_fotaurl(char *url, uint32_t len)
{
	if( xSemaphoreTake( hbb_info.xSemaphore, 1000) == pdTRUE)
	{
		memset(hbb_info.config.fotaurl, 0, 128);
		strncpy(hbb_info.config.fotaurl, url, len);
		xSemaphoreGive(hbb_info.xSemaphore);
		sync_hbb_info();
	} else {
		PRINTF("%s failed\r\n", __func__);
		return -1;
	}

	PRINTF("%s success\r\n", __func__);

	return 0;
}

int hb_get_sensor_list()
{


	PRINTF("Get sensor list........\r\n");
//#ifdef MQTT
//	MQTT_SEND_Q_MSG mqtt_get_dev_msg;
//	mqtt_get_dev_msg.send_cmd = MQTT_GET_DEV_LIST;
//	mqtt_get_dev_msg.get_dev_list.func = FUN_DEV_LIST;
//	xQueueSend(mqtt_send_q_handle, &mqtt_get_dev_msg, portMAX_DELAY);
//#else
	if(HOMEBOX_GROUP == DEVICE_TYPE_Group(ver_info.dt))
	{
		UPLINK_Q_MSG uplink_get_dev_msg;
		uplink_get_dev_msg.cmd = UPLINK_GET_SUB_DEVICES;
		uplink_get_dev_msg.sub_dev_req.pos = 1;
		if(uplink_get_dev_msg.sub_dev_req.pos <= 0 || uplink_get_dev_msg.sub_dev_req.pos >= 256){
			PRINTF("The argument of list is invalid!\r\n");
			return 0;
		}
		xQueueSend(uplink_q_handle, &uplink_get_dev_msg, portMAX_DELAY);
	}
#ifdef BRIDGE
	else if(BRIDGE_GROUP == DEVICE_TYPE_Group(ver_info.dt))
	{
		vTaskDelay(1000);
		sent_plc_event_msg(PLC_CMD_REQUEST_SENSOR_LIST,ver_info.dt,0,++plc_local_seq,0);
	}
#endif
	EventBits_t uxBits;
	uxBits = xEventGroupWaitBits(hbbeventgroup, HBB_GETLIST_EVENT,
						pdTRUE,	pdFALSE, WAIT_SENSOR_LIST_TIMEOUT);
	if (uxBits & HBB_GETLIST_EVENT)
	{
		PR_INFO("get sensor list success\r\n");
		sync_sensor_list();
	} else {
		PR_INFO("get sensor list failed\r\n");
		get_sensor_list();
	}
//#endif


	return 0;
}

void init_hbb()
{
	hbb_info.xSemaphore = xSemaphoreCreateMutex();
	hbb_info.timer = xTimerCreate
            ( "hbb",
              hbb_info.config.heartbeat_interval,
              pdTRUE,
              ( void * ) 0,
			  hbCallback
            );

	 if( hbb_info.timer == NULL )
	 {
		 PRINTF("create hbb timer failed\r\n");
	 }

	 if (xTimerStart(hbb_info.timer, 0) != pdPASS)
		 		PRINTF("start hbb info timer failed\r\n");


	 PRINTF("send hb boot info\r\n");

}

void init_sensor_list()
{
	for(int i=0; i<MAX_SENSOR_COUNT; i++)
	{
		sprintf(sensor_status[i].name, "%d", i);
		PRINTF("send message %d %s\r\n", strtoul(sensor_status[i].name, NULL, 10), sensor_status[i].name);
		sensor_status[i].send_message.header = PROTOCOL_3xx;
		sensor_status[i].send_message.en = 0;
		sensor_status[i].send_message.sn_dest = 0;
		sensor_status[i].send_message.sn_mitt = hbb_info.config.id;
		sensor_status[i].send_message.cmd = RF_CMD_GENERIC_REQUEST;
		sensor_status[i].send_message.enable_cmd = 0;
		sensor_status[i].send_message.sequence = local_seq++;
		memset(&sensor_status[i].sensor_attr, 0, sizeof(struct attr_item));
		memset(&sensor_status[i].todo_work, 0, sizeof(struct work_msg) * 2);
		vListInitialise( &sensor_status[i].todo_list );
		vListInitialise( &sensor_status[i].attr_list );
	}
}

int timer_get_index(TimerHandle_t xTimer)
{
	return strtoul(pcTimerGetTimerName( xTimer ), NULL, 10);
}

void alarmCallback( TimerHandle_t xTimer )
{
	RF_MSG* rf_msg;
	int idx;
	SENSOR_STATUS_STORAGE_BLOCK *status;

	idx = timer_get_index(xTimer);
	if (idx == -1)
		return;

	status = &sensor_status[idx];
	rf_msg = &sensor_status[idx].send_message;

	PRINTF("timer name %s %x %d retry %d\r\n", pcTimerGetTimerName( xTimer ), rf_msg->sn_dest, status->x_tick, status->qretry_count);

	if (status->qretry_count >= 6)
	{
		xTimerStop(xTimer, 0);

		if (status->x_tick < (hbb_info.config.heartbeat_interval/2))
		{
			if( xTimerChangePeriod( xTimer, status->x_tick*2, 0) != pdPASS )
				DBG_MESSAGE("change alarm timer failed\r\n");
			status->x_tick = status->x_tick*2;

			if (xTimerStart(xTimer, 0) != pdPASS)
				PRINTF("start alarm timer failed\r\n");
			status->qretry_count = 0;

			PRINTF("alarm retry change timer to %d\r\n", status->x_tick);
		}
	}

	if (rf_msg->alarm_status != 1) {
		if (status->x_tick == SENSOR_ALARM_POLLING_INTERVAL)
			PRINTF("warning sensor query 10s, not in alarm\r\n");
	}

	PRINTF("alarm send RF_CMD_GENERIC_REQUEST %x\r\n", rf_msg->sn_dest);
	rf_msg->cmd = RF_CMD_GENERIC_REQUEST;
	rf_msg->sequence++;
	status->qretry_count++;

	rf_send_then_receive(rf_msg);

	if( xTimerReset( xTimer, 0 ) != pdPASS )
	{
		PRINTF("reset timer failed\r\n");
	}
	PRINTF("timer name %s leave\r\n", pcTimerGetTimerName( xTimer ));
}

void init_timer_list()
{
	SENSOR_STATUS_STORAGE_BLOCK *status = &sensor_status[0];

	for(int i=0; i<MAX_SENSOR_COUNT; i++)
	{
		sensor_status[i].alarm_timers = xTimerCreate
            ( /* Just a text name, not used by the RTOS
              kernel. */
              sensor_status[i].name,
              /* The timer period in ticks, must be
              greater than 0. */
			  SENSOR_POLLING_INTERVAL,
              /* The timers will auto-reload themselves
              when they expire. */
              pdFALSE,
              /* The ID is used to store a count of the
              number of times the timer has expired, which
              is initialised to 0. */
              ( void * ) 0,
              /* Each timer calls the same callback when
              it expires. */
			  alarmCallback
            );
		 if( status->alarm_timers == NULL )
		 {
			 PRINTF("create alarm timer failed\r\n");
		 }

		 sensor_status[i].x_tick = SENSOR_POLLING_INTERVAL;
	}


}

int sensor_get_index_id(uint32_t id)
{
	for(int i=0; i<MAX_SENSOR_COUNT; i++)
	{
		if (sensor_status[i].send_message.sn_dest == id)
			return i;
	}

//	PRINTF("sensor id not match\r\n");
	return -1;
}

uint32_t sid_to_devicetype(uint32_t sid)
{
	uint32_t dt = 0;

	switch(sid>>24)
	{
		case SENSOR_TYPE_WATER:
			dt = DEVICE_TYPE(SENSOR_GROUP, WATER_SENSOR, 1);
			break;
		case SENSOR_TYPE_SMOKE:
			dt = DEVICE_TYPE(SENSOR_GROUP, SMOKE_SENSOR, 1);
			break;
		case SENSOR_TYPE_MAGNETIC:
			dt = DEVICE_TYPE(SENSOR_GROUP, MAGNETIC_SENSOR, 1);
			break;
		case SENSOR_TYPE_PIR:
			dt = DEVICE_TYPE(SENSOR_GROUP, PIR_SENSOR, 1);
			break;
		case VOLTAGE_SENSOR:
			dt = DEVICE_TYPE(SENSOR_GROUP, VOLTAGE_SENSOR, 1);
			break;
		case SENSOR_TYPE_PIR_TX:
			dt = DEVICE_TYPE(SENSOR_GROUP, PIR_SENSOR, 2);
			break;
		case SENSOR_TYPE_SMOKE_TX:
			dt = DEVICE_TYPE(SENSOR_GROUP, SMOKE_SENSOR, 2);
			break;
		case SENSOR_TYPE_WATER_TX:
			dt = DEVICE_TYPE(SENSOR_GROUP, WATER_SENSOR, 2);
			break;
		case SENSOR_TYPE_MAGNETIC_TX:
			dt = DEVICE_TYPE(SENSOR_GROUP, MAGNETIC_SENSOR, 2);
			break;
		case SENSOR_TYPE_CONSUMPTION:
			dt = DEVICE_TYPE(SENSOR_GROUP, CONSUMPTION_SENSOR, 1);
			break;
		case SENSOR_TYPE_CO:
			dt = DEVICE_TYPE(SENSOR_GROUP, CO_SENSOR, 1);
			break;
		case SENSOR_TYPE_TEMPERATURE:
			dt = DEVICE_TYPE(SENSOR_GROUP, TEMPERATURE_SENSOR, 1);
			break;
		case SENSOR_TYPE_SIREN:
			dt = DEVICE_TYPE(SENSOR_GROUP, SIREN, 1);
			break;
		case SENSOR_TYPE_ACTUATOR:
			dt = DEVICE_TYPE(SENSOR_GROUP, ACTUATOR, 1);
			break;
		case SENSOR_TYPE_GAS:
			dt = DEVICE_TYPE(SENSOR_GROUP, GAS_SENSOR, 1);
			break;
		case BRIDGE_TYPE_PLC:
			dt = DEVICE_TYPE(BRIDGE_GROUP, BRIDGE_, 1);
			break;
		case SENSOR_TYPE_REMOTE:
			dt = DEVICE_TYPE(SENSOR_GROUP, SMOKE_SENSOR, 1);
			break;
		default:
			PRINTF("unexpected sensor type received\r\n");
			break;
	}

	return dt;
}

int id_in_devicelist(LONG_ID96 *longid, DEVICE_LIST *dl)
{
	int i;

	for (i = 0; i < dl->num; i++)
	{
		if ((longid->id == dl->idn[i].lid.id)
				&& (longid->idh == dl->idn[i].lid.idh)
				&& (longid->idm == dl->idn[i].lid.idm))
		{
			return 1;
		}
	}

	return 0;
}

int find_sensorlist(LONG_ID96 *longid)
{
	int idx;

	for (idx = 0; idx < MAX_SENSOR_COUNT; idx++)
	{
		if (!((longid->id == sensor_status[idx].send_message.sn_dest)
				&& (longid->idh == sensor_status[idx].longidh)
				&& (longid->idm == sensor_status[idx].longidm)))
			return idx;
	}

	PRINTF("sensor not found\r\n");
	return -ENODEV;
}

int uninstall_sensor(LONG_ID96 *longid, int idx)
{
	if (idx < 0 && idx >= MAX_SENSOR_COUNT)
	{
		return -EINVAL;
	}

	if (!((longid->id == sensor_status[idx].send_message.sn_dest)
				&& (longid->idh == sensor_status[idx].longidh)
				&& (longid->idm == sensor_status[idx].longidm)))
	{
		return -EINVAL;
	}

	if (sensor_status[idx].send_message.sn_mitt == hbb_info.config.id)
	{
		PRINTF("uninstall sensor %x on homebox\r\n", sensor_status[idx].send_message.sn_dest);
		if (sensor_status[idx].send_message.alarm_status == 0)
		{
			sensor_status[idx].send_message.sn_dest = 0;
			sensor_status[idx].send_message.sn_mitt = 0;
			sensor_status[idx].longidh = 0;
			sensor_status[idx].longidm = 0;
			sensor_status[idx].device_type = 0;
			sensor_status[idx].send_message.en = 0;
			sensor_status[idx].type_handle = NULL;

			return 0;
		}
		else
		{
			return -EAGAIN;
		}
	}
	else
	{
		PRINTF("uninstall sensor %x on bridge\r\n", sensor_status[idx].send_message.sn_dest);

		return 0;
	}

	return -ENODEV;
}

void uninstall_sensors(DEVICE_LIST *dl)
{
	int j;
	LONG_ID96 slid;

	for (j = 0; j < MAX_SENSOR_COUNT; j++)
	{
		if (!sensor_status[j].send_message.sn_dest)
			continue;
		slid.id = sensor_status[j].send_message.sn_dest;
		slid.idh = sensor_status[j].longidh;
		slid.idm = sensor_status[j].longidm;
		if (!id_in_devicelist(&slid, dl))
			uninstall_sensor(&slid, j);
	}
}

int install_sensor(LONG_ID96 *longid, uint32_t dt)
{
	int i;
	uint32_t sid = longid->id;
	int found = MAX_SENSOR_COUNT;

#ifdef PLC
	if (DEVICE_TYPE_Group(dt) == BRIDGE_GROUP)
	{
		bridge_list[0].id = longid->id;
		bridge_list[0].longidh = longid->idh;
		bridge_list[0].longidm = longid->idm;
		bridge_list[0].dt = dt;

		return 0;
	}
#endif

	//add by cheryl
	if(sid == 0 || sid == 0xffffffff)
	{
		return -1;
	}

	for(i=0; i<MAX_SENSOR_COUNT; i++) {
		if (sensor_status[i].send_message.sn_dest == sid)
		{
			PRINTF("sensor %x has installed\r\n", sid);
			return -1;
		}
		else if (sensor_status[i].send_message.sn_dest == 0)
		{
			if (found == MAX_SENSOR_COUNT)
				found = i;
		}
	}

	if (found == MAX_SENSOR_COUNT)
	{
		PRINTF("sensor list full\r\n");
		return -1;
	}

//	dt = sid_to_devicetype(sid);
//	if (dt == 0)
//		return -1;


	if (get_sensor_type_handle(&sensor_status[found], dt))
	{
		return -1;
	}

	sensor_status[found].send_message.sn_dest = sid;
	sensor_status[found].send_message.sn_mitt = hbb_info.config.id;
	sensor_status[found].longidh = longid->idh;
	sensor_status[found].longidm = longid->idm;
	sensor_status[found].device_type = dt;
	sensor_status[found].send_message.en = 1;

	return 0;
}


void test_enable_sensor()
{
	int i;
	LONG_ID96 lid;
#ifndef BRIDGE
	lid.idh = HBB_LONG_IDH;
	lid.idm = HBB_LONG_IDM;
#endif
#ifdef CPU_MK24FN256VDC12
	//	lid.id = 0x05555555;

//	lid.id = 0x50020025;
//	install_sensor(&lid, sid_to_devicetype(lid.id));
#endif

	PRINTF("sensor list+++++++++++++++++++++\r\n");
	for(i=0; i<MAX_SENSOR_COUNT; i++)
		if (sensor_status[i].send_message.sn_dest != 0)
			PRINTF("%x\r\n", sensor_status[i].send_message.sn_dest);
}

void hbb_first_event( void )
{

	hbb_send_uplink_event(hbb_info.config.id, INFO);

}

void rf_message_send_msqg(uint8_t *buf);

err_t
hb_do_rf_send_callback(uint32_t sid, RF_MSG *msg)
{
  err_t err;
  sys_sem_t sem;
  SENSOR_STATUS_STORAGE_BLOCK *sensor_info;
  RF_MSG* rf_msg;
  int idx = sensor_get_index_id(sid);

  if (idx == -1)
	  return ERR_VAL;

  sensor_info = &sensor_status[idx];
  rf_msg = &sensor_info->send_message;

  err = sys_sem_new(&sem, 0);
  if (err != ERR_OK) {
    return err;
  }

  msg->sn_mitt = sid;
  msg->header = PROTOCOL_CALLBACK;
  rf_msg->cmd = msg->cmd;
  rf_msg->sn_dest = msg->sn_mitt;
  rf_msg->sequence++;
  rf_msg->enable_cmd = msg->enable_cmd;
  rf_msg->enable_data = msg->enable_data;

  msg->sequence = rf_msg->sequence;
  sensor_info->status = SENSOR_STATUS_WAIT_CALLBACK;
  sensor_info->err = &err;
  sensor_info->sem = &sem;
  sensor_info->arg = msg;
  sensor_info->arglen = sizeof(RF_MSG);

  rf_message_send_msqg((uint8_t *)msg);

  if (sys_arch_sem_wait(&sem, 2000) == SYS_ARCH_TIMEOUT)
	  err = ERR_TIMEOUT;
  sensor_info->sem = NULL;
  sensor_info->status = SENSOR_STATUS_NORMAL;
  sensor_info->arg = NULL;
  sensor_info->arglen = 0;
  sys_sem_free(&sem);

  return err;
}

void sensor_status_task(void *pvParameters)
{
	RF_MSG xReceivedmsg;
	hbb_config oldconfig;
#ifndef BRIDGE
	init_hbb();
#else

#endif
	AES_set_decrypt_key(rf_aeskey, 128, &dec_key);
	AES_set_encrypt_key(rf_aeskey, 128, &enc_key);
	init_sensor_list();
	get_default_hbbconfig(&oldconfig);

	hb_get_sensor_list();
	test_enable_sensor();

	init_timer_list();

	hbb_send_uplink_data(hbb_info.config.id, 40,  7, "%d", reset_reason);
	InitCheckPeripherals();

#ifndef BRIDGE
	hbb_first_event();
	vTaskDelay(500);
	hbCallback(hbb_info.timer);
#endif
	for(;;)
	{
		// waken up by RF
		memset(&xReceivedmsg, 0, sizeof(xReceivedmsg));
		if (xQueueReceive( rx_msg_queue, &xReceivedmsg, portMAX_DELAY ))
		{
//          uint8_t *fifobuf;
//			fifobuf = &xReceivedmsg.header;
//		    taskENTER_CRITICAL();
//		    PRINTF("xqueue received sn mitt %x\r\n", xReceivedmsg.sn_mitt);
//		    taskEXIT_CRITICAL();

			switch(xReceivedmsg.header)
			{
				case PROTOCOL_217:
				{
					protocol_2_17_sensor_type(&xReceivedmsg);
					break;
				}
				case PROTOCOL_3xx:
				{
	        		AES_decrypt((unsigned char *)&xReceivedmsg.sn_mitt, (unsigned char *)&xReceivedmsg.sn_mitt, &dec_key);

	            	if (Crc16_Calc((uint8_t*)&xReceivedmsg.sn_dest, PACKET_CRC_LENGTH) != xReceivedmsg.crc) {
	            		PRINTF("\r\nRF_RECV CRC ERROR!\r\n");
	            		break;
	            	}

					protocol_2_17_sensor_type(&xReceivedmsg);
					break;
				}
				case PROTOCOL_CALLBACK:
				{
					handle_msg_callback(&xReceivedmsg);
					break;
				}
				default:
				{
					DBG_MESSAGE("protocol version error\r\n");
					rf_receive();
					break;
				}
			}
		}
	}
}

