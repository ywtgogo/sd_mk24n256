#ifdef PLC
#define PR_MODULE "m:plc,"
#include "fsl_uart_freertos.h"
#include "fsl_debug_console.h"
#include "clock_config.h"
#include "fsl_uart.h"
#include "fsl_port.h"
#include "pin_mux.h"
#include "uart_plc.h"
#include "rtc_fun.h"
#include "board.h"
#include "message.h"
#include "version.h"
#include "plc_k24_api.h"
#ifndef BRIDGE
#include "uplink_protocol.h"
#endif
//for test
#include "plc_if.h"
/*******************************************************************************
 * Definitions
 ******************************************************************************/

/* Task priorities. */
#define PLC_UART_PRIORITY 5

/*******************************************************************************
 * Variables
 ******************************************************************************/



uint32_t plc_local_seq;
uint8_t plc_background_buffer[5000];
uart_rtos_handle_t PLC_handle;
struct _uart_handle PLC_t_handle;
size_t temp_recved;
PLC_MSG plc_send_message;
PLC_MSG_LINK_NODE *plc_retry_header = NULL;
TimerHandle_t plc_retry_timers;
#ifndef BRIDGE
PLC_FW_BLOCK plc_sent_fw_block;
#endif

PLC_SENSOR_LIST plc_sent_senosr_list;
PLC_SENSOR_LIST plc_recv_sensor_list;

uint32_t seq_record[MAX_SENSOR_COUNT];

PLC_MSG plc_rev_message;
PLC_FW_BLOCK plc_recv_fw_block;

SemaphoreHandle_t plc_fota_block_sem;
SemaphoreHandle_t plc_sensor_query_back_sem;
SemaphoreHandle_t plc_slist_back_sem;
EventGroupHandle_t BridgeFwReqAckEventGroup = NULL;

bool wait_plc_data_timeout_flag = false;

struct rtos_uart_config plc_uart_config = {
    .baudrate = 115200,
    .parity = kUART_ParityDisabled,
    .stopbits = kUART_OneStopBit,
    .buffer = plc_background_buffer,
    .buffer_size = sizeof(plc_background_buffer),
};


uint32_t  plc_uart_init()
{
	PRINTF("PLC UART initialization\r\n");
	uint32_t ret = 0;

	plc_uart_config.srcclk = CLOCK_GetFreq(PLC_UART_CLKSRC);
	plc_uart_config.base = PLC_UART;
    NVIC_SetPriority(PLC_UART_RX_TX_IRQn, PLC_UART_PRIORITY);
    if (0 < UART_RTOS_Init(&PLC_handle, &PLC_t_handle, &plc_uart_config))
    {
    	PRINTF("PLC Error : UART initialization failed.\r\n");
        ret = 1;
    }
    return ret;
}

void plcalarmCallback( TimerHandle_t xTimer )
{
	send_retry_msg();
}

void plc_info_init()
{
	plc_local_seq = 0;
	memset(&plc_send_message,0x00,sizeof(plc_send_message));
#ifndef BRIDGE
	plc_send_message.sn_mitt = HBB_ID;
#else
//for test
	plc_send_message.sn_dest = hbb_info.config.master;
	plc_send_message.sn_mitt = BRIDGE_ID;
#endif
	plc_send_message.header = PLC_MSG_HEADER;
	plc_sent_senosr_list.header = PLC_SENSOR_LIST_HEADER;
#ifndef BRIDGE
	plc_sent_fw_block.header = PLC_FW_BLOCK_HEADER;
#endif

	plc_retry_timers = xTimerCreate
    ( /* Just a text name, not used by the RTOS
    	kernel. */
    	"plc_timer",
		/* The timer period in ticks, must be
        greater than 0. */
		PLC_ALARM_POLLING_INTERVAL,
        /* The timers will auto-reload themselves
        when they expire. */
        pdFALSE,
        /* The ID is used to store a count of the
        number of times the timer has expired, which
        is initialised to 0. */
        ( void * ) 0,
        /* Each timer calls the same callback when
        it expires. */
	    plcalarmCallback
      );
}

void send_retry_msg()
{
	PLC_MSG_LINK_NODE *search = plc_retry_header;

	while(search)
	{
		send_plc_msg((uint8_t*)(&(search->msg)),PLC_PACKAGE_LEN);
		PRINTF("PLC %s sent : %x\r\n",search->msg.cmd == PLC_CMD_ALARM?"alarm":"alarm clear",search->msg.sn_sensor);
		search = search->next;
	}
}
void start_plc_alarm_timer(PLC_MSG *plc_message)
{
	PLC_MSG_LINK_NODE *new_msg = pvPortMalloc(sizeof(PLC_MSG_LINK_NODE));
	if (new_msg == NULL)
		return 0;
	memcpy(new_msg,plc_message,sizeof(PLC_MSG));
	new_msg->next = NULL;

	if(!plc_retry_header)
	{
		plc_retry_header = new_msg;
	}
	else
	{
		PLC_MSG_LINK_NODE *tmp = plc_retry_header;
		while(tmp->next)
		{
			tmp = tmp->next;
		}
		tmp->next = new_msg;
	}

	send_retry_msg();
}

void move_plc_retry_msg(PLC_MSG *plc_message)
{

	if(plc_retry_header->msg.sequence == plc_message->sequence)
	{
		PLC_MSG_LINK_NODE *free = plc_retry_header;
		plc_retry_header=plc_retry_header->next;
		vPortFree(free);
		if(!plc_retry_header)
		{
			xTimerStop(plc_retry_timers, 0);
		}
		return;
	}

	PLC_MSG_LINK_NODE *search = plc_retry_header->next;
	while(search)
	{
		if(search->msg.sequence == plc_message->sequence)
		{
			plc_retry_header->next = search->next;
			vPortFree(search);
			return;
		}
		search = search->next;
	}
}
/* uart send api
void send_plc_msg(uint8_t *plc_message,uint32_t length)
{
	if (kStatus_Success != UART_RTOS_Send(&PLC_handle,plc_message,length))
	{
		PRINTF("PLC ERR : PLC SENT MESSAGE FAILED\r\n");
	}
	else
	{
//		PRINTF("PLC SENT\r\n");
//		if (length>50) return;
//		for(uint32_t i = 0; i<length;i++ )
//			PRINTF(", %x",*(plc_message+i));
	}
}
*/
void get_then_sent_slist()
{
	for(uint32_t i=0; i<MAX_SENSOR_COUNT; i++)
	{
		memcpy((uint8_t *)&plc_sent_senosr_list.sensor_info[i].msg, (uint8_t *)&sensor_status[i].send_message, sizeof(RF_MSG));
	}
	plc_sent_senosr_list.crc = crc_cal((uint8_t*)&plc_sent_senosr_list.sensor_info,PLC_SENSOR_LIST_LEN-4);

	send_plc_msg((uint8_t *)&plc_sent_senosr_list,PLC_SENSOR_LIST_LEN);
}

void update_sensor_route()
{
	for(uint32_t i=0; i<MAX_SENSOR_COUNT; i++)
	{
		memcpy((uint8_t *)&sensor_status[i].send_message, (uint8_t *)&plc_recv_sensor_list.sensor_info[i].msg, sizeof(RF_MSG));

		if(!sensor_status[i].send_message.en)
			continue;

		sensor_status[i].device_type = sid_to_devicetype(sensor_status[i].send_message.sn_dest);
		if(sensor_status[i].device_type == 0)
		{
			PRINTF("ERR : sensor %x install dt err");
		}
		if (get_sensor_type_handle(&sensor_status[i], sensor_status[i].send_message.sn_dest))
		{
			PRINTF("ERR : sensor %x install hd err");
		}
	}
}

void update_plc_sensor_status()
{
	for(uint32_t i=0; i<MAX_SENSOR_COUNT; i++)
	{
		if((plc_recv_sensor_list.sensor_info[i].msg.sn_dest == sensor_status[i].send_message.sn_dest) &&
			sensor_status[i].send_message.en &&
			plc_recv_sensor_list.sensor_info[i].msg.en)
		{
			memcpy((uint8_t *)&sensor_status[i].send_message, (uint8_t *)&plc_recv_sensor_list.sensor_info[i].msg, sizeof(RF_MSG));
		}
	}
}


void package_plc_msg(uint8_t plc_cmd,uint32_t device_type,uint32_t bridge_id,uint32_t sid,uint32_t seq,uint32_t data)
{
	plc_send_message.bridge_sw = ver_info.swver;
	plc_send_message.bridge_hw = ver_info.hwver;
	plc_send_message.header = PLC_MSG_HEADER;
	plc_send_message.cmd = plc_cmd;
	plc_send_message.device_type = device_type;
#ifndef BRIDGE
	plc_send_message.sn_dest = bridge_id;
#else
	plc_send_message.sn_dest = hbb_info.config.master;
#endif
	plc_send_message.sn_sensor=sid;
	get_current_systime(&plc_send_message.time_stamp);
	plc_send_message.sequence = seq;
	plc_send_message.data = data;
	plc_send_message.crc = crc_cal((uint8_t*)&plc_send_message.retry,PLC_PACKAGE_LEN-4);
}

void deal_PLC_CMD_GENERIC_REQUEST(PLC_MSG *plc_message)
{
#ifdef BRIDGE
	if(RTC_SYNC == plc_message->data)
	{
		PRINTF("PLC SET RTC = %d",(uint32_t)(plc_message->time_stamp /1000));
		set_rtc_date(plc_message->time_stamp);
		package_plc_msg(PLC_CMD_GENERIC_REQUEST_ACK,0,0,plc_message->sn_sensor,plc_message->sequence,0);
		send_plc_msg((uint8_t*)&plc_send_message,PLC_PACKAGE_LEN);
		PRINTF("PLC tx rtc sync ack to %x\r\n",plc_send_message.sn_dest);
	}
	else
	{
		PRINTF("PLC rx query\r\n");
		xQueueSend(plc_to_bridge_queue,plc_message, portMAX_DELAY);
	}
#else

	PRINTF("PLC %s plc-bridge : %x\r\n",plc_message->data == RTC_SYNC?"RTC SYNC":"query" ,plc_message->sn_dest);
	send_plc_msg((uint8_t*)plc_message,PLC_PACKAGE_LEN);
#endif
}

void deal_PLC_CMD_GENERIC_REQUEST_ACK(PLC_MSG *plc_message)
{
#ifdef BRIDGE
	PRINTF("PLC tx query ack\r\n");
	send_plc_msg((uint8_t*)plc_message,PLC_PACKAGE_LEN);
	PRINTF("PLC tx slist \r\n");
	get_then_sent_slist();
#else
	if(GENERIC_REQUEST == plc_message->data)
	{
		PRINTF("PLC rx query ack from %x\r\n",plc_message->sn_mitt);
		update_plc_sensor_status();
		//todo mul bridge
		bridge_list[0].hw = plc_message->bridge_hw;
		bridge_list[0].sw = plc_message->bridge_sw;
		xSemaphoreGive(plc_sensor_query_back_sem);
	}
	else
	{
		PRINTF("PLC rtc sync from %x\r\n",plc_message->sn_mitt);
	}
#endif
}

void deal_ALARM(PLC_MSG *plc_message)
{
#ifdef BRIDGE
	start_plc_alarm_timer(plc_message);
#else
	PRINTF("PLC rx alarm  %x\r\n", plc_message->sn_sensor);
	package_plc_msg(PLC_CMD_ALARM_ACK,0, plc_message->sn_mitt,plc_message->sn_sensor,plc_message->sequence,0);
	send_plc_msg((uint8_t*)&plc_send_message,PLC_PACKAGE_LEN);
	PRINTF("PLC tx alarm ack to %x\r\n",plc_send_message.sn_sensor);

	send_uplink_event(plc_message->sn_sensor, ALERT);

#endif
}

void deal_UNALARM(PLC_MSG *plc_message)
{
#ifdef BRIDGE
	start_plc_alarm_timer(plc_message);
#else
	PRINTF("PLC alarm Clear receive from plc-sensor : %x\r\n",plc_message->sn_sensor);
	package_plc_msg(PLC_CMD_ALARM_CLEAR_ACK,0,plc_message->sn_mitt,plc_message->sn_sensor,plc_message->sequence,0);
	send_plc_msg((uint8_t*)&plc_send_message,PLC_PACKAGE_LEN);
	PRINTF("PLC alarm ack send to plc-sensor : %x\r\n",plc_send_message.sn_sensor);

	send_uplink_event(plc_message->sn_sensor, UNALERT);

#endif
}

void deal_ALARM_UNALARM_ACK(PLC_MSG *plc_message)
{
	PRINTF("PLC %s recv : %x\r\n",plc_message->cmd==PLC_CMD_ALARM_ACK?"alarm ack":"alarm clear ack", plc_message->sn_sensor);
	move_plc_retry_msg(plc_message);
}

void deal_TOGGLE(PLC_MSG *plc_message)
{
#ifdef BRIDGE
	PRINTF("PLC recv :%s sensor: %x\r\n",plc_message->data==DISENGAGE?"disengage":"engage", plc_message->sn_sensor);
	xQueueSend(plc_to_bridge_queue,plc_message, portMAX_DELAY);
#else
	PRINTF("PLC set sensor %x to %s\r\n", plc_message->sn_sensor,plc_message->data==DISENGAGE?"disengage":"engage");
	send_plc_msg((uint8_t*)plc_message,PLC_PACKAGE_LEN);
#endif
}

void deal_TOGGLE_ACK(PLC_MSG *plc_message)
{
#ifdef BRIDGE
	PRINTF("PLC %x sensor is %s\r\n", plc_message->sn_sensor,plc_message->data==DISENGAGE?"disengage":"engage");
	send_plc_msg((uint8_t*)plc_message,PLC_PACKAGE_LEN);
#else
	int idx = sensor_get_index_id(plc_message->sn_sensor);
	SENSOR_STATUS_STORAGE_BLOCK *status;
	status = &sensor_status[idx];

	if(plc_message->data == status->send_message.enable_data)
	{
		PRINTF("PLC Toggle sensor %x successfully, sensor is %s\r\n",plc_message->sn_sensor,plc_message->data>0?"engaged":"disengaged");
	}
	else
	{
		PRINTF("PLC Toggle sensor %x failed, sensor is %s\r\n",plc_message->sn_sensor,plc_message->data>0?"engaged":"disengaged");
	}
#endif
}

void deal_PLC_CMD_SET_PIR_SENSITIVITY(PLC_MSG *plc_message)
{
#ifdef BRIDGE
	PRINTF("PLC recv :set pir sensor %x to %s sensitivity\r\n", plc_message->sn_sensor,plc_message->data==LOW_SENSITIVITY?"low":"high");
	xQueueSend(plc_to_bridge_queue,plc_message, portMAX_DELAY);
#else
	PRINTF("PLC sent :set pir sensor %x to %s sensitivity\r\n", plc_message->sn_sensor,plc_message->data==LOW_SENSITIVITY?"low":"high");
	send_plc_msg((uint8_t*)plc_message,PLC_PACKAGE_LEN);
#endif
}

void deal_PLC_CMD_SET_PIR_SENSITIVITY_ACK(PLC_MSG *plc_message)
{
#ifdef BRIDGE
	PRINTF("PLC set pir sensitivity ack\r\n");
	send_plc_msg((uint8_t*)plc_message,PLC_PACKAGE_LEN);
#else
	PRINTF("PLC set pir sensitivity success!\r\n");
#endif
}

void deal_PLC_CMD_SENT_SENSOR_LIST(PLC_MSG *plc_message)
{
#ifdef BRIDGE
	PRINTF("PLC rx sLIST\r\n");
	if(plc_message->sn_mitt != hbb_info.config.master )
	{
		hbb_info.config.master= plc_message->sn_mitt;
		sync_hbb_info();
	}

	xQueueSend(plc_to_bridge_queue,plc_message, portMAX_DELAY);
#else
	PRINTF("PLC tx slist to %x\r\n", plc_message->sn_dest);
	send_plc_msg((uint8_t*)plc_message,PLC_PACKAGE_LEN);
	get_then_sent_slist();
#endif
}

void deal_PLC_CMD_SENT_SENSOR_LIST_ACK(PLC_MSG *plc_message)
{
#ifdef BRIDGE
	PRINTF("PLC sent sensor list ack\r\n");
	send_plc_msg((uint8_t*)plc_message,PLC_PACKAGE_LEN);
	get_then_sent_slist();
#else
	PRINTF("PLC rx slist ack from %x\r\n", plc_message->sn_mitt);
	update_sensor_route();
	xSemaphoreGive(plc_slist_back_sem);
#endif
}

void deal_PLC_CMD_DEPLOY_SENSOR_LIST(PLC_MSG *plc_message)
{
#ifdef BRIDGE
	PRINTF("PLC rx deploy slist\r\n");
	update_sensor_route();
	xEventGroupSetBits(hbbeventgroup, HBB_GETLIST_EVENT);

	if(plc_message->sn_mitt != hbb_info.config.master )
	{
		hbb_info.config.master= plc_message->sn_mitt;
		sync_hbb_info();
	}

	package_plc_msg(PLC_CMD_DEPLOY_SENSOR_LIST_ACK,0,hbb_info.config.id,0,plc_message->sequence,0);
	send_plc_msg((uint8_t *)&plc_send_message,PLC_PACKAGE_LEN);
	PRINTF("PLC tx deploy ack , MY HBB IS %x\r\n",hbb_info.config.master);
#else
	PRINTF("PLC deploy slist to %x\r\n", plc_message->sn_dest);
	send_plc_msg((uint8_t*)plc_message,PLC_PACKAGE_LEN);
	get_then_sent_slist();
#endif
}

void deal_PLC_CMD_DEPLOY_SENSOR_LIST_ACK(PLC_MSG *plc_message)
{
	PRINTF("PLC rx deploy slist ack from %x\r\n", plc_message->sn_mitt);
}

void deal_PLC_CMD_REQUEST_SENSOR_LIST(PLC_MSG *plc_message)
{
#ifdef BRIDGE
	PRINTF("PLC tx request slist\r\n");
	send_plc_msg((uint8_t*)plc_message,PLC_PACKAGE_LEN);
#else
	deploy_sensor_list_to_bridge(plc_message->sn_mitt,plc_message->sequence);
#endif
}


void deal_PLC_CMD_UPDATE_FW_ACK(PLC_MSG *plc_message)
{
#ifdef BRIDGE
	PRINTF("PLC sent fota cmd ack, dt:%x \r\n",plc_message->device_type);
	package_plc_msg(PLC_CMD_UPDATE_FW_ACK,plc_message->device_type,hbb_info.config.id,plc_message->sn_sensor,plc_message->sequence,0);
	send_plc_msg((uint8_t *)&plc_send_message,PLC_PACKAGE_LEN);
	xQueueSend(plc_fota_queue,plc_message, portMAX_DELAY);
#else
	PRINTF("PLC rx fota cmd ack from %x, dt:%x\r\n",plc_message->sn_mitt,plc_message->device_type);
	xEventGroupSetBits( BridgeFwReqAckEventGroup, B_BRIDGE_FW_REQ_ACK);
#endif
}

void deal_PLC_CMD_UPDATE_FW(PLC_MSG *plc_message)
{
#ifdef BRIDGE
	PRINTF("PLC rx fota cmd , dt:%x \r\n",plc_message->device_type);
	deal_PLC_CMD_UPDATE_FW_ACK(plc_message);
#else
	PRINTF("PLC tx fota cmd to %x, dt:%x\r\n",plc_message->sn_dest,plc_message->device_type);
	send_plc_msg((uint8_t*)plc_message,PLC_PACKAGE_LEN);
#endif
}

void deal_PLC_CMD_REQUEST_IMG_BLOCK(PLC_MSG *plc_message)
{
#ifdef BRIDGE
	PRINTF("PLC req img, dt:%x ,NO.%d\r\n",plc_message->device_type,plc_message->data);
	send_plc_msg((uint8_t*)plc_message,PLC_PACKAGE_LEN);
#else
	//todo tell hb to sent block
	//for test
	plc_request_img_block_ack(plc_message->sn_mitt,plc_message->device_type,plc_message->data,plc_message->sequence);
#endif
}

void deal_PLC_CMD_REQUEST_IMG_BLOCK_ACK(PLC_MSG *plc_message)
{
#ifdef BRIDGE
	PRINTF("PLC rx IMG NO.%d\r\n",plc_message->data);
	xSemaphoreGive(plc_fota_block_sem);
#else
	PRINTF("PLC request fw img ,device type : %x, block NO.%d\r\n", plc_message->device_type,plc_message->data);
	send_plc_msg((uint8_t*)plc_message,PLC_PACKAGE_LEN);
	send_plc_msg((uint8_t*)&plc_sent_fw_block,PLC_FW_BLOCK_LEN);
#endif
}
void deal_PLC_CMD_FW_UPDATE_RESULT(PLC_MSG *plc_message)
{
#ifdef BRIDGE
	PRINTF("PLC tx fota result : %s\r\n",plc_message->data?"SUCCESS":"FAIL");
	send_plc_msg((uint8_t*)plc_message,PLC_PACKAGE_LEN);
	PRINTF("PLC tx slist \r\n");
	get_then_sent_slist();
#else
	//UPDATE SUCCESS
	PRINTF("PLC rx fota %x result : %s\r\n",plc_message->device_type,plc_message->data?"SUCCESS":"FAIL");
	update_plc_sensor_status();
	xEventGroupSetBits( BridgeFwReqAckEventGroup, B_SENSOR_FW_RET_ACK);
#endif
}
void plc_status_task(void *pvParameters)
{
	PLC_MSG plc_message = {0};
	plc_info_init();
	while(1)
	{
		if(xQueueReceive(plc_msg_queue,&plc_message,portMAX_DELAY))
		{
			switch(plc_message.cmd)
			{
				case PLC_CMD_GENERIC_REQUEST:
					deal_PLC_CMD_GENERIC_REQUEST(&plc_message);
					break;
				case PLC_CMD_GENERIC_REQUEST_ACK:
					deal_PLC_CMD_GENERIC_REQUEST_ACK(&plc_message);
					break;
				case PLC_CMD_ALARM:
					deal_ALARM(&plc_message);
					break;
				case PLC_CMD_ALARM_CLEAR:
					deal_UNALARM(&plc_message);
					break;
				case PLC_CMD_ALARM_ACK:
				case PLC_CMD_ALARM_CLEAR_ACK:
					deal_ALARM_UNALARM_ACK(&plc_message);
					break;
				case PLC_CMD_TOGGLE:
					deal_TOGGLE(&plc_message);
					break;
				case PLC_CMD_TOGGLE_ACK:
					deal_TOGGLE_ACK(&plc_message);
					break;
				case PLC_CMD_SET_PIR_SENSITIVITY:
					deal_PLC_CMD_SET_PIR_SENSITIVITY(&plc_message);
					break;
				case PLC_CMD_SET_PIR_SENSITIVITY_ACK:
					deal_PLC_CMD_SET_PIR_SENSITIVITY_ACK(&plc_message);
					break;
				case PLC_CMD_SENT_SENSOR_LIST:
					deal_PLC_CMD_SENT_SENSOR_LIST(&plc_message);
					break;
				case PLC_CMD_SENT_SENSOR_LIST_ACK:
					deal_PLC_CMD_SENT_SENSOR_LIST_ACK(&plc_message);
					break;
				case PLC_CMD_DEPLOY_SENSOR_LIST:
					deal_PLC_CMD_DEPLOY_SENSOR_LIST(&plc_message);
					break;
				case PLC_CMD_DEPLOY_SENSOR_LIST_ACK:
					deal_PLC_CMD_DEPLOY_SENSOR_LIST_ACK(&plc_message);
					break;
				case PLC_CMD_REQUEST_SENSOR_LIST:
					deal_PLC_CMD_REQUEST_SENSOR_LIST(&plc_message);
					break;
				case PLC_CMD_UPDATE_FW:
					deal_PLC_CMD_UPDATE_FW(&plc_message);
					break;
				case PLC_CMD_UPDATE_FW_ACK:
					deal_PLC_CMD_UPDATE_FW_ACK(&plc_message);
					break;
				case PLC_CMD_REQUEST_IMG_BLOCK:
					deal_PLC_CMD_REQUEST_IMG_BLOCK(&plc_message);
					break;
				case PLC_CMD_REQUEST_IMG_BLOCK_ACK:
					deal_PLC_CMD_REQUEST_IMG_BLOCK_ACK(&plc_message);
					break;
				case PLC_CMD_FW_UPDATE_RESULT:
					deal_PLC_CMD_FW_UPDATE_RESULT(&plc_message);
					break;
				default:
					PRINTF("ERR :¡¡UNEXPECT PLC MESSAGE\r\n");
					break;
			}
		}

	}
}

PLC_DATA_MSG rev_data;
#include "test_ping.h"
void plc_get_message_task(void *pvParameters)
{

	uint8_t tmp_buf[PLC_FW_BLOCK_LEN+8] = {0};
	uint8_t *rev_point = tmp_buf;

	vTaskDelay(1000);
	TEST_PING_SN(NULL);
	while(1)
	{
		xQueueReceive(plc_data_queue,&rev_data,portMAX_DELAY);

		if(PLC_MSG_HEADER == rev_data.header_msg.header)
		{
			memcpy(&plc_rev_message,&rev_data.header_msg,PLC_PACKAGE_LEN);
			PRINTF("\r\nPLC need %d,PLC rev len= %d  \r\n",PLC_PACKAGE_LEN,rev_data.len);
			//check crc
			if(crc_cal((uint8_t*)&plc_rev_message.retry,PLC_PACKAGE_LEN-4)!= plc_rev_message.crc)
			{
				PRINTF("PLC ERR : message crc check err\r\n");
				continue;
			}

			if((PLC_CMD_SENT_SENSOR_LIST == plc_rev_message.cmd) ||
				(PLC_CMD_SENT_SENSOR_LIST_ACK == plc_rev_message.cmd) ||
				(PLC_CMD_DEPLOY_SENSOR_LIST == plc_rev_message.cmd) ||
				(PLC_CMD_REQUEST_IMG_BLOCK_ACK == plc_rev_message.cmd) ||
				(PLC_CMD_FW_UPDATE_RESULT == plc_rev_message.cmd))
			{
				//continue receive
				rev_point = tmp_buf;
			}
			else
			{
				PRINTF("PLC GET MSG\r\n");
				xQueueSend(plc_msg_queue,&plc_rev_message, portMAX_DELAY);
			}
			continue;
		}

		if((PLC_CMD_SENT_SENSOR_LIST == plc_rev_message.cmd) ||
			(PLC_CMD_SENT_SENSOR_LIST_ACK == plc_rev_message.cmd) ||
			(PLC_CMD_DEPLOY_SENSOR_LIST == plc_rev_message.cmd) ||
			(PLC_CMD_FW_UPDATE_RESULT == plc_rev_message.cmd) )
		{
			memcpy(rev_point,&rev_data.plc_data,rev_data.len);
			PRINTF("\r\nPLC need %d,PLC rev len= %d  \r\n",PLC_SENSOR_LIST_LEN,rev_data.len);
//			if(rev_point+rev_data.len > (&tmp_buf+PLC_SENSOR_LIST_LEN))
//			{
//				PRINTF("PLC ERR : get list len wrong\r\n");
//				rev_point = tmp_buf;
//				continue;
//			}
			rev_point += rev_data.len;
			if(rev_point >= (&tmp_buf[0]+PLC_SENSOR_LIST_LEN))//get list
			{
				memcpy(&plc_recv_sensor_list,tmp_buf,PLC_SENSOR_LIST_LEN);
				rev_point = tmp_buf;
				if(crc_cal((uint8_t*)&plc_recv_sensor_list.sensor_info,PLC_SENSOR_LIST_LEN-4)!= plc_recv_sensor_list.crc)
				{
					PRINTF("PLC ERR : list crc err\r\n");
					continue;
				}
				xQueueSend(plc_msg_queue,&plc_rev_message, portMAX_DELAY);
			}
		}
		else if(PLC_CMD_REQUEST_IMG_BLOCK_ACK == plc_rev_message.cmd)
		{
			memcpy(rev_point,&rev_data.plc_data,rev_data.len);
			rev_point = tmp_buf;
//			if(rev_point+rev_data.len > (&tmp_buf[0]+PLC_FW_BLOCK_LEN))
//			{
//				PRINTF("PLC ERR : get block len wrong\r\n");
//				continue;
//			}
			rev_point += rev_data.len;
			if(rev_point >= (&tmp_buf[0]+PLC_FW_BLOCK_LEN))//get block
			{
				memcpy(&plc_recv_fw_block,tmp_buf,PLC_FW_BLOCK_LEN);
				xQueueSend(plc_msg_queue,&plc_rev_message, portMAX_DELAY);
			}
		}
	}
}

/*
void plc_get_message_task(void *pvParameters)
{
	size_t received_len;
	uint16_t header = 0;
	uint8_t r_byte;
	uint8_t tmp_buf[PLC_FW_BLOCK_LEN] = {0};
	uint8_t *point = tmp_buf;

	plc_uart_init();
	while(1)
	{
		 UART_RTOS_Receive(&PLC_handle,&r_byte,1,&received_len);
//		 PRINTF(", %x",r_byte);
		 if(wait_plc_data_timeout_flag)
		 {
				PRINTF("time out , addr: %x\r\n",point);
			 	wait_plc_data_timeout_flag = false;
				header = 0;
				point = tmp_buf;
		 }
		*point = r_byte;

		if(header == PLC_MSG_HEADER)
		{

			if(point == &tmp_buf[PLC_PACKAGE_LEN-3])
			{
				memcpy(&plc_rev_message.retry,tmp_buf,PLC_PACKAGE_LEN-2);
				if(crc_cal((uint8_t*)&plc_rev_message.retry,PLC_PACKAGE_LEN-4)!= plc_rev_message.crc)
				{
					PRINTF("PLC ERR : message crc check err\r\n");
					header = 0;
					continue;
				}
#ifdef BRIDGE
				if(plc_rev_message.sn_dest != hbb_info.config.id)
				{
					PRINTF("PLC MSG NOT FOR ME,dest is %x!\r\n",plc_rev_message.sn_dest);
					continue;
				}
#endif
				if((PLC_CMD_SENT_SENSOR_LIST == plc_rev_message.cmd) ||
					(PLC_CMD_SENT_SENSOR_LIST_ACK == plc_rev_message.cmd) ||
					(PLC_CMD_DEPLOY_SENSOR_LIST == plc_rev_message.cmd) ||
					(PLC_CMD_REQUEST_IMG_BLOCK_ACK == plc_rev_message.cmd) ||
					(PLC_CMD_FW_UPDATE_RESULT == plc_rev_message.cmd))
				{
					// continue receive the sensor list
					//for test
//					uint64_t timestamp;
//					get_current_systime(&timestamp);
//					PRINTF("PLC recv : %d msg , %d:%d\r\n",plc_rev_message.cmd,(uint32_t)(timestamp / 1000), (uint32_t)(timestamp % 1000));
				}
				else
				{
					xQueueSend(plc_msg_queue,&plc_rev_message, portMAX_DELAY);
				}

				point = tmp_buf;
				header = 0;
			}
			else
			{
				point++;
			}
		}
		else if(header == PLC_SENSOR_LIST_HEADER)
		{
			if(point == &tmp_buf[PLC_SENSOR_LIST_LEN-3])
			{
				memcpy(&plc_recv_sensor_list.crc,tmp_buf,PLC_SENSOR_LIST_LEN-2);
				if(crc_cal((uint8_t*)&plc_recv_sensor_list.sensor_info,PLC_SENSOR_LIST_LEN-4)!= plc_recv_sensor_list.crc)
				{
					PRINTF("PLC ERR : msg crc err\r\n");
					header = 0;
					continue;
				}
				// continue receive the sensor list
				//for test
//				uint64_t timestamp;
//				get_current_systime(&timestamp);
//				PRINTF("PLC recv list : %d msg , %d:%d\r\n",plc_rev_message.cmd,(uint32_t)(timestamp / 1000), (uint32_t)(timestamp % 1000));
				xQueueSend(plc_msg_queue,&plc_rev_message, portMAX_DELAY);
				point = tmp_buf;
				header = 0;
			}
			else
			{
				point++;
			}
		}
		else if(header == PLC_FW_BLOCK_HEADER)
		{
			if(point == &tmp_buf[PLC_FW_BLOCK_LEN-3])
			{
				memset(&plc_recv_fw_block, 0xff, PLC_FW_BLOCK_LEN);
				memcpy(&plc_recv_fw_block.reserve,tmp_buf,PLC_FW_BLOCK_LEN-2);
				xQueueSend(plc_msg_queue,&plc_rev_message, portMAX_DELAY);
				point = tmp_buf;
				header = 0;
			}
			else
			{
				point++;
			}
		}
		else
		{
			if((*(uint16_t *)(point-1) == PLC_MSG_HEADER) || (*(uint16_t *)(point-1) == PLC_SENSOR_LIST_HEADER) || (*(uint16_t *)(point-1) == PLC_FW_BLOCK_HEADER))
			{
//				PRINTF("PLC GET %x\r\n",*(uint16_t *)(point-1));
				header = *(uint16_t *)(point-1);
				point = tmp_buf;
			}
			else
			{
				point++;
				if(point>= &tmp_buf[PLC_FW_BLOCK_LEN]) point = tmp_buf+1;
			}
		}


	}
}
*/
#endif
