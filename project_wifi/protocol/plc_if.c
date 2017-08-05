#define PR_MODULE "plc\1"

#include "uart_plc.h"
#include "rtc_fun.h"
#include "task.h"
#include "hb_protocol.h"
#include "protocol_2_17.h"
#include "fota.h"
#include "rtc_fun.h"
#include "spiflash_config.h"
#include "version.h"
/*******************************************************************************
 * API
 ******************************************************************************/
#ifdef PLC
#ifdef BRIDGE
void sent_plc_event_msg(uint8_t plc_cmd,uint32_t device_type,uint32_t sid,uint32_t seq,uint32_t data)
{
	PLC_MSG plc_msg_sent = {0};
	plc_msg_sent.bridge_sw = ver_info.swver;
	plc_msg_sent.bridge_hw = ver_info.hwver;
	plc_msg_sent.header = PLC_MSG_HEADER;
	plc_msg_sent.sn_mitt = BRIDGE_ID;
	plc_msg_sent.cmd = plc_cmd;
	plc_msg_sent.device_type = device_type;
	plc_msg_sent.sn_dest= hbb_info.config.master;
	plc_msg_sent.sn_sensor=sid;
	get_current_systime(&plc_msg_sent.time_stamp);
	plc_msg_sent.sequence = seq;
	plc_msg_sent.data = data;
	plc_msg_sent.crc = crc_cal((uint8_t*)&plc_msg_sent.retry,PLC_PACKAGE_LEN-4);
	xQueueSend(plc_msg_queue,&plc_msg_sent, portMAX_DELAY);
}
void sync_rtc_time_ack(uint32_t data,uint32_t seq)
{
	sent_plc_event_msg(PLC_CMD_GENERIC_REQUEST_ACK,0,0,seq,RTC_SYNC);
}

void query_bridge_sensor_ack(uint32_t data,uint32_t seq)
{
	sent_plc_event_msg(PLC_CMD_GENERIC_REQUEST_ACK,0,0,seq,GENERIC_REQUEST);
}

void plc_alarm_happened(uint32_t sid,uint32_t seq)
{
	sent_plc_event_msg(PLC_CMD_ALARM,0,sid,seq,0);
}

void plc_alarm_cleared(uint32_t sid,uint32_t seq)
{
	sent_plc_event_msg(PLC_CMD_ALARM_CLEAR,0,sid,seq,0);
}

void plc_toggle_bridge_sensor_ack(uint32_t data,uint32_t sid,uint32_t seq)
{
	sent_plc_event_msg(PLC_CMD_TOGGLE_ACK,0,sid,seq,data);
}

void set_bridge_pir_sens_ack(uint32_t data,uint32_t sid,uint32_t seq)
{
	sent_plc_event_msg(PLC_CMD_SET_PIR_SENSITIVITY_ACK,0,sid,seq,data);
}

void sent_sensor_list_ack(uint32_t seq)
{
	sent_plc_event_msg(PLC_CMD_SENT_SENSOR_LIST_ACK,0,0,seq,0);
}

void bridge_get_sensor_list(uint32_t seq)
{
	sent_plc_event_msg(PLC_CMD_REQUEST_SENSOR_LIST,0,0,seq,0);
}

void deploy_sensor_list_ack(uint32_t seq)
{
	sent_plc_event_msg(PLC_CMD_DEPLOY_SENSOR_LIST_ACK,0,0,seq,0);
}

void plc_fw_need_update_ack(uint32_t seq)
{
	sent_plc_event_msg(PLC_CMD_UPDATE_FW_ACK,0,0,seq,0);
}

void plc_request_img_block(uint32_t device_type,uint32_t data,uint32_t seq)
{
	sent_plc_event_msg(PLC_CMD_REQUEST_IMG_BLOCK,device_type,0,seq,data);
}

void plc_fw_update_result(uint32_t device_type,uint32_t data,uint32_t seq)
{
	sent_plc_event_msg(PLC_CMD_FW_UPDATE_RESULT,device_type,0,seq,data);
}


#else
void sent_plc_event_msg(uint8_t plc_cmd,uint32_t device_type,uint32_t bridge_id,uint32_t sid,uint32_t seq,uint32_t data)
{
	PLC_MSG plc_msg_sent = {0};
	plc_msg_sent.header = PLC_MSG_HEADER;
	plc_msg_sent.sn_mitt = hbb_info.config.id;
	plc_msg_sent.cmd = plc_cmd;
	plc_msg_sent.device_type = device_type;
	plc_msg_sent.sn_dest=bridge_id;
	plc_msg_sent.sn_sensor=sid;
	get_current_systime(&plc_msg_sent.time_stamp);
	plc_msg_sent.sequence = seq;
	plc_msg_sent.data = data;
	plc_msg_sent.crc = crc_cal((uint8_t*)&plc_msg_sent.retry,PLC_PACKAGE_LEN-4);
	xQueueSend(plc_msg_queue,&plc_msg_sent, portMAX_DELAY);
}

void sync_bridge_rtc_time(uint32_t bridge_id,uint32_t seq)
{
	sent_plc_event_msg(PLC_CMD_GENERIC_REQUEST,0,bridge_id,0,seq,RTC_SYNC);
}

void query_bridge_sensor(uint32_t bridge_id,uint32_t seq)
{
	sent_plc_event_msg(PLC_CMD_GENERIC_REQUEST,0,bridge_id,0,seq,GENERIC_REQUEST);
}

void disengage_bridge_sensor(uint32_t bridge_id,uint32_t sid,uint32_t seq)
{
	sent_plc_event_msg(PLC_CMD_TOGGLE,0,bridge_id,sid,seq,DISENGAGE);
}

void engage_bridge_sensor(uint32_t bridge_id,uint32_t sid,uint32_t seq)
{
	sent_plc_event_msg(PLC_CMD_TOGGLE,0,bridge_id,sid,seq,ENGAGE);
}

void set_bridge_pir_low_sens(uint32_t bridge_id,uint32_t sid,uint32_t seq)
{
	sent_plc_event_msg(PLC_CMD_SET_PIR_SENSITIVITY,0,bridge_id,sid,seq,LOW_SENSITIVITY);
}

void set_bridge_pir_high_sens(uint32_t bridge_id,uint32_t sid,uint32_t seq)
{
	sent_plc_event_msg(PLC_CMD_SET_PIR_SENSITIVITY,0,bridge_id,sid,seq,HIGH_SENSITIVITY);
}

void sent_sensor_list_to_bridge(uint32_t bridge_id,uint32_t seq)
{
	sent_plc_event_msg(PLC_CMD_SENT_SENSOR_LIST,0,bridge_id,0,seq,0);
}

void deploy_sensor_list_to_bridge(uint32_t bridge_id,uint32_t seq)
{
	sent_plc_event_msg(PLC_CMD_DEPLOY_SENSOR_LIST,0,bridge_id,0,seq,0);
}

void plc_fw_need_update(uint32_t bridge_id,uint32_t device_type,uint32_t seq)
{
	sent_plc_event_msg(PLC_CMD_UPDATE_FW,device_type,bridge_id,0,seq,0);
}

void plc_request_img_block_ack(uint32_t bridge_id,uint32_t device_type,uint32_t data,uint32_t seq)
{
	uint32_t fota_img_store_addr;
	if(BRIDGE_GROUP == DEVICE_TYPE_Group(device_type))
	{
		fota_img_store_addr = FOTA_BRIDGE_ADDR_START;
	}
	else if(SENSOR_GROUP == DEVICE_TYPE_Group(device_type))
	{
		fota_img_store_addr = FOTA_SENSOR_IMG_ADDR_START;
	}
	else
	{
		PRINTF("ERR : Wrong fota request device\r\n");
		return;
	}
	norflash_read_data_ll(&flash_master_rtos_handle, fota_img_store_addr+FOTA_IMG_CELL_DOWN_LEN*data, FOTA_IMG_CELL_DOWN_LEN,&plc_sent_fw_block.data);
	sent_plc_event_msg(PLC_CMD_REQUEST_IMG_BLOCK_ACK,device_type,bridge_id,0,seq,data);
}
#endif
/********************************************************************************/


#ifdef BRIDGE
void check_sensor_rssi(void)
{
	err_t err;
	TickType_t		xLastWakeTime;
	RF_MSG rf_msg_query;
	rf_msg_query.cmd = RF_CMD_GENERIC_REQUEST;

//for test
//	uint64_t timestamp;
//	get_current_systime(&timestamp);

	for (int j=0; j<6; j++)
	{
//		PRINTF("BEFORE , %d:%d\r\n",(uint32_t)(timestamp / 1000), (uint32_t)(timestamp % 1000));
		xLastWakeTime = xTaskGetTickCount();
		for (int i=0; i<MAX_SENSOR_COUNT; i++)
		{
			if( sensor_status[i].send_message.en)
			{
				if (!check_sensor_last_tick(&sensor_status[i], 5000))
				{
					err = hb_do_rf_send_callback(sensor_status[i].send_message.sn_dest, &rf_msg_query);
				}
			}
		}
//		get_current_systime(&timestamp);
//		PRINTF("AFTER , %d:%d\r\n",(uint32_t)(timestamp / 1000), (uint32_t)(timestamp % 1000));
		if(!err)
		{
			break;
		}
	}
}


void do_toggle(uint32_t sid,uint8_t toggle_data)
{
	RF_MSG rf_msg_query;
	TickType_t xLastWakeTime = xTaskGetTickCount();
	rf_msg_query.cmd = RF_CMD_GENERIC_REQUEST;
	rf_msg_query.enable_cmd = 1;
	rf_msg_query.enable_data = toggle_data;

	hb_do_rf_send_callback(sid, &rf_msg_query);
	vTaskDelayUntil(&xLastWakeTime,100);
}

void do_set_pir_sensitivity(uint32_t sid,uint8_t data)
{
	RF_MSG rf_msg_query;
	TickType_t xLastWakeTime = xTaskGetTickCount();
	//todo
	if(LOW_SENSITIVITY == data)
	{
		rf_msg_query.cmd = RF_CMD_CHANGE_LOW_SENSITIVITY;
	}else
	{
		rf_msg_query.cmd = RF_CMD_CHANGE_HIGH_SENSITIVITY;
	}
	hb_do_rf_send_callback(sid, &rf_msg_query);
	vTaskDelayUntil(&xLastWakeTime,500);
}

void plc_bridge_task(void *pvParameters)
{
	PLC_MSG plc_bridge_msg;
	int idx;
	memset(&plc_bridge_msg,0x00,sizeof(plc_bridge_msg));
	while(1)
	{
		if(xQueueReceive(plc_to_bridge_queue,&plc_bridge_msg,portMAX_DELAY))
		{
			switch(plc_bridge_msg.cmd)
			{
				case PLC_CMD_GENERIC_REQUEST:
					check_sensor_status();
 					query_bridge_sensor_ack(GENERIC_REQUEST,plc_bridge_msg.sequence);
					break;
				case PLC_CMD_SENT_SENSOR_LIST:
					update_sensor_route();
					check_all_sensor_flag = true;
					check_sensor_rssi();
					check_all_sensor_flag = false;
					sent_sensor_list_ack(plc_bridge_msg.sequence);
					break;
				case PLC_CMD_TOGGLE:
					do_toggle(plc_bridge_msg.sn_sensor,(uint8_t)plc_bridge_msg.data);
					idx = sensor_get_index_id(plc_bridge_msg.sn_sensor);
					plc_toggle_bridge_sensor_ack(sensor_status[idx].send_message.enable_data,plc_bridge_msg.sn_sensor,plc_bridge_msg.sequence);
					break;
				case PLC_CMD_SET_PIR_SENSITIVITY:
					do_set_pir_sensitivity(plc_bridge_msg.sn_sensor,(uint8_t)plc_bridge_msg.data);
					idx = sensor_get_index_id(plc_bridge_msg.sn_sensor);
					set_bridge_pir_sens_ack(plc_bridge_msg.data/*need a structure :pir sensitivity*/,plc_bridge_msg.sn_sensor,plc_bridge_msg.sequence);
					//todo
					break;
				default:
					break;
			}
		}
	}

}
#endif
#endif
