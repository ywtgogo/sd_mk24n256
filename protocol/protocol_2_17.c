#include "protocol_2_17.h"
#include "hb_protocol.h"
#include "uplink_protocol.h"
#include "timers.h"
#include <stdbool.h>
#include <stdarg.h>
#include <stdio.h>
#include "lwip/sys.h"
#include "rtc_fun.h"
#include "version.h"
#include "mqtt_task.h"
#include "plc_if.h"
#include "uart_plc.h"
#include "audio_play.h"
#ifdef WIFI_MODULE
#include "wifi_module.h"
#endif

#ifdef MQTT
extern QueueHandle_t mqtt_send_q_handle;
#endif


#define UPLINK_Q_DELAY 10000

#define SID_TO_TYPE(sid) ((sid >> 24) & 0xFF)
bool check_all_sensor_flag = false;
uint32_t aging_test_flag=0;
uint32_t aging_test_for_ppp=0;
extern uint32_t wifi_led_status_type;


uint32_t GET_SENSOR_VBATT(SENSOR_STATUS_STORAGE_BLOCK *sensor_state)
{
	int vbatt = sensor_state->send_message.vbatt;

	vbatt = 16 * vbatt * vbatt - 44;
	return  (vbatt > 0) ? vbatt:0;
}

int send_uplink_event_core(LONG_ID96 *sid, uint32_t dt, uint32_t status, uint32_t fwv, uint32_t hwv, int p, char *str)
{
	UPLINK_Q_MSG uplink_msg;

	memset(&uplink_msg, 0, sizeof(uplink_msg));

	uplink_msg.event.slid.idh = sid->idh;
	uplink_msg.event.slid.idm = sid->idm;
	uplink_msg.event.slid.id = sid->id;
	uplink_msg.cmd = UPLINK_SEND_EVENT;
	uplink_msg.event.device_type = dt;
	if (uplink_msg.event.device_type == 0)
		return -1;
	uplink_msg.event.status = status;
	uplink_msg.event.fwv = fwv;
	uplink_msg.event.hwv = hwv;
	uplink_msg.event.p = p;
	if (!str)
		uplink_msg.event.data[0] = '0';
	else
	{
		if (strlen(str) < 50)
			strcpy(uplink_msg.event.data, str);
	}

	get_current_systime(&uplink_msg.event.ts);

	PRINTF("%s sid %x status %d %s\r\n", __func__, sid->id, status, uplink_msg.event.data);

#if 0
	if (mqtt_send_q_handle != NULL)
		xQueueSend(mqtt_send_q_handle, (MQTT_SEND_Q_MSG *)&uplink_msg, portMAX_DELAY);
#else
	if (uplink_q_handle)
		xQueueSend(uplink_q_handle, &uplink_msg, UPLINK_Q_DELAY);
#endif

	return 0;
}

int send_uplink_event(uint32_t sid, uint32_t status)
{
	int idx = sensor_get_index_id(sid);
	SENSOR_STATUS_STORAGE_BLOCK *sensor;
	uint32_t fwversion;
	char data[50];
	int vbatt;
	LONG_ID96 slid;

	if (idx == -1)
	{
		PRINTF("invalid value\r\n");
		return -1;
	}

	sensor = &sensor_status[idx];
	slid.id = sid;
	slid.idh = sensor->longidh;
	slid.idm = sensor->longidm;
	fwversion = (sensor->send_message.fw_major << 24) | (sensor->send_message.fw_minor << 16);
	vbatt = GET_SENSOR_VBATT(sensor);
	sprintf(data, "10401/vt:%u", vbatt);


	if(HOMEBOX_GROUP == DEVICE_TYPE_Group(ver_info.dt))
	{

		send_uplink_event_core(&slid, sensor->device_type, status, fwversion, 0, 1, data);
	    hbb_info.uplink_sent++;

		if ((vbatt < 30) && (status == INFO))
		{
			PRINTF("low batt %d\r\n", vbatt);
			send_uplink_event_core(&slid, sensor->device_type, WARNING, fwversion, 0, 1, data);
			hbb_info.uplink_sent++;
		}
	}
#ifdef PLC
	else if(BRIDGE_GROUP == DEVICE_TYPE_Group(ver_info.dt))
	{
		switch(status)
		{
			case ALERT:
				sent_plc_event_msg(PLC_CMD_ALARM,sensor->device_type,sid,++plc_local_seq,0);
				break;
			case UNALERT:
				sent_plc_event_msg(PLC_CMD_ALARM_CLEAR,sensor->device_type,sid,++plc_local_seq,0);
				break;
			default:
				PRINTF("ERR: wrong status:%d,%s\r\n",status,__func__);
				break;
		}

	}
#endif
	else
	{
		PRINTF("ERR : error device type:%s\r\n",__func__);
	}


	return 0;
}

int hbb_send_uplink_event(uint32_t sid, uint32_t status)
{

	LONG_ID96 lid;


	lid.id = sid;
	lid.idh = hbb_info.config.longidh;
	lid.idm = hbb_info.config.longidm;
	send_uplink_event_core(&lid, ver_info.dt, status, ver_info.swver, ver_info.hwver, 1, NULL);
	hbb_info.uplink_sent++;

	return 0;
}

int hbb_send_uplink_data(uint32_t sid, uint32_t status, int p, char *fmt_ptr, ...)
{
	va_list ap;
	char data[50];
	int result;
	LONG_ID96 lid;

	lid.id = sid;
	lid.idh = hbb_info.config.longidh;
	lid.idm = hbb_info.config.longidm;

	memset(data, 0, 50);
    va_start(ap, fmt_ptr);
    result = vsprintf(data, fmt_ptr, ap);
    va_end(ap);

    PRINTF("%s %s\r\n", __func__, data);
	send_uplink_event_core(&lid, ver_info.dt, status, ver_info.swver, ver_info.hwver, p, data);
	hbb_info.uplink_sent++;

	return result;
}

int sensor_send_uplink_data(uint32_t sid, uint32_t status, int p, char *fmt_ptr, ...)
{
	va_list ap;
	char data[50];
	int result;
	LONG_ID96 slid;
	SENSOR_STATUS_STORAGE_BLOCK *sensor;
	int idx = sensor_get_index_id(sid);
	uint32_t fwversion;
	int vbatt;

	if (idx == -1)
	{
		PRINTF("invalid value\r\n");
		return -1;
	}

	sensor = &sensor_status[idx];
	fwversion = (sensor->send_message.fw_major << 24) | (sensor->send_message.fw_minor << 16);
	slid.id = sid;
	slid.idh = sensor->longidh;
	slid.idm = sensor->longidm;
	memset(data, 0, 50);
    va_start(ap, fmt_ptr);
    result = vsprintf(data, fmt_ptr, ap);
    va_end(ap);

    vbatt = ((sensor->send_message.vbatt + 1) / 4) * 100;
	sprintf(data + result, "10401/vt:%d", vbatt);

    PRINTF("%s %s\r\n", __func__, data);
	send_uplink_event_core(&slid, sensor->device_type, status, 0, 0, p, data);
	hbb_info.uplink_sent++;


	if(HOMEBOX_GROUP == DEVICE_TYPE_Group(ver_info.dt))
	{
		send_uplink_event_core(&slid, sensor->device_type, status, fwversion, 0, 1, data);
		hbb_info.uplink_sent++;
	}
#ifdef PLC
	else if(BRIDGE_GROUP == DEVICE_TYPE_Group(ver_info.dt))
	{
		switch(status)
		{
			case ALERT:
				sent_plc_event_msg(PLC_CMD_ALARM,sensor->device_type,sid,++plc_local_seq,0);
				break;
			case UNALERT:
				sent_plc_event_msg(PLC_CMD_ALARM_CLEAR,sensor->device_type,sid,++plc_local_seq,0);
				break;
			default:
				PRINTF("ERR: wrong status:%d,%s\r\n",status,__func__);
				break;
		}

	}
#endif
	else
	{
		PRINTF("ERR : error device type:%s\r\n",__func__);
	}

	return result;
}


void sync_aging_test_info()
{
	taskENTER_CRITICAL();
	FLASH_RTOS_Erase_Sector(&aging_test_flag, AGING_TEST_INFO_CONFIG, 1);
	taskEXIT_CRITICAL();
}

void get_aging_test_info()
{
	memcpy((void *)&aging_test_flag, (void *)(AGING_TEST_INFO_CONFIG),  sizeof(aging_test_flag));
}


void protocol_2_17_sensor_type(RF_MSG *get_queue_packet)
{
	int idx = sensor_get_index_id(get_queue_packet->sn_mitt);
	SENSOR_STATUS_STORAGE_BLOCK *status;
	RF_MSG* rf_msg = get_queue_packet;
	uint32_t sn_temp=0;
	u8_t term_status = 0xff;
	//PRINTF("..msg....cmd=0x%x..sn_mitt=0x%x..sn_dest=0x%x..\r\n",get_queue_packet->cmd,get_queue_packet->sn_mitt,get_queue_packet->sn_dest);
	if (idx == -1)
	{
		if((get_queue_packet->sn_mitt == 0xA5A5A5A5)&&
			((get_queue_packet->sn_dest == 0xAAAAAAA)||(get_queue_packet->sn_dest == hbb_info.config.id)))
		{
			PRINTF("..test..cmd=0x%x..\r\n",get_queue_packet->cmd);
			#if 1
			if(get_queue_packet->cmd == RF_CMD_AGING_TEST_PREPARE)
			{
				sn_temp = get_queue_packet->sn_dest;
				rf_msg->sn_dest = get_queue_packet->sn_mitt;
				rf_msg->sn_mitt = sn_temp;
				rf_msg->cmd = RF_CMD_AGING_TEST_PREPARE_ACK;
				rf_msg->sequence = get_queue_packet->sequence;
				rf_send_then_receive(rf_msg);
				PRINTF("..aging test prepare..\r\n");
			}
			else if(get_queue_packet->cmd == RF_CMD_AGING_TEST_START)
			{
				get_aging_test_info();	 //get aging test info, set to aging_test_flag
				if(aging_test_flag != 0)
				{	
					PRINTF("..aging test start..\r\n");
					aging_test_for_ppp = 1;
					#ifdef WIFI_MODULE
					WIFI_RED_LED_ON();
					WIFI_GREEN_LED_OFF();
					#else
					PPP_GREEN_LED_OFF();
					PPP_RED_LED_ON();
					#endif
					while(1)
					{
						sn_temp = get_queue_packet->sn_dest;
						rf_msg->sn_dest = get_queue_packet->sn_mitt;
						rf_msg->sn_mitt = sn_temp;
						rf_msg->cmd = RF_CMD_AGING_TEST_START_ACK;
						rf_msg->sequence = get_queue_packet->sequence;
						rf_send_then_receive(rf_msg);
						vTaskDelay(5000);
						#ifdef WIFI_MODULE
						wifi_led_status_type = WIFI_LED_RED_TOGGLE;
						#else
						PPP_RED_LED_OFF();
						vTaskDelay(500);
						PPP_RED_LED_ON();
						#endif
					}
				}
				else
				{
					aging_test_for_ppp = 0;
					PRINTF("..aging test has done..\r\n");
				}
			}
			else if(get_queue_packet->cmd == RF_CMD_AGING_TEST_COMPLETE)
			{
				sn_temp = get_queue_packet->sn_dest;
				rf_msg->sn_dest = get_queue_packet->sn_mitt;
				rf_msg->sn_mitt = sn_temp;
				rf_msg->cmd = RF_CMD_AGING_TEST_COMPLETE_ACK;
				rf_msg->sequence = get_queue_packet->sequence;
				rf_send_then_receive(rf_msg);
				aging_test_flag = 0;
				aging_test_for_ppp = 0;
				sync_aging_test_info();
				PRINTF("..aging test complete..\r\n");
			}
			else if(get_queue_packet->cmd == RF_CMD_TERM_TEST_QUERY)
			{
				term_status = get_queue_packet->vbatt|(get_queue_packet->wor<<2);
				PRINTF("..term test..status=%d..vbatt=%d..wor=%d..\r\n",term_status,get_queue_packet->vbatt,get_queue_packet->wor);
				if(term_status == 0)
				{
					sn_temp = get_queue_packet->sn_dest;
					rf_msg->sn_dest = get_queue_packet->sn_mitt;
					rf_msg->sn_mitt = sn_temp;
					rf_msg->cmd = RF_CMD_TERM_TEST_QUERY_ACK;
					rf_msg->sequence = get_queue_packet->sequence;
					rf_msg->vbatt = get_queue_packet->vbatt;
					rf_msg->wor = get_queue_packet->wor;
					rf_msg->fw_major = is_networking();
					PRINTF("..term test..fw_major=%d..\r\n",rf_msg->fw_major);
					rf_send_then_receive(rf_msg);
					PRINTF("..term test networking status..\r\n");
				}
				else if((term_status >= 1)&&(term_status <= 3))
				{
					sn_temp = get_queue_packet->sn_dest;
					rf_msg->sn_dest = get_queue_packet->sn_mitt;
					rf_msg->sn_mitt = sn_temp;
					rf_msg->cmd = RF_CMD_TERM_TEST_QUERY_ACK;
					rf_msg->sequence = get_queue_packet->sequence;
					rf_msg->vbatt = get_queue_packet->vbatt;
					rf_msg->wor = get_queue_packet->wor;
					PRINTF("..term test..id=0x%x..idm=0x%x..idh=0x%x..\r\n",hbb_info.config.id,hbb_info.config.longidm,hbb_info.config.longidh);
					switch(term_status)
					{
						case 1:
							rf_msg->fw_major = (hbb_info.config.id&0xff);
							rf_msg->data = (hbb_info.config.id&0xff00)>>8;
							rf_msg->fw_minor = (hbb_info.config.id&0xff0000)>>16;
							rf_msg->rf_reset_cnt = (hbb_info.config.id&0xff000000)>>24;
							break;
						case 2:
							rf_msg->fw_major = (hbb_info.config.longidm&0xff);
							rf_msg->data = (hbb_info.config.longidm&0xff00)>>8;
							rf_msg->fw_minor = (hbb_info.config.longidm&0xff0000)>>16;
							rf_msg->rf_reset_cnt = (hbb_info.config.longidm&0xff000000)>>24;
							break;
						case 3:
							rf_msg->fw_major = (hbb_info.config.longidh&0xff);
							rf_msg->data = (hbb_info.config.longidh&0xff00)>>8;
							rf_msg->fw_minor = (hbb_info.config.longidh&0xff0000)>>16;
							rf_msg->rf_reset_cnt = (hbb_info.config.longidh&0xff000000)>>24;
							break;
					}					
					PRINTF("..term test..fw_major=%d..\r\n",rf_msg->fw_major);
					rf_send_then_receive(rf_msg);
					PRINTF("..term test send hb longid..\r\n");
				}
				else
				{
					PRINTF("..term test..status error..\r\n");
				}
			}
			#endif
		}
		else
		{
			return;
		}
	}

	status = &sensor_status[idx];

	PRINTF("..dest=%8x, mitt=%8x, seq=%d, cmd=%2d, alm=%d, sent=%d, batt=%d, wor=%d,enable_cmd=%d, enable_data=%d, fw=%d.%d\r\n",
			get_queue_packet->sn_dest,get_queue_packet->sn_mitt,get_queue_packet->sequence,get_queue_packet->cmd,get_queue_packet->alarm_status,get_queue_packet->alarm_sent,
			get_queue_packet->vbatt,get_queue_packet->wor,get_queue_packet->enable_cmd,get_queue_packet->enable_data,get_queue_packet->fw_major,get_queue_packet->fw_minor);

    for(int j=0;j<23;j++)
	{
		PRINTF("%02x ", (uint8_t)(*((uint8_t *)&get_queue_packet->header+j)));
	}
	if(check_all_sensor_flag)
	{
		if(get_queue_packet->rssi > status->send_message.rssi)
		{
			status->send_message.rssi = get_queue_packet->rssi;
			status->send_message.sn_mitt = hbb_info.config.id;
		}
		status->last_ack_tick = xTaskGetTickCount();
	}
	else
	{
		if (hbb_info.config.id != status->send_message.sn_mitt)
			return;
		
		PRINTF("##received rf msg sn_mitt %x\r\n", get_queue_packet->sn_mitt);
		status->send_message.rssi = get_queue_packet->rssi;
		status->send_message.vbatt = get_queue_packet->vbatt;
		if (status->type_handle)
			if(status->type_handle->handle_rf_message)
				status->type_handle->handle_rf_message(status, get_queue_packet);

	}

	if (status->status == SENSOR_STATUS_WAIT_CALLBACK)
	{
		PRINTF("%s wait callback\r\n", __func__);

		status->status = SENSOR_STATUS_NORMAL;
		PRINTF("sn_dest %x\r\n", get_queue_packet->sn_dest);
		PRINTF("sn_mitt %x\r\n", get_queue_packet->sn_mitt);
		PRINTF("cmd %x\r\n", get_queue_packet->cmd);
		PRINTF("rssi %d\r\n", get_queue_packet->rssi);
		PRINTF("alarm status %d\r\n", get_queue_packet->alarm_status);
		PRINTF("alarm sent %d\r\n", get_queue_packet->alarm_sent);
		PRINTF("vbatt %d\r\n", get_queue_packet->vbatt);
		PRINTF("fw_major %x\r\n", get_queue_packet->fw_major);
		PRINTF("fw_minor %x\r\n", get_queue_packet->fw_minor);
		if (status->sem != NULL)
			sys_sem_signal(status->sem);
	}
	rf_receive();
}

void handle_msg_callback(RF_MSG *get_queue_packet)
{

	int idx = sensor_get_index_id(get_queue_packet->sn_mitt);
	SENSOR_STATUS_STORAGE_BLOCK *status;

	PRINTF("%s\r\n", __func__);
	if (idx == -1)
		return;

	status = &sensor_status[idx];

	if (!check_all_sensor_flag && status->send_message.sn_mitt != hbb_info.config.id)
		return;

	RF_MSG* rf_msg = &status->send_message;

	PRINTF("%s rf send %x cmd %d\r\n", __func__, rf_msg->sn_dest, rf_msg->cmd);

	rf_send_then_receive(rf_msg);
}

//int alloc_sensor_attrs(SENSOR_STATUS_STORAGE_BLOCK *status)
//{
//	struct attr_item *item = ( struct attr_item * ) pvPortMalloc( sizeof( struct attr_item ) );
//	if( item != NULL )
//	{
//		vListInitialiseItem( &( item->xGenericListItem ) );
//		item->nattr = 0;
//		vListInsert( &( status->attr_list ), &( item->xGenericListItem ) );
//}
//
//int free_sensor_attrs(SENSOR_STATUS_STORAGE_BLOCK *status)
//{
//
//}

struct sensor_attribute *get_free_attr(SENSOR_STATUS_STORAGE_BLOCK *status)
{
	struct attr_item *aitem = &status->sensor_attr;

	if (aitem->nattr >= 4)
		return NULL;

	for (int i = 0; i < 4; i++)
	{
		if (aitem->attr[i].index == 0)
		{
			aitem->nattr++;
			printf("%s  num %d %d\r\n", __func__, aitem->nattr, i);
			return &aitem->attr[i];
		}
	}

	PRINTF("sensor attr bug\r\n");
	return NULL;
}

struct sensor_attribute *find_attr(SENSOR_STATUS_STORAGE_BLOCK *status, int index)
{
	struct attr_item *aitem = &status->sensor_attr;

	for (int i = 0; i < aitem->nattr; i++)
	{
		printf("%s %d %d\r\n", __func__, index, aitem->attr[i].index);
		if (aitem->attr[i].index == index)
		{
			return &aitem->attr[i];
		}
	}

	return NULL;
}

void put_attr(SENSOR_STATUS_STORAGE_BLOCK *status, int index)
{
	struct attr_item *aitem = &status->sensor_attr;
	struct sensor_attribute *sattr;

	sattr = find_attr(status, index);
	if (sattr)
	{
		memset(sattr, 0, sizeof(sattr));
		aitem->nattr--;

		if (aitem->nattr < 0)
			PRINTF("sensor attr put bug\r\n");
	}
}

void print_attr(SENSOR_STATUS_STORAGE_BLOCK *status)
{
	struct attr_item *aitem = &status->sensor_attr;

	for (int i = 0; i < aitem->nattr; i++)
	{
		printf("attr index %d, value %08x\r\n", aitem->attr[i].index, aitem->attr[i].value);
	}

}

int add_todo_work(SENSOR_STATUS_STORAGE_BLOCK *status,
		void (* function)(void *arg, struct sensor_attribute *attribute),
		struct sensor_attribute *attr)
{
	struct work_msg *work;

	for (int i = 0; i < 2; i++)
	{
		work =  &status->todo_work[i];
		if (work->arg == NULL)
		{
			work->arg = attr;
			work->error = 0;
			work->retry = 0;
			work->function = function;
			work->x_tick = xTaskGetTickCount();

			printf("add work %d\r\n", work->arg->index);
			return 0;
		}
	}

	return -1;
}



void get_attribute(void *sarg, struct sensor_attribute *arg)
{
	SENSOR_STATUS_STORAGE_BLOCK *status = (SENSOR_STATUS_STORAGE_BLOCK *)sarg;
	RF_MSG_EXT *rf_packet_ext = (RF_MSG_EXT *)&status->send_message;

	printf("%s %d\r\n", __func__, __LINE__);

	rf_packet_ext->cmd = RF_CMD_GET_ATTRIBUTE;
	rf_packet_ext->sequence = local_seq++;
	rf_packet_ext->index = arg->index;

	rf_send_then_receive_no_preamble((RF_MSG *)rf_packet_ext);
}

void set_attribute(void *sarg, struct sensor_attribute *arg)
{
	SENSOR_STATUS_STORAGE_BLOCK *status = (SENSOR_STATUS_STORAGE_BLOCK *)sarg;
	RF_MSG_EXT *rf_packet_ext = (RF_MSG_EXT *)&status->send_message;

	printf("%s %d\r\n", __func__, __LINE__);

	rf_packet_ext->cmd = RF_CMD_SET_ATTRIBUTE;
	rf_packet_ext->sequence = local_seq++;
	rf_packet_ext->index = arg->index;
//	rf_packet_ext->value = arg->value;

	__put_unaligned_le32(arg->value, (uint8_t *)&rf_packet_ext->value);

    for(int j=0;j<23;j++)
	{
		PRINTF("%02x ", (uint8_t)(*((uint8_t *)&rf_packet_ext->header+j)));
	}

	rf_send_then_receive_no_preamble((RF_MSG *)rf_packet_ext);
}

void print_todo_work(SENSOR_STATUS_STORAGE_BLOCK *status)
{
	struct work_msg *work;

	for (int i = 0; i < 2; i++)
	{
		work =  &status->todo_work[i];
		if (work->arg != NULL)
		{
			printf("todo index %d func %s\r\n", work->arg->index, (work->function==get_attribute)?"get":"set");
		}
	}
}

uint32_t get_sensor_attribute(SENSOR_STATUS_STORAGE_BLOCK *status, int index)
{
	struct sensor_attribute *sattr;

	sattr = find_attr(status, index);
	if (sattr)
	{
		printf("%s %d\r\n", __func__, __LINE__);

		sattr->value = portMAX_DELAY;
		if (!add_todo_work(status, get_attribute, sattr))
			return -EAGAIN;
	}
	else
	{
		sattr = get_free_attr(status);
		printf("%s %d\r\n", __func__, __LINE__);

		if (sattr)
		{
			printf("%s %d\r\n", __func__, __LINE__);

			sattr->index = index;
			sattr->value = portMAX_DELAY;
			if (!add_todo_work(status, get_attribute, sattr))
				return -EAGAIN;
		}
	}

	return portMAX_DELAY;
}


int set_sensor_attribute(SENSOR_STATUS_STORAGE_BLOCK *status, int index, uint32_t value)
{
	struct sensor_attribute *sattr;

	sattr = find_attr(status, index);
	if (sattr)
	{
		if (sattr->value == value)
			return 0;
		else
		{
			sattr->index = index;
			sattr->value = value;
			if (!add_todo_work(status, set_attribute, sattr))
				return -EAGAIN;
		}
	}
	else
	{
		sattr = get_free_attr(status);
		if (sattr)
		{
			sattr->index = index;
			sattr->value = value;
			if (!add_todo_work(status, set_attribute, sattr))
				return -EAGAIN;
		}
	}

	return portMAX_DELAY;
}

uint32_t user_get_sensor_attribute(uint32_t sid, int index)
{
	SENSOR_STATUS_STORAGE_BLOCK *sensor_info;
	int idx = sensor_get_index_id(sid);
	printf("%s %d sid %x %d\r\n", __func__, __LINE__, sid, index);

	if (idx == -1)
		  return ERR_VAL;

	sensor_info = &sensor_status[idx];

	return get_sensor_attribute(sensor_info, index);
}

int user_set_sensor_attribute(uint32_t sid, int index, uint32_t value)
{
	SENSOR_STATUS_STORAGE_BLOCK *sensor_info;
	int idx = sensor_get_index_id(sid);

	if (idx == -1)
		  return ERR_VAL;

	sensor_info = &sensor_status[idx];

	return set_sensor_attribute(sensor_info, index, value);
}

int update_todo_work(SENSOR_STATUS_STORAGE_BLOCK *status, RF_MSG_EXT *rf_packet_ext)
{
	struct work_msg *work;
	int index = rf_packet_ext->index;
	uint32_t value = __get_unaligned_le32((uint8_t *)&rf_packet_ext->value);
	uint8_t cmd = rf_packet_ext->cmd;

	printf("%s %d index%d\r\n", __func__, __LINE__, index);

	for (int i = 0; i < 2; i++)
	{
		work =  &status->todo_work[i];

		if (work->arg)
			printf("%s %d %d\r\n", __func__, __LINE__, work->arg->index);

		if (work->function && work->arg && (work->arg->index == index))
		{
			if (cmd == RF_CMD_GET_ATTRIBUTE_ACK && value != 0xFFFFFFFF)
				work->arg->value = value;

			work->arg = NULL;
			work->function = NULL;
			printf("%s %d %p\r\n", __func__, __LINE__, work->arg);

			return 0;
		}
	}
	return 1;
}

int pending_todo_work(SENSOR_STATUS_STORAGE_BLOCK *status)
{
	struct work_msg *work;
	int pending = 0;

	printf("%s %d\r\n", __func__, __LINE__);
	for (int i = 0; i < 2; i++)
	{
		work =  &status->todo_work[i];
		if (work->arg != NULL)
		{
			pending++;
		}
	}

	return pending;
}

void do_todo_work(SENSOR_STATUS_STORAGE_BLOCK *status)
{
	struct work_msg *work;
	printf("%s %d\r\n", __func__, __LINE__);

	for (int i = 0; i < 2; i++)
	{
		work =  &status->todo_work[i];
		if (work->arg != NULL)
		{
			work->retry++;
			work->function(status, work->arg);
		}
	}
}


int smart_water_smoke_2_17_handle(SENSOR_STATUS_STORAGE_BLOCK *status, RF_MSG *get_queue_packet)
{
	RF_MSG* rf_msg = &status->send_message;

	switch(get_queue_packet->cmd)
	{
		case RF_CMD_GENERIC_REQUEST_ACK:
		{
			PRINTF("%s cmd=%d rf status %d current status %d\r\n", __func__, RF_CMD_GENERIC_REQUEST_ACK, get_queue_packet->alarm_status, rf_msg->alarm_status);

			if(get_queue_packet->alarm_status == 0)
			{
				if (get_queue_packet->alarm_sent == 1)
				{
					rf_msg->sn_dest = get_queue_packet->sn_mitt;
					rf_msg->cmd = RF_CMD_ALARM_REARM;
					rf_msg->sequence++;

					rf_send_then_receive(rf_msg);
				}
				/* back to normal */
				if (get_queue_packet->alarm_sent == 0)
				{
//					if (status->x_tick != SENSOR_POLLING_INTERVAL)
//					{
//						xTimerStop(status->alarm_timers, 0);
//						if( xTimerChangePeriod( status->alarm_timers, SENSOR_POLLING_INTERVAL, 0) != pdPASS )
//							DBG_MESSAGE("change alarm timer failed\r\n");
//					}
//					status->qretry_count = 0;

					if(rf_msg->alarm_status == 1)
					{
						rf_msg->sn_dest = get_queue_packet->sn_mitt;
						rf_msg->cmd = RF_CMD_ALARM_REARM;
						rf_msg->sequence++;

						rf_send_then_receive(rf_msg);
					}
				}
			}

			status->last_ack_tick = xTaskGetTickCount();
			rf_msg->fw_major = get_queue_packet->fw_major;
			rf_msg->fw_minor = get_queue_packet->fw_minor;

			rf_receive();
			break;
		}
		case RF_CMD_UNSOLICITED_STATUS:
		{
			vTaskDelay(5);

			PRINTF("%s cmd=%d rf status %d current status %d\r\n", __func__, RF_CMD_UNSOLICITED_STATUS, get_queue_packet->alarm_status, rf_msg->alarm_status);
			print_log("id=%x cmd=%d rf status %d current status %d\r\n", rf_msg->sn_dest, RF_CMD_UNSOLICITED_STATUS, get_queue_packet->alarm_status, rf_msg->alarm_status);

			rf_msg->sn_dest = get_queue_packet->sn_mitt;
			rf_msg->cmd = RF_CMD_UNSOLICITED_STATUS_ACK;
			rf_msg->sequence = get_queue_packet->sequence;


			rf_send_then_receive_no_preamble(rf_msg);
			DBG_MESSAGE("RF_CMD_UNSOLICITED_STATUS_ACK sent\r\n");

			xTimerStop(status->alarm_timers, 0);
			if( xTimerChangePeriod( status->alarm_timers, SENSOR_ALARM_POLLING_INTERVAL, 0) != pdPASS )
					DBG_MESSAGE("change alarm timer failed\r\n");

			status->x_tick = SENSOR_ALARM_POLLING_INTERVAL;

			PRINTF("start alarm timer %d\r\n", status->x_tick);
			if (xTimerStart(status->alarm_timers, 0) != pdPASS)
				PRINTF("start alarm timer failed\r\n");

			if (rf_msg->alarm_status == 0)
			{

				send_uplink_event(rf_msg->sn_dest, ALERT);
				status->last_uplink_status = ALERT;

				rf_msg->alarm_status = 1;
#ifndef WIFI_MODULE
				if (status->type_handle->devicetype == SENSOR_TYPE_WATER)
					start_play_plan(AUDIO_ALARM_WATER);
				else if (status->type_handle->devicetype == SENSOR_TYPE_SMOKE)
					start_play_plan(AUDIO_ALARM_SMOKE);
#endif
			}
			status->qretry_count = 0;
			status->last_sequence = get_queue_packet->sequence;

			break;
		}
		case RF_CMD_ALARM_REARM_ACK:
		{
			PRINTF("%s cmd=%d rf status %d current status %d\r\n", __func__, RF_CMD_ALARM_REARM_ACK, get_queue_packet->alarm_status, rf_msg->alarm_status);
			print_log("id=%x cmd=%d rf status %d current status %d\r\n", rf_msg->sn_dest, RF_CMD_UNSOLICITED_STATUS, get_queue_packet->alarm_status, rf_msg->alarm_status);
			rf_msg->alarm_status = 0;
			send_uplink_event(rf_msg->sn_dest, UNALERT);
			status->last_uplink_status = UNALERT;

			if (status->x_tick != SENSOR_POLLING_INTERVAL)
			{
				xTimerStop(status->alarm_timers, 0);
				if( xTimerChangePeriod( status->alarm_timers, SENSOR_POLLING_INTERVAL, 0) != pdPASS )
					DBG_MESSAGE("change alarm timer failed\r\n");
				status->x_tick = SENSOR_POLLING_INTERVAL;

			}

			rf_receive();
			break;
		}
		default:
			DBG_MESSAGE("unexpected rf messsage received");
			return 1;
	}

	return 0;
}


int smart_mag_2_17_handle(SENSOR_STATUS_STORAGE_BLOCK *status, RF_MSG *get_queue_packet)
{
	RF_MSG* rf_msg = &status->send_message;

	switch(get_queue_packet->cmd)
	{
		case RF_CMD_GENERIC_REQUEST_ACK:
		{
			PRINTF("%s cmd=%d rf status %d current status %d\r\n", __func__, RF_CMD_GENERIC_REQUEST_ACK, get_queue_packet->alarm_status, rf_msg->alarm_status);
			//RF_MSG* rf_msg = get_msg(recv_packet.sn_mitt);
			status->send_message.enable_data = get_queue_packet->enable_data;
			if(get_queue_packet->alarm_status == 0)
			{
				rf_msg->alarm_status = 0;

				status->x_tick = hbb_info.config.heartbeat_interval;
				xTimerStop(status->alarm_timers, 0);

				PRINTF("dis alarm\r\n");
				print_log("id=%x cmd=%d rf status %d current status %d\r\n", rf_msg->sn_dest, RF_CMD_UNSOLICITED_STATUS, get_queue_packet->alarm_status, rf_msg->alarm_status);

				status->qretry_count = 0;

				if (status->last_uplink_status == ALERT)
				{
					send_uplink_event(rf_msg->sn_dest, UNALERT);
					status->last_uplink_status = UNALERT;
				}

			}
			rf_msg->fw_major = get_queue_packet->fw_major;
			rf_msg->fw_minor = get_queue_packet->fw_minor;
			status->last_ack_tick = xTaskGetTickCount();
			rf_receive();
			break;
		}
		case RF_CMD_UNSOLICITED_STATUS:
		{
			vTaskDelay(5);

			PRINTF("%s cmd=%d rf status %d current status %d\r\n", __func__, RF_CMD_UNSOLICITED_STATUS, get_queue_packet->alarm_status, rf_msg->alarm_status);
			print_log("id=%x cmd=%d rf status %d current status %d\r\n", rf_msg->sn_dest, RF_CMD_UNSOLICITED_STATUS, get_queue_packet->alarm_status, rf_msg->alarm_status);

			//engage_status = 0;
			rf_msg->sn_dest = get_queue_packet->sn_mitt;
			rf_msg->cmd = RF_CMD_UNSOLICITED_STATUS_ACK;
			rf_msg->sequence = get_queue_packet->sequence;

			rf_send_then_receive_no_preamble(rf_msg);

			xTimerStop(status->alarm_timers, 0);
			if( xTimerChangePeriod( status->alarm_timers, SENSOR_ALARM_POLLING_INTERVAL, 0) != pdPASS )
				DBG_MESSAGE("change alarm timer failed\r\n");

			if (get_queue_packet->alarm_status == 1)
			{
				status->x_tick = SENSOR_ALARM_POLLING_INTERVAL;

				PRINTF("start alarm timer\r\n");
				if (xTimerStart(status->alarm_timers, 0) != pdPASS)
					PRINTF("start alarm timer failed\r\n");

				if (!check_sensor_last_alarm(status, 5000))
				{
					if (status->last_uplink_status == ALERT)
					{
						if (!hbb_info.disengaged)
							send_uplink_event(rf_msg->sn_dest, UNALERT);
					}
					if (!hbb_info.disengaged)
						send_uplink_event(rf_msg->sn_dest, ALERT);
					status->last_uplink_status = ALERT;
					rf_msg->alarm_status = 1;
#ifndef WIFI_MODULE
					start_play_plan(AUDIO_ALARM_MAG);
#endif
				}
			}
			else
			{
				if (rf_msg->alarm_status == 1)
				{
					rf_msg->alarm_status = 0;
					status->qretry_count = 0;
					if (!hbb_info.disengaged)
						send_uplink_event(rf_msg->sn_dest, UNALERT);
					status->last_uplink_status = UNALERT;
				}
			}
			status->qretry_count = 0;
			status->last_sequence = get_queue_packet->sequence;
			status->last_ack_tick = xTaskGetTickCount();
			status->last_alarm_tick = xTaskGetTickCount();

			break;
		}
		default:
			DBG_MESSAGE("unexpected rf messsage received");
			return 1;
	}

	return 0;
}

int smart_pir_2_17_handle(SENSOR_STATUS_STORAGE_BLOCK *status, RF_MSG *get_queue_packet)
{
	RF_MSG* rf_msg = &status->send_message;

	switch(get_queue_packet->cmd)
	{
		case RF_CMD_GENERIC_REQUEST_ACK:
		{
			status->send_message.enable_data = get_queue_packet->enable_data;
			PRINTF("%s cmd=%d rf status %d current status %d\r\n", __func__, RF_CMD_GENERIC_REQUEST_ACK, get_queue_packet->alarm_status, rf_msg->alarm_status);
			//RF_MSG* rf_msg = get_msg(recv_packet.sn_mitt);

			rf_msg->fw_major = get_queue_packet->fw_major;
			rf_msg->fw_minor = get_queue_packet->fw_minor;
			rf_msg->alarm_status = get_queue_packet->alarm_status;
			status->last_ack_tick = xTaskGetTickCount();
			if (get_queue_packet->alarm_status == 0)
			{
				rf_msg->alarm_status = 0;
				if (status->x_tick == SENSOR_ALARM_POLLING_INTERVAL)
				{
					xTimerStop(status->alarm_timers, 0);
					if( xTimerChangePeriod( status->alarm_timers, SENSOR_POLLING_INTERVAL, 0) != pdPASS )
						DBG_MESSAGE("change alarm timer failed\r\n");

					status->x_tick = SENSOR_POLLING_INTERVAL;

					PRINTF("start polling timer\r\n");
					if (xTimerStart(status->alarm_timers, 0) != pdPASS)
						PRINTF("start alarm timer failed\r\n");
				}
			}

			rf_receive();
			break;
		}
		case RF_CMD_UNSOLICITED_STATUS:
		{
			PRINTF("%s cmd=%d rf status %d current status %d\r\n", __func__, RF_CMD_UNSOLICITED_STATUS, get_queue_packet->alarm_status, rf_msg->alarm_status);
			print_log("id=%x cmd=%d rf status %d current status %d\r\n", rf_msg->sn_dest, RF_CMD_UNSOLICITED_STATUS, get_queue_packet->alarm_status, rf_msg->alarm_status);

			//engage_status = 0;
			rf_msg->sn_dest = get_queue_packet->sn_mitt;
			rf_msg->cmd = RF_CMD_UNSOLICITED_STATUS_ACK;
			rf_msg->sequence = get_queue_packet->sequence;

			//test code add by ywt
			vTaskDelay(5);

			rf_send_then_receive_no_preamble(rf_msg);

			if (status->last_sequence != get_queue_packet->sequence)
			{
				status->last_uplink_status = ALERT;
				if (!hbb_info.disengaged)
					send_uplink_event(rf_msg->sn_dest, ALERT);

				status->last_sequence = get_queue_packet->sequence;
#ifndef WIFI_MODULE
				start_play_plan(AUDIO_ALARM_PIR);
#endif
				PRINTF("dis alarm\r\n");
				vTaskDelay(1);

				status->last_uplink_status = UNALERT;
				if (!hbb_info.disengaged)
					send_uplink_event(rf_msg->sn_dest, UNALERT);
			}
			status->qretry_count = 0;
			status->last_sequence = get_queue_packet->sequence;


			break;
		}
		case RF_CMD_CHANGE_LOW_SENSITIVITY_ACK:
			DBG_MESSAGE("RF_CMD_CHANGE_LOW_SENSITIVITY_ACK received");
			status->last_sequence = get_queue_packet->sequence;
			break;
		case RF_CMD_CHANGE_HIGH_SENSITIVITY_ACK:
			DBG_MESSAGE("RF_CMD_CHANGE_HIGH_SENSITIVITY_ACK received");
			status->last_sequence = get_queue_packet->sequence;
			break;
		default:
			DBG_MESSAGE("unexpected rf messsage received");
			return 1;
	}

	return 0;
}

int smart_lp_sensor_handle(SENSOR_STATUS_STORAGE_BLOCK *status, RF_MSG *get_queue_packet)
{
	RF_MSG* rf_msg = &status->send_message;

	switch(get_queue_packet->cmd)
	{
		case RF_CMD_HEARTBEAT:
		{
			PRINTF("%s cmd=%d rf status %d current status %d\r\n", __func__, RF_CMD_HEARTBEAT, get_queue_packet->alarm_status, rf_msg->alarm_status);
			vTaskDelay(10);

			rf_msg->fw_major = get_queue_packet->fw_major;
			rf_msg->fw_minor = get_queue_packet->fw_minor;

			rf_msg->sn_dest = get_queue_packet->sn_mitt;
			rf_msg->cmd = RF_CMD_HEARTBEAT_ACK;
			rf_msg->sequence = get_queue_packet->sequence;
			status->last_sequence = get_queue_packet->sequence;

			if (pending_todo_work(status) == 0)
				rf_send_then_receive_no_preamble(rf_msg);
			else
				do_todo_work(status);

			if (!check_sensor_last_tick(status, 60000 * 15))
			{
				send_uplink_event(rf_msg->sn_dest, INFO);
				status->last_ack_tick = xTaskGetTickCount();
			}

			rf_receive();
			break;
		}
		case RF_CMD_UNSOLICITED_STATUS:
		{
			vTaskDelay(5);

			PRINTF("%s cmd=%d rf status %d current status %d\r\n", __func__, RF_CMD_UNSOLICITED_STATUS, get_queue_packet->alarm_status, rf_msg->alarm_status);
			print_log("id=%x cmd=%d rf status %d current status %d\r\n", rf_msg->sn_dest, RF_CMD_UNSOLICITED_STATUS, get_queue_packet->alarm_status, rf_msg->alarm_status);

			rf_msg->sn_dest = get_queue_packet->sn_mitt;
			rf_msg->cmd = RF_CMD_UNSOLICITED_STATUS_ACK;
			rf_msg->sequence = get_queue_packet->sequence;


			rf_send_then_receive_no_preamble(rf_msg);
			DBG_MESSAGE("RF_CMD_UNSOLICITED_STATUS_ACK sent\r\n");

			if (get_queue_packet->alarm_status == 1)
			{
				if (!check_sensor_last_alarm(status, 5000))
				{
					if (status->last_uplink_status == ALERT)
					{
						if (!hbb_info.disengaged)
							send_uplink_event(rf_msg->sn_dest, UNALERT);
					}
					if (!hbb_info.disengaged)
						send_uplink_event(rf_msg->sn_dest, ALERT);
					status->last_uplink_status = ALERT;
					rf_msg->alarm_status = 1;
				}
			}
			else
			{
				if (rf_msg->alarm_status == 1)
				{
					rf_msg->alarm_status = 0;
					if (!hbb_info.disengaged)
						send_uplink_event(rf_msg->sn_dest, UNALERT);
					status->last_uplink_status = UNALERT;
				}
			}
			status->last_sequence = get_queue_packet->sequence;
			status->last_alarm_tick = xTaskGetTickCount();
			break;
		}
		case RF_CMD_GET_ATTRIBUTE_ACK:
		{
			RF_MSG_EXT *rf_packet_ext =  (RF_MSG_EXT *)get_queue_packet;
			PRINTF("%s cmd=%d index %d value %x\r\n", __func__, RF_CMD_GET_ATTRIBUTE_ACK,
					rf_packet_ext->index, __get_unaligned_le32((uint8_t *)&rf_packet_ext->value));

			update_todo_work(status, rf_packet_ext);

			if (pending_todo_work(status) == 0)
			{
				rf_msg->fw_major = get_queue_packet->fw_major;
				rf_msg->fw_minor = get_queue_packet->fw_minor;

				rf_msg->sn_dest = get_queue_packet->sn_mitt;
				rf_msg->cmd = RF_CMD_HEARTBEAT_ACK;
				rf_msg->sequence = get_queue_packet->sequence;
				rf_send_then_receive_no_preamble(rf_msg);
			}
			else
				do_todo_work(status);

			rf_receive();
			break;
		}
		case RF_CMD_SET_ATTRIBUTE_ACK:
		{
			RF_MSG_EXT *rf_packet_ext =  (RF_MSG_EXT *)get_queue_packet;

			PRINTF("%s cmd=%d index %d value %x\r\n", __func__, RF_CMD_SET_ATTRIBUTE_ACK,
					rf_packet_ext->index, __get_unaligned_le32((uint8_t *)&rf_packet_ext->value));

			update_todo_work(status, rf_packet_ext);

			if (pending_todo_work(status) == 0)
			{
				rf_msg->fw_major = get_queue_packet->fw_major;
				rf_msg->fw_minor = get_queue_packet->fw_minor;

				rf_msg->sn_dest = get_queue_packet->sn_mitt;
				rf_msg->cmd = RF_CMD_HEARTBEAT_ACK;
				rf_msg->sequence = get_queue_packet->sequence;
				rf_send_then_receive_no_preamble(rf_msg);
			}
			else
				do_todo_work(status);

			rf_receive();
			break;
		}
		default:
			DBG_MESSAGE("unexpected rf messsage received");
			return 1;
	}

	return 0;
}

int smart_remote_2_17_handle(SENSOR_STATUS_STORAGE_BLOCK *status, RF_MSG *get_queue_packet)
{
	RF_MSG* rf_msg = &status->send_message;

	switch(get_queue_packet->cmd)
	{
		case RF_CMD_PUSH_ACTIVATION:
		{
			PRINTF("%s cmd=%d rf status %d current status %d\r\n", __func__, RF_CMD_PUSH_ACTIVATION, get_queue_packet->alarm_status, rf_msg->alarm_status);
			//RF_MSG* rf_msg = get_msg(recv_packet.sn_mitt);

			if (!check_sensor_last_alarm(status, 3000))
			{

				hbb_info.disengaged = hbb_info.disengaged ^ 1;
				PRINTF("homebox disengaged %d\r\n", hbb_info.disengaged);
			}

			rf_msg->fw_major = get_queue_packet->fw_major;
			rf_msg->fw_minor = get_queue_packet->fw_minor;
			status->last_alarm_tick = xTaskGetTickCount();
			status->last_sequence = get_queue_packet->sequence;
			PRINTF("last_alarm_tick %d\r\n", status->last_alarm_tick);

			rf_receive();
			break;
		}
		default:
			DBG_MESSAGE("unexpected rf messsage received");
			return 1;
	}

	return 0;
}

int mono_water_smoke_2_17_handle(SENSOR_STATUS_STORAGE_BLOCK *status, RF_MSG *get_queue_packet)
{
	RF_MSG* rf_msg = &status->send_message;

	switch(get_queue_packet->cmd)
	{
		case RF_CMD_UNSOLICITED_STATUS:
		{
			PRINTF("RF_CMD_UNSOLICITED_STATUS received %d\r\n", get_queue_packet->alarm_status);

			status->last_sequence = get_queue_packet->sequence;
			status->last_ack_tick = xTaskGetTickCount();
			status->last_alarm_tick = xTaskGetTickCount();
			rf_msg->alarm_status = get_queue_packet->alarm_status;
			rf_msg->fw_major = get_queue_packet->fw_major;
			rf_msg->fw_minor = get_queue_packet->fw_minor;

			if (!check_sensor_last_tick(status, 30000))
			{
				if (rf_msg->alarm_status)
					send_uplink_event(rf_msg->sn_dest, ALERT);
				else
					send_uplink_event(rf_msg->sn_dest, UNALERT);
			}
			break;
		}

		default:
			DBG_MESSAGE("unexpected rf messsage received");
			return 1;
	}

	return 0;
}

int mono_mag_2_17_handle(SENSOR_STATUS_STORAGE_BLOCK *status, RF_MSG *get_queue_packet)
{
	RF_MSG* rf_msg = &status->send_message;

	switch(get_queue_packet->cmd)
	{
		case RF_CMD_UNSOLICITED_STATUS:
		{
			PRINTF("RF_CMD_UNSOLICITED_STATUS received %d\r\n", get_queue_packet->alarm_status);

			status->last_sequence = get_queue_packet->sequence;
			status->last_ack_tick = xTaskGetTickCount();
			status->last_alarm_tick = xTaskGetTickCount();
			rf_msg->alarm_status = get_queue_packet->alarm_status;
			rf_msg->fw_major = get_queue_packet->fw_major;
			rf_msg->fw_minor = get_queue_packet->fw_minor;

			if (!check_sensor_last_tick(status, 30000))
			{
				if (rf_msg->alarm_status)
					send_uplink_event(rf_msg->sn_dest, ALERT);
				else
					send_uplink_event(rf_msg->sn_dest, UNALERT);
			}
			break;
		}

		default:
			DBG_MESSAGE("unexpected rf messsage received");
			return 1;
	}

	return 0;
}

int mono_pir_2_17_handle(SENSOR_STATUS_STORAGE_BLOCK *status, RF_MSG *get_queue_packet)
{
	RF_MSG* rf_msg = &status->send_message;

	switch(get_queue_packet->cmd)
	{
		case RF_CMD_UNSOLICITED_STATUS:
		{
			PRINTF("RF_CMD_UNSOLICITED_STATUS received %d\r\n", get_queue_packet->alarm_status);

			status->last_sequence = get_queue_packet->sequence;
			status->last_ack_tick = xTaskGetTickCount();
			status->last_alarm_tick = xTaskGetTickCount();
			rf_msg->alarm_status = get_queue_packet->alarm_status;
			rf_msg->fw_major = get_queue_packet->fw_major;
			rf_msg->fw_minor = get_queue_packet->fw_minor;

			if (!check_sensor_last_tick(status, 30000))
			{
				if (rf_msg->alarm_status)
					send_uplink_event(rf_msg->sn_dest, ALERT);
				else
					send_uplink_event(rf_msg->sn_dest, UNALERT);
			}
			break;
		}

		default:
			DBG_MESSAGE("unexpected rf messsage received");
			return 1;
	}

	return 0;
}

int siren_3_5_query(SENSOR_STATUS_STORAGE_BLOCK *status, RF_MSG *get_queue_packet)
{

	switch(get_queue_packet->cmd)
	{

	}

	return 0;
}

int actuactor_3_5_handle(SENSOR_STATUS_STORAGE_BLOCK *status, RF_MSG *get_queue_packet)
{

	switch(get_queue_packet->cmd)
	{

	}

	return 0;
}

int generic_3_5_handle(SENSOR_STATUS_STORAGE_BLOCK *status, RF_MSG *get_queue_packet)
{

	switch(get_queue_packet->cmd)
	{

	}

	return 0;
}

int consumption_3_5_handle(SENSOR_STATUS_STORAGE_BLOCK *status, RF_MSG *get_queue_packet)
{
	RF_MSG* rf_msg = &status->send_message;

	switch(get_queue_packet->cmd)
	{
		case RF_CMD_GENERIC_REQUEST_ACK2:
		{
			PRINTF("RF_CMD_GENERIC_REQUEST_ACK received %d\r\n", rf_msg->alarm_status);

			status->last_ack_tick = xTaskGetTickCount();
			rf_msg->alarm_status = get_queue_packet->alarm_status;
			rf_msg->fw_major = get_queue_packet->fw_major;
			rf_msg->fw_minor = get_queue_packet->fw_minor;

			rf_receive();
			break;
		}
		case RF_CMD_UNSOLICITED_STATUS2:
		{
			PRINTF("RF_CMD_UNSOLICITED_STATUS received %d\r\n", rf_msg->alarm_status);

			rf_msg->sn_dest = get_queue_packet->sn_mitt;
			rf_msg->cmd = RF_CMD_UNSOLICITED_STATUS_ACK2;
			rf_msg->sequence = get_queue_packet->sequence;


			rf_send_then_receive_no_preamble(rf_msg);
			DBG_MESSAGE("RF_CMD_UNSOLICITED_STATUS_ACK sent\r\n");

			xTimerStop(status->alarm_timers, 0);
			if( xTimerChangePeriod( status->alarm_timers, SENSOR_ALARM_POLLING_INTERVAL, 0) != pdPASS )
					DBG_MESSAGE("change alarm timer failed\r\n");

			status->x_tick = SENSOR_ALARM_POLLING_INTERVAL;

			PRINTF("start alarm timer %d\r\n", status->x_tick);
			if (xTimerStart(status->alarm_timers, 0) != pdPASS)
				PRINTF("start alarm timer failed\r\n");

			if (rf_msg->alarm_status == 0)
			{
				send_uplink_event(rf_msg->sn_dest, ALERT);

				rf_msg->alarm_status = 1;
			}
			status->qretry_count = 0;
			status->last_sequence = get_queue_packet->sequence;

			break;
		}
		default:
			DBG_MESSAGE("unexpected rf messsage received");
			return 1;
	}

	return 0;
}

#define MK_SENSOR_TYPE_CORE(_name, _type, _version,_init, _handle, _callback, _query) \
{ 															   \
	.name = _name,                                             \
	.devicetype = _type,											   \
	.protover = _version,									   \
	.init = _init,											   \
	.handle_rf_message = _handle,							   \
	.receive_callback = _callback,                             \
	.query = _query,									   \
}

#define MK_SENSOR_TYPE(_name, _type, _version, _handle) \
		MK_SENSOR_TYPE_CORE(_name, _type, _version, NULL, _handle, NULL, NULL)

struct sensor_type type_array[] =
{
	MK_SENSOR_TYPE("water", DEVICE_TYPE(SENSOR_GROUP, WATER_SENSOR, 1), PROTOCOL_217, smart_water_smoke_2_17_handle),
	MK_SENSOR_TYPE("smoke", DEVICE_TYPE(SENSOR_GROUP, SMOKE_SENSOR, 1), PROTOCOL_217, smart_water_smoke_2_17_handle),
	MK_SENSOR_TYPE("magnetic", DEVICE_TYPE(SENSOR_GROUP, MAGNETIC_SENSOR, 1), PROTOCOL_217, smart_mag_2_17_handle),
	MK_SENSOR_TYPE("pir", DEVICE_TYPE(SENSOR_GROUP, PIR_SENSOR, 1), PROTOCOL_217, smart_pir_2_17_handle),
	MK_SENSOR_TYPE("water-lp", DEVICE_TYPE(SENSOR_GROUP, WATER_SENSOR, 3), PROTOCOL_217, smart_lp_sensor_handle),
	MK_SENSOR_TYPE("smoke-lp", DEVICE_TYPE(SENSOR_GROUP, SMOKE_SENSOR, 3), PROTOCOL_217, smart_lp_sensor_handle),
	MK_SENSOR_TYPE("magnetic-lp", DEVICE_TYPE(SENSOR_GROUP, MAGNETIC_SENSOR, 3), PROTOCOL_217, smart_lp_sensor_handle),
	MK_SENSOR_TYPE("pir-lp", DEVICE_TYPE(SENSOR_GROUP, PIR_SENSOR, 3), PROTOCOL_217, smart_lp_sensor_handle),
	MK_SENSOR_TYPE("remote", DEVICE_TYPE(SENSOR_GROUP, SENSOR_TYPE_REMOTE, 1), PROTOCOL_217, smart_remote_2_17_handle),
	MK_SENSOR_TYPE("gas", DEVICE_TYPE(SENSOR_GROUP, SENSOR_TYPE_GAS, 1), PROTOCOL_217, smart_water_smoke_2_17_handle),
	MK_SENSOR_TYPE("carbon monoxide", DEVICE_TYPE(SENSOR_GROUP, SENSOR_TYPE_CO, 1), PROTOCOL_217, smart_water_smoke_2_17_handle),
	MK_SENSOR_TYPE("temperature", DEVICE_TYPE(SENSOR_GROUP, SENSOR_TYPE_TEMPERATURE, 1), PROTOCOL_35, NULL),
	MK_SENSOR_TYPE_CORE("siren", DEVICE_TYPE(SENSOR_GROUP, SENSOR_TYPE_SIREN, 1), PROTOCOL_35, NULL, generic_3_5_handle, NULL, siren_3_5_query),
	MK_SENSOR_TYPE_CORE("actuactor", DEVICE_TYPE(SENSOR_GROUP, SENSOR_TYPE_ACTUATOR, 1), PROTOCOL_35, NULL, generic_3_5_handle, NULL, siren_3_5_query),
	MK_SENSOR_TYPE_CORE("consumption", DEVICE_TYPE(SENSOR_GROUP, SENSOR_TYPE_CONSUMPTION, 1), PROTOCOL_35, NULL, consumption_3_5_handle, NULL, siren_3_5_query),
};


int get_sensor_type_handle(SENSOR_STATUS_STORAGE_BLOCK *status, uint32_t dt)
{
	int i;;

	for (i = 0; i < sizeof(type_array)/sizeof(type_array[0]); i++)
	{
		PRINTF("dt %x  devicetype %x\r\n", dt, type_array[i].devicetype);
		if (type_array[i].devicetype == dt)
		{
			status->type_handle = &type_array[i];
			return 0;
		}
	}

	PRINTF("can not find type handle\r\n");
	return -1;
}
