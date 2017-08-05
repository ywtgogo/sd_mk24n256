/*
 * Copyright (c) 2015, Freescale Semiconductor, Inc.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *
 * o Redistributions of source code must retain the above copyright notice, this list
 *   of conditions and the following disclaimer.
 *
 * o Redistributions in binary form must reproduce the above copyright notice, this
 *   list of conditions and the following disclaimer in the documentation and/or
 *   other materials provided with the distribution.
 *
 * o Neither the name of Freescale Semiconductor, Inc. nor the names of its
 *   contributors may be used to endorse or promote products derived from this
 *   software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
 * ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */
#define PR_MODULE "ppp\1"

/* FreeRTOS kernel includes. */
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "timers.h"

/* Freescale includes. */
#include "fsl_device_registers.h"
#include "fsl_debug_console.h"
#include "board.h"

#include "fsl_uart_freertos.h"
#include "fsl_uart.h"
#include "fsl_port.h"
#include "pin_mux.h"
#include "clock_config.h"
#include "pppif.h"
#include "ppp.h"
#include "modem_AT.h"
#include "board.h"
#include "netif.h"
#include "lwip/dns.h"
#include "lwip/tcpip.h"
#include "ppp.h"
#include "uart_ppp.h"
#include "hbb_config.h"
#include "fsl_adc16.h"
#include "uplink_protocol.h"
#include "hb_protocol.h"
#include "log_task.h"
#include "mqtt_task.h"
#include "json.h"
#include "fota.h"
#include "audio_play.h"

/*******************************************************************************
 * Variables
 ******************************************************************************/
extern u32_t aging_test_for_ppp;
uint8_t ppp_background_buffer[2048];
uart_rtos_handle_t ppp_handle;
SemaphoreHandle_t call_http_mqtt_sem;				/*	*/
struct _uart_handle ppp_t_handle;
const char *send_ring_overrun = "\r\nRing buffer overrun!\r\n";
const char *send_hardware_overrun = "\r\nHardware buffer overrun!\r\n";

struct rtos_uart_config modem_uart_config = {
    .baudrate = 115200,
    .parity = kUART_ParityDisabled,
    .stopbits = kUART_OneStopBit,
    .buffer = ppp_background_buffer,
    .buffer_size = sizeof(ppp_background_buffer),
};

u8_t buf_ppp[PPP_BUFFER_LEN];
u32_t next_seq_no = 0;
u32_t ppp_pd = 0;
u32_t bat_adc_counter = 0;
u32_t bat_adc_shut_counter = 0;
uint32_t bat_volt_low_flag = 0;
u32_t power_ac_dc_status = 1;
u32_t sos_incoming_call_status = 0;
u32_t sim_status = 0;
SEQ_TBL  * seq_func_g;
u8_t ppp_temp_recved;
u8_t ppp_rec[1];

#if defined(EU_DEMO)

ppp_gprs_info_conf ppp_gprs_info = {
	.ppp_relink_count = 0,
	.ppp_gprs_timer_counter = 0,
	.ppp_gprs_current_seq = SEQ_001,
	.ppp_gprs_current_status = K24_STATUS_LIVE,
	.ppp_uart_current_counter = 1,
	.ppp_uart_old_counter = 0,
	.ppp_uart_timer_counter = 0,
	.ppp_uart_current_status = K24_UART_STATUS_LIVE,
	.http_ask_ppp_to_negotiate = 0,
	.modem_need_at_mode = 0,
	.modem_need_apn_update = 0,
	.modem_need_sos_update = 0,
	.irq_for_sms_or_incoming_call = NO_SMS_OR_INCOMING_CALL,
	.irq_for_sos_call = SOS_NO_CALL,
	.ppp_modem_init_flag = 1,
	.incoming_call_answer_timeout = 27,
	
	.gprs_apn = "\"sldrei.at\"\r",
	.ppp_pap_user = "sladmin",
	.ppp_pap_passwd = "slpasswd",
	.sos_call = "+436607622941\r",
	.sms_max_number_me = 50,
	.sms_current_number_me = 30,
};

ppp_gprs_apn_pap_info_conf ppp_gprs_apn_pap_info = {
	.gprs_apn = "\"sldrei.at\"\r",
	.ppp_pap_user = "sladmin",
	.ppp_pap_passwd = "slpasswd",
	.sos_call = "+436607622941\r",
	.incoming_call_answer_timeout = 27,
	.sms_max_number_me = 50,
	.sms_curr_number_me = 30,
};
#elif defined(CN_DEMO)
ppp_gprs_info_conf ppp_gprs_info = {
	.ppp_relink_count = 0,
	.ppp_gprs_timer_counter = 0,
	.ppp_gprs_current_seq = SEQ_001,
	.ppp_gprs_current_status = K24_STATUS_LIVE,
	.ppp_uart_current_counter = 1,
	.ppp_uart_old_counter = 0,
	.ppp_uart_timer_counter = 0,
	.ppp_uart_current_status = K24_UART_STATUS_LIVE,
	.http_ask_ppp_to_negotiate = 0,
	.modem_need_at_mode = 0,
	.modem_need_apn_update = 0,
	.modem_need_sos_update = 0,
	.irq_for_sms_or_incoming_call = NO_SMS_OR_INCOMING_CALL,
	.irq_for_sos_call = SOS_NO_CALL,
	.ppp_modem_init_flag = 1,
	.incoming_call_answer_timeout = 27,
	
	.gprs_apn = "\"sl3gnet\"\r",
	.ppp_pap_user = "sladmin",
	.ppp_pap_passwd = "slpasswd",
	.sos_call = "+8613776637279\r",
	.sms_max_number_me = 50,
	.sms_current_number_me = 30,
};

ppp_gprs_apn_pap_info_conf ppp_gprs_apn_pap_info = {
	.gprs_apn = "\"sl3gnet\"\r",
	.ppp_pap_user = "sladmin",
	.ppp_pap_passwd = "slpasswd",
	.sos_call = "+8613776637279\r",
	.incoming_call_answer_timeout = 27,
	.sms_max_number_me = 50,
	.sms_curr_number_me = 30,
};
#else
ppp_gprs_info_conf ppp_gprs_info = {
	.ppp_relink_count = 0,
	.ppp_gprs_timer_counter = 0,
	.ppp_gprs_current_seq = SEQ_001,
	.ppp_gprs_current_status = K24_STATUS_LIVE,
	.ppp_uart_current_counter = 1,
	.ppp_uart_old_counter = 0,
	.ppp_uart_timer_counter = 0,
	.ppp_uart_current_status = K24_UART_STATUS_LIVE,
	.http_ask_ppp_to_negotiate = 0,
	.modem_need_at_mode = 0,
	.modem_need_apn_update = 0,
	.modem_need_sos_update = 0,
	.irq_for_sms_or_incoming_call = NO_SMS_OR_INCOMING_CALL,
	.irq_for_sos_call = SOS_NO_CALL,
	.ppp_modem_init_flag = 1,
	.incoming_call_answer_timeout = 27,
	
	.gprs_apn = "\"sl3gnet\"\r",
	.ppp_pap_user = "sladmin",
	.ppp_pap_passwd = "slpasswd",
	.sos_call = "112\r",
	.sms_max_number_me = 50,
	.sms_current_number_me = 30,
};

ppp_gprs_apn_pap_info_conf ppp_gprs_apn_pap_info = {
	.gprs_apn = "\"sl3gnet\"\r",
	.ppp_pap_user = "sladmin",
	.ppp_pap_passwd = "slpasswd",
	.sos_call = "112\r",
	.incoming_call_answer_timeout = 27,
	.sms_max_number_me = 50,
	.sms_curr_number_me = 30,
};

#endif

#ifdef CPU_MK24FN256VDC12

/*******************************************************************************
 * Prototypes
 ******************************************************************************/
extern CHAR at_cmd_sos_call[32];
extern void lwip_init(void);
extern void pppInit(void);
extern void ppp_state_callback(void *ctx, int errCode, void *arg);
extern void pppos_input(int pd, u_char* data, int len);
extern err_t pppifNetifInit(struct netif *netif);
extern int sifdown(int pd);
extern void sync_gprs_info();
extern int defer_log_uplink();
/*******************************************************************************
 * Code
 ******************************************************************************/
/*
*hao.yunran
*2017.1.6
*
*/
u32_t update_sos_call_number(CHAR* sos_number)
{
	u32_t ret = AT_RET_OK;
	u32_t sos_len = 0;

	sos_len = strlen(sos_number);
	if(sos_len > 32)
	{
		return AT_RET_FAIL;
	}
	memset(&ppp_gprs_apn_pap_info.sos_call, 0, sizeof(ppp_gprs_apn_pap_info.sos_call));
	strncpy(ppp_gprs_apn_pap_info.sos_call, sos_number, sos_len);
	memset(&at_cmd_sos_call, 0, sizeof(at_cmd_sos_call));
	
	sprintf(at_cmd_sos_call,"ATD%s\r", sos_number);

	PRINTF("..update_sos_call_number..%s\r\n",at_cmd_sos_call);	
	sync_gprs_info();
	return ret;
}

/*
*hao.yunran
*2017.1.6
*
*/
u32_t update_incoming_call_auto_answer_timeout(u32_t timeout)
{
	u32_t ret = AT_RET_OK;
	PRINTF("..update_incoming_call_auto_answer_timeout..%d\r\n",timeout);	
	ppp_gprs_apn_pap_info.incoming_call_answer_timeout = timeout;
	sync_gprs_info();
	return ret;
}

/*
*hao.yunran
*2017.1.6
*
*/
u32_t update_power_info(DATA_INFO *data_info, DATA_ITEM_STRUCT *data_item)
{
	u32_t Adc16_Value = 0;
	u32_t nPG_Value = 0;
	char format[] = "\"10201/vt:%d,10302/s:%s\"";

	if((Adc16_Value = BAT_adc_read()) > BAT_ADC_NOR_VALUE)
		return AT_RET_FAIL;

	if((nPG_Value = BAT_nPG_READ()) == 0x1)
		sprintf(data_item->info, format, (Adc16_Value*100)/BAT_ADC_NOR_VALUE, "AC");
	else
		sprintf(data_item->info, format, (Adc16_Value*100)/BAT_ADC_NOR_VALUE, "DC");
	data_item->status = INFO;
	return AT_RET_OK;
}

/*
*hao.yunran
*2017.1.6
*
*/
u32_t update_bat_voltage(DATA_INFO *data_info, DATA_ITEM_STRUCT *data_item)
{
	u32_t ret = AT_RET_OK;

	return ret;
}

/*
*hao.yunran
*2017.1.6
*
*/
u32_t update_power_supply_mode(DATA_INFO *data_info, DATA_ITEM_STRUCT *data_item)
{
	u32_t ret = AT_RET_OK;

	return ret;
}

/*
*hao.yunran
*2017.1.6
*dial GPRS, if successful, we start LCP Config
*/
u32_t ppp_check_status()
{
	if(pdFALSE == xSemaphoreTake(call_http_mqtt_sem, 0U))
	{
		return ERR_TIMEOUT;
	}
	if(ppp_gprs_info.modem_need_at_mode == 1)
	{
		PRINTF("modem_need_at_mode.....\r\n");
		xSemaphoreGive(call_http_mqtt_sem);
	  return ERR_RTE;
	}

	if(get_lwip_link_state() == PPP_ESTABLISHED)
	{
    	//GPIO_ClearPinsOutput(GPIOD, 1U << 15);
    	xSemaphoreGive(call_http_mqtt_sem);
		return ERR_OK;
	}
	else if(get_lwip_link_state() == PPP_DEAD)
	{
		PR_INFO("PPP is dead. try to negotitate!\r\n");
		ppp_gprs_info.http_ask_ppp_to_negotiate = HTTP_ASK_PPP_START_NEGOT;
    	//GPIO_SetPinsOutput(GPIOD, 1U << 15);
	}
	else
	{
		PR_INFO("PPP is negotitating! wait....\r\n");

	}
	//GPIO_SetPinsOutput(GPIOD, 1U << 15);
	vTaskDelay(PPP_DIAL_TIMEOUT);
	//GPIO_ClearPinsOutput(GPIOD, 1U << 15);
	if(ppp_gprs_info.ppp_negotiate_result != PPP_NEGOTIATE_OK)
	{
    	//GPIO_SetPinsOutput(GPIOD, 1U << 15);
		PR_INFO("PPP error!\r\n");
		xSemaphoreGive(call_http_mqtt_sem);
		return ERR_TIMEOUT;
	}
	xSemaphoreGive(call_http_mqtt_sem);
	return ERR_OK;
}

/*
*hao.yunran
*2016.10.10
*dial GPRS, if successful, we start LCP Config
*/
void BAT_adc_init()
{
    adc16_config_t adc16ConfigStruct;
    adc16_channel_config_t adc16ChannelConfigStruct;

	ADC16_GetDefaultConfig(&adc16ConfigStruct);
	ADC16_Init(K24_ADC16_BASE, &adc16ConfigStruct);
	ADC16_EnableHardwareTrigger(K24_ADC16_BASE, false); /* Make sure the software trigger is used. */
	if (kStatus_Success == ADC16_DoAutoCalibration(K24_ADC16_BASE))
	{
		PRINTF("..ADC16_DoAutoCalibration().Done.\r\n");
	}
	else
	{
		PRINTF("..ADC16_DoAutoCalibration().Failed.\r\n");
	}
		adc16ChannelConfigStruct.channelNumber = K24_ADC16_USER_CHANNEL;
		adc16ChannelConfigStruct.enableInterruptOnConversionCompleted = false; /* Enable the interrupt. */
		adc16ChannelConfigStruct.enableDifferentialConversion = false;
        ADC16_SetChannelConfig(K24_ADC16_BASE, K24_ADC16_CHANNEL_GROUP, &adc16ChannelConfigStruct);
}

/*
*hao.yunran
*2016.10.10
*dial GPRS, if successful, we start LCP Config
*/
u32_t BAT_adc_read()
{
	volatile u32_t Adc16ConversionValue;
	
    adc16_channel_config_t adc16ChannelConfigStruct;
	adc16ChannelConfigStruct.channelNumber = K24_ADC16_USER_CHANNEL;
	adc16ChannelConfigStruct.enableInterruptOnConversionCompleted = false; /* Enable the interrupt. */
	adc16ChannelConfigStruct.enableDifferentialConversion = false;

	ADC16_SetChannelConfig(K24_ADC16_BASE, K24_ADC16_CHANNEL_GROUP, &adc16ChannelConfigStruct);
	Adc16ConversionValue = ADC16_GetChannelConversionValue(K24_ADC16_BASE, K24_ADC16_CHANNEL_GROUP);	
	//PRINTF("^_^ADC value=0x%x\n",Adc16ConversionValue);
	return Adc16ConversionValue;
}

u32_t send_bat_status(uint32_t status,int p,char *data_str)
{
	//u32_t i;
	char data[20];
	memset(data, 0, 20);
	
	strncpy(data, data_str, strlen(data_str));
	PRINTF("0 %s \r\n",data);

	hbb_send_uplink_data(hbb_info.config.id, status,  p, "%s", data);

	return 0;
}

/*
*hao.yunran
*2016.10.10
*dial GPRS, if successful, we start LCP Config
*/
u32_t enable_sos_sms_interrupt()
{
	GPIO_ClearPinsInterruptFlags(MODEM_SOS_GPIO, 1U << MODEM_SOS_GPIO_PIN);
	EnableIRQ(MODEM_SOS_IRQ);
	GPIO_ClearPinsInterruptFlags(MODEM_RING_SMS_GPIO, 1U << MODEM_RING_SMS_GPIO_PIN);
	EnableIRQ(MODEM_RING_SMS_IRQ);
	return AT_RET_OK;
}

/*
*hao.yunran
*2016.10.10
*dial GPRS, if successful, we start LCP Config
*/
u32_t set_all_param_to_data_mode()
{
	sos_incoming_call_status = NO_CALL;
	ppp_gprs_info.modem_need_at_mode = 0;
	ppp_gprs_info.irq_for_sos_call = SOS_NO_CALL;
	ppp_gprs_info.irq_for_sms_or_incoming_call = NO_SMS_OR_INCOMING_CALL;
	return AT_RET_OK;
}

/*
*hao.yunran
*2016.10.10
*dial GPRS, if successful, we start LCP Config
*/
u32_t gsm_turn_at_mode_wait_uart_ready(u32_t timeout)
{
	at_ret ret = AT_RET_FAIL;		
	u32_t dcd_value = 0;
	u32_t to_at_timeout = 1600;
	u32_t at_delay_timeout = 1200;
	u32_t time = xTaskGetTickCount();//get_sys_tick();
	//UART_RTOS_Receive_NoBlocking(&ppp_handle, buf_ppp, sizeof(buf_ppp));

	PRINTF("..turn_at_mode_wait_uart_ready..\r\n");	
	dcd_value = MODEM_DCD_READ();
	ppp_gprs_info.modem_need_at_mode = 1;
	if(dcd_value == 1)
	{
		ret = AT_RET_OK;
	}
	else
	{
		while((xTaskGetTickCount() - time) < to_at_timeout)
		{		
			if(SOS_CALL_HAND_UP == ppp_gprs_info.irq_for_sos_call)
			{	
				DisableIRQ(MODEM_SOS_IRQ);
				return AT_RET_SOS_CALL_DOWN;
			}
			else
			{
				vTaskDelay(100);
			}
		}
		
		modem_at_cmd_plus(&ppp_handle, AT_REPEAT_MAX, AT_TIMEOUT_MAX, WAIT_RETURN);

		time = xTaskGetTickCount();
		while((xTaskGetTickCount() - time) < at_delay_timeout)
		{
			if(SOS_CALL_HAND_UP == ppp_gprs_info.irq_for_sos_call)
			{	
				DisableIRQ(MODEM_SOS_IRQ);
				return AT_RET_SOS_CALL_DOWN;
			}
			else
			{
				vTaskDelay(100);
			}
		}	
		
		ret = modem_at_cmd_at(&ppp_handle, AT_REPEAT_MAX, AT_TIMEOUT_MAX, WAIT_RETURN);
		if(ret != AT_RET_OK)
		{
			PRINTF("..not.change.to.at..\r\n");
			return AT_RET_FAIL;
		}
		else if(ret == AT_RET_OK)
		{
			time = xTaskGetTickCount();
			while((xTaskGetTickCount() - time) < (timeout*1000))
			{
				if(pdFALSE == xSemaphoreTake(ppp_handle.rx_sem, 0U))
				{			
					PRINTF("..uart.rx_sem.used_now..\r\n");
					vTaskDelay(100);
					ret = AT_RET_FAIL;
				}
				else
				{				
					ret = AT_RET_OK;
					break;
				}
			}
			if(ret == AT_RET_OK)
			{
				xSemaphoreGive(ppp_handle.rx_sem);				
				PRINTF("..sem ready..\r\n"); 
			}
			else
			{
				xSemaphoreGive(ppp_handle.rx_sem);				
			}
		}
	}
	//PRINTF("..hh4....................%d\r\n",ret);
	return ret;
}

/*
*hao.yunran
*2016.10.10
*dial GPRS, if successful, we start LCP Config
*/
u32_t gsm_incoming_call_auto_answer()
{
	u32_t ret = AT_RET_OK;
	ret = modem_at_cmd_ata(&ppp_handle, AT_REPEAT_MAX, AT_TIMEOUT_MAX, WAIT_RETURN);
	if(ret != AT_RET_OK)
	{
		PRINTF(".at cmd fail.\r\n");
	}
	ret = modem_wait_incoming_call_end_clcc(&ppp_handle);
	switch(ret)
	{	
			// the one who makes incoming call hands up the call
		case AT_RET_CALL_DOWN:   
			PRINTF(".incoming_call.end.\r\n");
			vTaskDelay(50);
			modem_at_cmd_chup(&ppp_handle, AT_REPEAT_NOR, AT_TIMEOUT_MAX, NOT_WAIT);
			vTaskDelay(50);
			ret = modem_at_cmd_ato(&ppp_handle, AT_REPEAT_NOR, AT_TIMEOUT_MAX, WAIT_RETURN);
			if(ret != AT_RET_OK)
			{
				PRINTF(".ato cmd fail.\r\n");
			}
			else
			{
				PRINTF(".back_to_data_mode.\n");
			}
			break;
			// press sos key to hand up the call
		case AT_RET_SOS_CALL_UP:
			//the calling time is too long to timeout
		case AT_RET_CALL_DOWN_TIMER_OUT:
			if(ret == AT_RET_SOS_CALL_UP)
			{
				PRINTF(".press_key_end_incoming_call.\r\n");
			}
			else
			{
				PRINTF(".incoming_call.timeout.\n");
			}
			vTaskDelay(50);
			ret = modem_at_cmd_chup(&ppp_handle, AT_REPEAT_NOR, AT_TIMEOUT_MAX, WAIT_RETURN);
			if(ret != AT_RET_OK)
			{
				PRINTF(".chup cmd fail.\r\n");
			}
			vTaskDelay(50);
			ret = modem_at_cmd_ato(&ppp_handle, AT_REPEAT_NOR, AT_TIMEOUT_MAX, WAIT_RETURN);
			if(ret != AT_RET_OK)
			{
				PRINTF(".ato cmd fail.\r\n");			
			}
			else
			{
				PRINTF(".back_to_data_mode.\n");
			}
			break;

		default:
			PRINTF(".incoming_call.end.\n");
			vTaskDelay(50);
			modem_at_cmd_chup(&ppp_handle, AT_REPEAT_NOR, AT_TIMEOUT_MAX, NOT_WAIT);
			vTaskDelay(50);
			ret = modem_at_cmd_ato(&ppp_handle, AT_REPEAT_MAX, AT_TIMEOUT_MAX, WAIT_RETURN);
			if(ret != AT_RET_OK)
			{
				PRINTF(".ato cmd fail.\r\n");			
			}
			else
			{
				PRINTF(".back_to_data_mode.\n");
			}
			break;

	}
	return ret;
}

/*
*hao.yunran
*2016.10.10
*dial GPRS, if successful, we start LCP Config
*/
u32_t gsm_incoming_call_press_key_answer()
{
	u32_t ret = AT_RET_OK;
	ppp_gprs_info.irq_for_sos_call = SOS_NO_CALL;
	PRINTF(".press_key_answer.\r\n");
	ret = modem_at_cmd_ata(&ppp_handle, AT_REPEAT_MAX, AT_TIMEOUT_MAX, WAIT_RETURN);
	if(ret != AT_RET_OK)
	{
		PRINTF(".at cmd fail.\r\n");
	}
	vTaskDelay(50);
	ret = modem_wait_incoming_call_end_clcc(&ppp_handle);
	switch(ret)
	{	
			// the one who makes incoming call hands up the call
		case AT_RET_CALL_DOWN:
			PRINTF(".incoming_call.end.by.caller.\r\n");
			vTaskDelay(50);
			modem_at_cmd_chup(&ppp_handle, AT_REPEAT_NOR, AT_TIMEOUT_MAX, NOT_WAIT);
			vTaskDelay(50);
			ret = modem_at_cmd_ato(&ppp_handle, AT_REPEAT_NOR, AT_TIMEOUT_MAX, WAIT_RETURN);
			if(ret != AT_RET_OK)
			{
				PRINTF(".ato cmd fail.\r\n");
			}
			else
			{
				PRINTF(".back_to_data_mode.\n");
			}
			break;
			// press sos key to hand up the call
		case AT_RET_SOS_CALL_UP:
			// the call is timeout
		case AT_RET_CALL_DOWN_TIMER_OUT:
			if(ret == AT_RET_SOS_CALL_UP)
			{
				PRINTF(".press_key_end_incoming_call.\n");
			}
			else
			{
				PRINTF(".incoming_call.timeout.\n");
			}
			vTaskDelay(50);
			ret = modem_at_cmd_chup(&ppp_handle, AT_REPEAT_MAX, AT_TIMEOUT_MAX, WAIT_RETURN);
			if(ret != AT_RET_OK)
			{
				PRINTF(".chup cmd fail.\r\n");
			}
			vTaskDelay(50);
			ret = modem_at_cmd_ato(&ppp_handle, AT_REPEAT_MAX, AT_TIMEOUT_MAX, WAIT_RETURN);
			if(ret != AT_RET_OK)
			{
				PRINTF(".ato cmd fail.\r\n");
			}
			else
			{
				PRINTF(".back_to_data_mode.\n");
			}
			break;
			
		default :
			PRINTF(".incoming_call.end.\r\n");
			vTaskDelay(50);
			modem_at_cmd_chup(&ppp_handle, AT_REPEAT_NOR, AT_TIMEOUT_MAX, NOT_WAIT);
			vTaskDelay(50);
			ret = modem_at_cmd_ato(&ppp_handle, AT_REPEAT_NOR, AT_TIMEOUT_MAX, WAIT_RETURN);
			if(ret != AT_RET_OK)
			{
				PRINTF(".ato cmd fail.\r\n");
			}
			else
			{
				PRINTF(".back_to_data_mode.\n");
			}
			break;

	}

	return ret;
}

/*
*hao.yunran
*2016.10.10
*dial GPRS, if successful, we start LCP Config
*/
u32_t gsm_incoming_call_hand_up_by_caller()
{
	u32_t ret = AT_RET_OK;
	PRINTF(".incoming_call.hand_up.by_caller.\r\n");
	modem_at_cmd_chup(&ppp_handle, AT_REPEAT_NOR, AT_TIMEOUT_MAX, NOT_WAIT);
	vTaskDelay(50);
	ret = modem_at_cmd_ato(&ppp_handle, AT_REPEAT_NOR, AT_TIMEOUT_MAX, WAIT_RETURN);
	if(ret != AT_RET_OK)
	{
		PRINTF(".ato cmd fail.\r\n");
	}
	else
	{
		PRINTF(".back_to_data_mode.\n");
	}
	return ret;
}

/*
*hao.yunran
*2016.10.10
*dial GPRS, if successful, we start LCP Config
*/
u32_t gsm_incoming_call_mode_change_fail()
{
	u32_t ret = AT_RET_OK;
	modem_at_cmd_chup(&ppp_handle, AT_REPEAT_NOR, AT_TIMEOUT_MAX, NOT_WAIT);
	PRINTF(".incoming_call.hand_up_mode change fail.\n");
	vTaskDelay(50);
	ret = modem_at_cmd_ato(&ppp_handle, AT_REPEAT_NOR, AT_TIMEOUT_NOR, NOT_WAIT);
	if(ret != AT_RET_OK)
	{
		PRINTF(".ato cmd fail.\r\n");
	}
	else
	{
		PRINTF(".back_to_data_mode.\n");
	}
	//SPKR_OFF();
	set_all_param_to_data_mode();
	defer_log_uplink(); 	
	PRINTF("..cannot.turn_at.or.sem_fail..\r\n");			
	enable_sos_sms_interrupt();
	return ret;
}

/*
*hao.yunran
*2016.10.10
*dial GPRS, if successful, we start LCP Config
*/
u32_t gsm_incoming_call_hand_up_default()
{
	u32_t ret = AT_RET_OK;
	PRINTF(".incoming_call.end.default.\r\n");
	vTaskDelay(50);
	modem_at_cmd_chup(&ppp_handle, AT_REPEAT_NOR, AT_TIMEOUT_MAX, NOT_WAIT);
	vTaskDelay(50);
	ret = modem_at_cmd_ato(&ppp_handle, AT_REPEAT_NOR, AT_TIMEOUT_NOR, NOT_WAIT);
	if(ret != AT_RET_OK)
	{
		PRINTF(".ato cmd fail.\r\n");
	}
	else
	{
		PRINTF(".back_to_data_mode.\n");
	}
	return ret;
}

/*
*hao.yunran
*2016.10.10
*dial GPRS, if successful, we start LCP Config
*/
u32_t gsm_sms_apn_analyse()
{
	at_ret ret = AT_RET_OK;		
	//u32_t sms_numb = 0;

	PR_INFO("..update_apn..\r\n");
	memset(&ppp_gprs_apn_pap_info.gprs_apn, 0, sizeof(ppp_gprs_apn_pap_info.gprs_apn));
	memset(&ppp_gprs_apn_pap_info.ppp_pap_user, 0, sizeof(ppp_gprs_apn_pap_info.ppp_pap_user));
	memset(&ppp_gprs_apn_pap_info.ppp_pap_passwd, 0, sizeof(ppp_gprs_apn_pap_info.ppp_pap_passwd));
	
	strncpy(ppp_gprs_apn_pap_info.gprs_apn, ppp_gprs_info.gprs_apn, sizeof(ppp_gprs_info.gprs_apn));
	strncpy(ppp_gprs_apn_pap_info.ppp_pap_user, ppp_gprs_info.ppp_pap_user, sizeof(ppp_gprs_info.ppp_pap_user));
	strncpy(ppp_gprs_apn_pap_info.ppp_pap_passwd, ppp_gprs_info.ppp_pap_passwd, sizeof(ppp_gprs_info.ppp_pap_passwd));				
	if((ppp_gprs_info.sms_current_number_me+MAX_MARGIN_SMS_NUMB_ME) >= ppp_gprs_info.sms_max_number_me)
	{
		#if 0
		sms_numb = ppp_gprs_info.sms_current_number_me;
		for(; sms_numb > 0; sms_numb--)
		{
			modem_sms_delete(sms_numb);			
			vTaskDelay(60);
		}
		#endif
		vTaskDelay(60);
		ret = modem_at_cmd_cmgd_all(&ppp_handle, AT_REPEAT_MIN, AT_DELET_SMS_TIMEOUT_MAX, WAIT_RETURN);
	    if(ret != AT_RET_OK)
	    {
			PRINTF(".delete sms all fail.\r\n");
	    }
		vTaskDelay(60);
	}
	ret = modem_at_cmd_cnmi(&ppp_handle, AT_REPEAT_MIN, AT_TIMEOUT_MAX, NOT_WAIT);
	vTaskDelay(60);
	ppp_gprs_apn_pap_info.sms_curr_number_me = 1;
	ppp_gprs_apn_pap_info.sms_max_number_me = ppp_gprs_info.sms_max_number_me;		
	sync_gprs_info();
	//ppp_settings.user;
	//PRINTF("..this sms2..0x%x\r\n",seq_func_g);
	if(get_lwip_link_state() != PPP_DEAD)
	{
		sifdown(ppp_pd);
	}	
	pppCloseAll(ppp_pd);
	pppInit();
	ppp_gprs_info.ppp_negotiate_result = PPP_NEGOTIATE_FAIL;
	set_lwip_link_state(PPP_NEGOTIATE); 
	ret = AT_RET_OK;
	ppp_gprs_info.modem_need_apn_update = 1;
	return ret;
}

/*
*hao.yunran
*2016.10.10
*dial GPRS, if successful, we start LCP Config
*/
u32_t gsm_sms_sos_analyse()
{
	at_ret ret = AT_RET_OK;		
	//u32_t sms_numb = 0;

	PR_INFO("..update_sos..\r\n");
	memset(&ppp_gprs_apn_pap_info.sos_call, 0, sizeof(ppp_gprs_apn_pap_info.sos_call));
	strncpy(ppp_gprs_apn_pap_info.sos_call, ppp_gprs_info.sos_call, sizeof(ppp_gprs_info.sos_call));
	if((ppp_gprs_info.sms_current_number_me+MAX_MARGIN_SMS_NUMB_ME) >= ppp_gprs_info.sms_max_number_me)
	{
		#if 0
		sms_numb = ppp_gprs_info.sms_current_number_me;
		for(; sms_numb > 0; sms_numb--)
		{
			modem_sms_delete(sms_numb);			
			vTaskDelay(60);
		}
		#endif
		vTaskDelay(60);
		ret = modem_at_cmd_cmgd_all(&ppp_handle, AT_REPEAT_MIN, AT_DELET_SMS_TIMEOUT_MAX, WAIT_RETURN);
	    if(ret != AT_RET_OK)
	    {
			PRINTF(".delete sms all fail.\r\n");
	    }
		vTaskDelay(60);
	}
	ret = modem_at_cmd_cnmi(&ppp_handle, AT_REPEAT_MIN, AT_TIMEOUT_MAX, NOT_WAIT);
	vTaskDelay(60);
	ppp_gprs_apn_pap_info.sms_curr_number_me = 1;
	ppp_gprs_apn_pap_info.sms_max_number_me = ppp_gprs_info.sms_max_number_me;		
	//sync_gprs_info(); 						
	//sos_len = strlen(ppp_gprs_info.sos_call);
	memset(&at_cmd_sos_call, 0, sizeof(at_cmd_sos_call));				
	sprintf(at_cmd_sos_call,"ATD%s\r", ppp_gprs_info.sos_call); 			
	PRINTF(".sms.sos.%s\n",at_cmd_sos_call);
	
	ret = modem_at_cmd_ato(&ppp_handle, AT_REPEAT_MAX, AT_TIMEOUT_NOR, WAIT_RETURN);
	if(ret != AT_RET_OK)
	{
		PRINTF(".ato cmd fail.\r\n");
	}
	else
	{
		PRINTF(".back_to_data_mode.\n");
	}
	sync_gprs_info();
	ret = AT_RET_OK;	
	PRINTF(".sms.sos.end\n");	
	ppp_gprs_info.modem_need_sos_update = 1;
	return ret;
}

/*
*hao.yunran
*2016.10.10
*dial GPRS, if successful, we start LCP Config
*/
u32_t gsm_sms_process_default()
{
	at_ret ret = AT_RET_OK;		
	//u32_t sms_numb = 0;

	PRINTF("..no_apn_sos_update..\r\n");
	if((ppp_gprs_info.sms_current_number_me+MAX_MARGIN_SMS_NUMB_ME) >= ppp_gprs_info.sms_max_number_me)
	{
		#if 0
		sms_numb = ppp_gprs_info.sms_current_number_me;
		for(; sms_numb > 0; sms_numb--)
		{
			modem_sms_delete(sms_numb);			
			vTaskDelay(60);
		}
		#endif
		vTaskDelay(60);
		ret = modem_at_cmd_cmgd_all(&ppp_handle, AT_REPEAT_MIN, AT_DELET_SMS_TIMEOUT_MAX, WAIT_RETURN);
	    if(ret != AT_RET_OK)
	    {
			PRINTF(".delete sms all fail.\r\n");
	    }
		vTaskDelay(60);
	}
	modem_at_cmd_cnmi(&ppp_handle, AT_REPEAT_MIN, AT_TIMEOUT_MAX, NOT_WAIT);
	vTaskDelay(60);
	ppp_gprs_apn_pap_info.sms_curr_number_me = 1;
	ppp_gprs_apn_pap_info.sms_max_number_me = ppp_gprs_info.sms_max_number_me;		
	sync_gprs_info();
	ret = modem_at_cmd_ato(&ppp_handle, AT_REPEAT_MAX, AT_TIMEOUT_NOR, WAIT_RETURN);
	if(ret != AT_RET_OK)
	{
		PRINTF(".ato cmd fail.\r\n");
	}
	else
	{
		PRINTF(".back_to_data_mode.\n");
	}
	return AT_RET_OK;
}

/*
*hao.yunran
*2016.10.10
*dial GPRS, if successful, we start LCP Config
*/
u32_t gsm_sos_call_hand_up_by_caller()
{
	u32_t ret = AT_RET_OK;
	PRINTF(".sos_call.press_key_hand_up.\n");
	ret = modem_at_cmd_chup(&ppp_handle, AT_REPEAT_NOR, AT_TIMEOUT_NOR, NOT_WAIT);
	if(ret == AT_RET_OK)
	{
		
	}
	vTaskDelay(50);
	ret = modem_at_cmd_ato(&ppp_handle, AT_REPEAT_NOR, AT_TIMEOUT_NOR, WAIT_RETURN);
	if(ret != AT_RET_OK)
	{
		PRINTF(".ato cmd fail.\r\n");
	}
	else
	{
		PRINTF(".back_to_data_mode.\n");
	}
	return ret;
}

/*
*hao.yunran
*2016.10.10
*dial GPRS, if successful, we start LCP Config
*/
u32_t gsm_sos_call_hand_up_by_answer()
{
	u32_t ret = AT_RET_OK;
	PRINTF(".sos_call.end.by.answer.\n");
	modem_at_cmd_chup(&ppp_handle, AT_REPEAT_NOR, AT_TIMEOUT_MAX, NOT_WAIT);
	vTaskDelay(50);
	ret = modem_at_cmd_ato(&ppp_handle, AT_REPEAT_NOR, AT_TIMEOUT_NOR, WAIT_RETURN);
	if(ret != AT_RET_OK)
	{
		PRINTF(".ato cmd fail.\r\n");
	}
	else
	{
		PRINTF(".back_to_data_mode.\n");
	}
	return ret;
}

/*
*hao.yunran
*2016.10.10
*dial GPRS, if successful, we start LCP Config
*/
u32_t gsm_sos_call_hand_up_by_timeout()
{
	u32_t ret = AT_RET_OK;
	PRINTF(".sos_call.end.timeout.\n");
	modem_at_cmd_chup(&ppp_handle, AT_REPEAT_NOR, AT_TIMEOUT_NOR, NOT_WAIT);
	vTaskDelay(50);
	ret = modem_at_cmd_ato(&ppp_handle, AT_REPEAT_NOR, AT_TIMEOUT_NOR, WAIT_RETURN);
	if(ret != AT_RET_OK)
	{
		PRINTF(".ato cmd fail.\r\n");
	}
	else
	{
		PRINTF(".back_to_data_mode.\n");
	}
	return ret;
}

/*
*hao.yunran
*2016.10.10
*dial GPRS, if successful, we start LCP Config
*/
u32_t gsm_sos_call_hand_up_during_wait_uart()
{
	u32_t ret = AT_RET_OK;
	modem_at_cmd_chup(&ppp_handle, AT_REPEAT_NOR, AT_TIMEOUT_NOR, NOT_WAIT);
	vTaskDelay(50);
	ret = modem_at_cmd_ato(&ppp_handle, AT_REPEAT_NOR, AT_TIMEOUT_NOR, NOT_WAIT);
	if(ret != AT_RET_OK)
	{
		PRINTF(".ato cmd fail.\r\n");
	}
	else
	{
		PRINTF(".back_to_data_mode.\n");
	}
	set_all_param_to_data_mode();
	defer_log_uplink();
	PRINTF("..sos_call_hand_up_during_wait_uart..\r\n");			
	enable_sos_sms_interrupt();
	return ret;
}

/*
*hao.yunran
*2016.10.10
*dial GPRS, if successful, we start LCP Config
*/
u32_t gsm_sos_call_mode_change_fail()
{
	u32_t ret = AT_RET_OK;
	modem_at_cmd_chup(&ppp_handle, AT_REPEAT_NOR, AT_TIMEOUT_MAX, NOT_WAIT);
	PRINTF(".sos_call.hand_up_mode_change_fail.\n");
	vTaskDelay(50);
	ret = modem_at_cmd_ato(&ppp_handle, AT_REPEAT_NOR, AT_TIMEOUT_NOR, NOT_WAIT);
	if(ret != AT_RET_OK)
	{
		PRINTF(".ato cmd fail.\r\n");
	}
	else
	{
		PRINTF(".back_to_data_mode.\n");
	}
	defer_log_uplink(); 
	//SPKR_OFF();
	set_all_param_to_data_mode();
	PRINTF("..cannot.turn_at.or.sem_fail..\r\n");	
	enable_sos_sms_interrupt();
	return ret;
}

/*
*hao.yunran
*2016.10.10
*dial GPRS, if successful, we start LCP Config
*/
u32_t gsm_sos_call_hand_up_default()
{
	u32_t ret = AT_RET_OK;
	modem_at_cmd_chup(&ppp_handle, AT_REPEAT_NOR, AT_TIMEOUT_MAX, NOT_WAIT);
	PRINTF(".sos_call.hand_up_default.\n");
	vTaskDelay(50);
	ret = modem_at_cmd_ato(&ppp_handle, AT_REPEAT_NOR, AT_TIMEOUT_NOR, NOT_WAIT);
	if(ret != AT_RET_OK)
	{
		PRINTF(".ato cmd fail.\r\n");
	}
	else
	{
		PRINTF(".back_to_data_mode.\n");
	}
	return ret;
}

/*
*hao.yunran
*2016.10.10
*dial GPRS, if successful, we start LCP Config
*/
u32_t gsm_sos_call_cmd_fail()
{
	u32_t ret = AT_RET_OK;
	PRINTF(".sos_call.send error.\n");
	modem_at_cmd_chup(&ppp_handle, AT_REPEAT_NOR, AT_TIMEOUT_MAX, NOT_WAIT);
	vTaskDelay(50);
	modem_at_cmd_ato(&ppp_handle, AT_REPEAT_NOR, AT_TIMEOUT_MAX, NOT_WAIT);
	return ret;
}

/*
*hao.yunran
*2016.10.10
*dial GPRS, if successful, we start LCP Config
*/
u32_t ppp_uart_receive_fail(u32_t error)
{
	at_ret ret = AT_RET_OK;		

	if (error == kStatus_UART_RxHardwareOverrun)
	{
		/* Notify about hardware buffer overrun */
		if (kStatus_Success != UART_RTOS_Send(&ppp_handle, (uint8_t *)send_hardware_overrun, strlen(send_hardware_overrun)))
		{
			PRINTF("RxHardwareOverrun PPP\r\n");
			ret = AT_RET_FAIL;	
		}
	}
	if (error == kStatus_UART_RxRingBufferOverrun)
	{
		/* Notify about ring buffer overrun */
		if (kStatus_Success != UART_RTOS_Send(&ppp_handle, (uint8_t *)send_ring_overrun, strlen(send_ring_overrun)))
		{
			PRINTF("RxRingBufferOverrun PPP\r\n");
			ret = AT_RET_FAIL;
		}
	}
	return ret;
}

/*
*hao.yunran
*2016.10.10
*dial GPRS, if successful, we start LCP Config
*/
u32_t ppp_uart_init()
{
	u32_t ret = AT_RET_OK;
	PRINTF("\r\n^_^seq001.ppp init\r\n");
	ppp_gprs_info.ppp_gprs_current_seq = SEQ_001;
	PR_INFO("device init\r\n");
	PPP_GREEN_LED_INIT();
	//PPP_GREEN_LED_OFF();
	PPP_RED_LED_INIT();
	//PPP_RED_LED_OFF();	
	MODEM_POWER_INIT();
	MODEM_EMERG_RST_INIT();
#if (BOARD_NAME >= HW_V02)
	//K24_POWER_LED_INIT(); 
	//K24_POWER_LED_ON();
#endif
	MODEM_DCD_INIT();
	SPKR_SWT_GPIO_INIT();
	SPKR_SWT_OFF();
	SPKR_GPIO_INIT();
	SPKR_ON();
	//MODEM_RING_INIT();
	modem_uart_config.srcclk = CLOCK_GetFreq(MODEM_UART_CLKSRC);
	modem_uart_config.base = MODEM_UART;
    NVIC_SetPriority(MODEM_UART_RX_TX_IRQn, 6);

    if (0 > UART_RTOS_Init(&ppp_handle, &ppp_t_handle, &modem_uart_config))
    {
        PRINTF("Error during UART initialization.\r\n");
        //vTaskSuspend(NULL);
        ret = AT_RET_FAIL;
    }
    //lwip_init();
	tcpip_init(NULL, NULL);
    pppInit();

    MODEM_RING_SMS_INIT();
	Modem_Ring_Sms_Irq_Init();	 
	MODEM_SOS_INIT();
	Modem_SOS_Irq_Init();
	return ret;
}

/*
*hao.yunran
*2016.10.10
*dial GPRS, if successful, we start LCP Config
*/
u32_t ppp_protoc_negotiate()
{
	u32_t * fd = NULL;
    u32_t *linkStatusCtx = NULL;
	u32_t ppp_len=0;
	u32_t dcd_value=0;
	u32_t ret = AT_RET_OK;
	//size_t temp_recved;
	//u8_t ch[1];
	PRINTF("\r\n^_^seq009.ppp nego\r\n");
	ppp_gprs_info.ppp_gprs_current_seq = SEQ_009;
	PPP_GREEN_LED_ON();
	ppp_pd = pppOpen((void*)fd,  &ppp_state_callback, (void *)linkStatusCtx);/*send lcp req to peer*/
	pppSetNetif(ppp_pd);
	ppp_gprs_info.ppp_negot_start_or_end = PPP_NEGOTIATE_START;	
	while(1)
	{
		if (get_lwip_link_state() == PPP_ESTABLISHED)
		{
			PR_INFO("..ppp_nego_up..\r\n");
		    dns_setserver(0, &ppp_gprs_info.ppp_netif_dns1);
		    PRINTF(" IPv4 addr     : %u.%u.%u.%u \r\n", ((u8_t *)&ppp_gprs_info.ppp_netif_ipaddr)[0], ((u8_t *)&ppp_gprs_info.ppp_netif_ipaddr)[1],
		                        ((u8_t *)&ppp_gprs_info.ppp_netif_ipaddr)[2], ((u8_t *)&ppp_gprs_info.ppp_netif_ipaddr)[3]);
		    PRINTF(" IPv4 dns1     : %u.%u.%u.%u \r\n", ((u8_t *)&ppp_gprs_info.ppp_netif_dns1)[0], ((u8_t *)&ppp_gprs_info.ppp_netif_dns1)[1],
		                        ((u8_t *)&ppp_gprs_info.ppp_netif_dns1)[2], ((u8_t *)&ppp_gprs_info.ppp_netif_dns1)[3]);
		    ppp_gprs_info.ppp_negotiate_result = PPP_NEGOTIATE_OK;	
			ppp_gprs_info.ppp_negot_start_or_end = PPP_NEGOTIATE_END;
			ppp_gprs_info.ppp_relink_count = 0;
			ppp_gprs_info.http_ask_ppp_to_negotiate = 0;
			dcd_value = MODEM_DCD_READ();
			PRINTF("seq009..ppp_up..dcd=%d \n",dcd_value);
			PPP_GREEN_LED_ON();
			PPP_RED_LED_OFF();
			ppp_gprs_info.ppp_modem_init_flag = 0;
			ret = AT_RET_OK;
			break;
		}
		if(get_lwip_link_state() == PPP_DEAD)
		{
			PR_INFO("..ppp_nego_down..\r\n");
			//PPP_GREEN_LED_OFF();
			pppCloseAll(ppp_pd);
			pppInit();
			ppp_gprs_info.ppp_negotiate_result = PPP_NEGOTIATE_FAIL;
			ppp_gprs_info.ppp_negot_start_or_end = PPP_NEGOTIATE_END;
			dcd_value = MODEM_DCD_READ();
			PRINTF("seq009..PPP_DEAD..dcd=%d \n",dcd_value);
			ppp_gprs_info.ppp_relink_count++;
			set_lwip_link_state(PPP_NEGOTIATE);
			if(ppp_gprs_info.http_ask_ppp_to_negotiate == 1)
			{
				//ppp_gprs_info.http_ask_ppp_to_negotiate = 0;
				ret = AT_RET_FAIL_HTTP;
			}
			else
			{
				ret = AT_RET_PPP_DOWN;
			}
			break;
		}				
		UART_RTOS_Receive(&ppp_handle, ppp_rec, 1, &ppp_temp_recved);
		if (ppp_rec[0] == 'A') {
			PRINTF("..maybe AT cmd..\n");
		} 
		else if(ppp_rec[0] == 0x7E) 
		{
			//PPP_GREEN_LED_TOGGLE();
			ppp_len = 0;
			buf_ppp[ppp_len]	= ppp_rec[0];
			ppp_len++;
			
			while(1) {
				UART_RTOS_Receive(&ppp_handle, ppp_rec, 1, &ppp_temp_recved);
				buf_ppp[ppp_len]	= ppp_rec[0];
				if (ppp_rec[0] == 0x7E) 
				{
					/* 判断是否发生头尾字符颠倒  */
					if (ppp_len == 1)
					{
						PRINTF("..ppp_data_len:%d..\r\n", ppp_len);
						continue;
					}
					/* 一帧结束,传递给需要接收帧数据的对象	  */
					PRINTF(".ppp_input.\r\n ");
					ppp_len++;
					pppos_input(ppp_pd,  buf_ppp, ppp_len);
					//PPP_GREEN_LED_TOGGLE();
					break;
				}
				else 
				{
					ppp_len++;
					if(ppp_len > PPP_BUFFER_LEN)
					{
						break;
				}
			}
		}
		}
		else
		{
			PRINTF("unexpect data:%x\n", ppp_rec[0]);
		}
	}

	return ret;
}

/*
*hao.yunran
*2016.10.10
*dial GPRS, if successful, we start LCP Config
*/
u32_t ppp_packet_proc()
{
	u32_t ppp_len = 0;
	u32_t dcd_value = 0;
	u32_t error;
    u32_t ret = AT_RET_OK;

	PRINTF("\r\n^_^seq010.ppp packet\r\n");
	ppp_gprs_info.ppp_gprs_current_seq = SEQ_010;
	sos_incoming_call_status = NO_CALL;
	while(1)
	{
		if (get_lwip_link_state() == PPP_DEAD)
		{
			PR_INFO("..ppp_down..\r\n");
			//PPP_GREEN_LED_OFF();
			pppCloseAll(ppp_pd);
			pppInit();
			ppp_gprs_info.ppp_negotiate_result = PPP_NEGOTIATE_FAIL;			
			set_lwip_link_state(PPP_NEGOTIATE); 
			dcd_value = MODEM_DCD_READ();
			PRINTF("seq010..PPP_DEAD..dcd=%d \n",dcd_value);
			ret = AT_RET_PPP_DOWN;
			break;
		}
		dcd_value = MODEM_DCD_READ();
		if(dcd_value == 1)
		{
			PR_INFO("..ppp_down_dcd..\r\n");
			//PPP_GREEN_LED_OFF();
			if(get_lwip_link_state() != PPP_DEAD)
			{
				sifdown(ppp_pd);
			}			
			pppCloseAll(ppp_pd);
			pppInit();
			ppp_gprs_info.ppp_negotiate_result = PPP_NEGOTIATE_FAIL;
			set_lwip_link_state(PPP_NEGOTIATE); 
			PRINTF("seq010..dcd=1..ppp_down\n");
			ret = AT_RET_PPP_DOWN;
			break;
		}
		if(ppp_gprs_info.modem_need_at_mode == 1)
		{
			vTaskDelay(100);
			continue;
		}
		if(ppp_gprs_info.modem_need_apn_update == 1)
		{
			//PPP_GREEN_LED_OFF();
			//ppp_gprs_info.modem_need_apn_update = 0;
			set_lwip_link_state(PPP_NEGOTIATE); 
			ret = AT_RET_APN_UPDATE;
			return ret;
		}
		PPP_RED_LED_OFF();
		PPP_GREEN_LED_ON();
		if(SOS_CALL_IRQ == ppp_gprs_info.irq_for_sos_call)
		{
			//__asm("bkpt #0");
			PPP_RED_LED_ON();
			PPP_GREEN_LED_OFF();
			return AT_RET_SOS_CALL_UP;
		}
		//if((SMS_OR_INCOMING_CALL == ppp_gprs_info.irq_for_sms_or_incoming_call)
		//	&&(NOT_SUPPORT_INCOMING_CALL != ppp_gprs_apn_pap_info.incoming_call_answer_timeout))
		if(SMS_OR_INCOMING_CALL == ppp_gprs_info.irq_for_sms_or_incoming_call)
		{
			//PPP_RED_LED_ON();
			//PPP_GREEN_LED_OFF();
			return AT_RET_SMS_UPDATE_RING;
		}
		
		error = UART_RTOS_Receive_Timeout(&ppp_handle, ppp_rec, 1, (size_t *)&ppp_temp_recved, MAX_UART_REC_TIMEOUT);
        if (error == kStatus_Success)
        {
			if (ppp_rec[0] == 'A') {
				PRINTF("..maybe AT cmd..\n");
			} 
			else if(ppp_rec[0] == 0x7E) 
			{
				//PRINTF("...ppp_packet...\r\n");
				PPP_GREEN_LED_TOGGLE();
				//PPP_GREEN_LED_ON();
				ppp_len = 0;
				buf_ppp[ppp_len]	= ppp_rec[0];
				ppp_len++;
				
				while(1) 
				{
					if(ppp_gprs_info.modem_need_at_mode == 1)
					{
						PRINTF("continue.at.mode.\r\n");
						break;
					}
					error =  UART_RTOS_Receive_Timeout(&ppp_handle, ppp_rec, 1, (size_t *)&ppp_temp_recved,MAX_UART_REC_TIMEOUT);
					if (error == kStatus_Success)
					{
						buf_ppp[ppp_len]	= ppp_rec[0];
						if (ppp_rec[0] == 0x7E)
						{
							/* 判断是否发生头尾字符颠倒  */
							if (ppp_len == 1)
							{
								PRINTF("..ppp_len..%d\r\n", ppp_len);
								continue;
							}
							/* 一帧结束,传递给需要接收帧数据的对象	  */
							ppp_len++;
							PRINTF("..ppp_in..\r\n");
							pppos_input(ppp_pd,  buf_ppp, ppp_len);
							PPP_GREEN_LED_TOGGLE();
							//PPP_GREEN_LED_ON();
							break;
						}
						else 
						{
							ppp_len++;
							if(ppp_len > PPP_BUFFER_LEN)
							{
								PRINTF("..ppp_len..too_long\r\n");
								break;
							}
						}
					}
					else if(error == kStatus_Timeout)
					{
						//vTaskDelay(10);
						break;
					}
					else
					{
						ppp_uart_receive_fail(error);
					}
				}
			}
			else if(ppp_rec[0] == 0xFF)
			{
				error =  UART_RTOS_Receive_Timeout(&ppp_handle, ppp_rec, 1, (size_t *)&ppp_temp_recved,MAX_UART_REC_TIMEOUT);
				if (error == kStatus_Success)
				{
					if(ppp_rec[0] == 0x3)
					{
						PRINTF("...ppp_packet...ff...\r\n");
						PPP_GREEN_LED_TOGGLE();
						//PPP_GREEN_LED_ON();
						ppp_len = 2;
						buf_ppp[0]	= 0x7E;
						buf_ppp[1]	= 0xFF;
						buf_ppp[ppp_len]	= ppp_rec[0];
						ppp_len++;
						
						while(1) 
						{
							if(ppp_gprs_info.modem_need_at_mode == 1)
							{
								PRINTF("continue.at.mode.\r\n");
								break;
							}
							error =  UART_RTOS_Receive_Timeout(&ppp_handle, ppp_rec, 1, (size_t *)&ppp_temp_recved,MAX_UART_REC_TIMEOUT);
							if (error == kStatus_Success)
							{
								buf_ppp[ppp_len]	= ppp_rec[0];
								if (ppp_rec[0] == 0x7E)
								{
									/* 判断是否发生头尾字符颠倒  */
									if (ppp_len == 1)
									{
										PRINTF("..ppp_len..%d\r\n", ppp_len);
										continue;
									}
									/* 一帧结束,传递给需要接收帧数据的对象	  */
									ppp_len++;
									PRINTF("..ppp_in..\r\n");
									pppos_input(ppp_pd,  buf_ppp, ppp_len);
									PPP_GREEN_LED_TOGGLE();
									//PPP_GREEN_LED_ON();
									break;
								}
								else 
								{
									ppp_len++;
									if(ppp_len > PPP_BUFFER_LEN)
									{
										PRINTF("..ppp_len..too_long\r\n");
										break;
									}
								}
							}
							else if(error == kStatus_Timeout)
							{
								//vTaskDelay(10);
								break;
							}
							else
							{
								ppp_uart_receive_fail(error);
							}
						}

					}

				}
				else
				{
					PRINTF("UNEXPECT_0 ch: %02x\r\n", ppp_rec[0]);
				}
			}
			else
			{
				PRINTF("UNEXPECT ch: %02x\r\n", ppp_rec[0]);
			}
			
		}
		else if(error == kStatus_Timeout)
		{
			//vTaskDelay(10);
		}
		else
		{
			ppp_uart_receive_fail(error);
		}
	}

	return ret;
}

/*
*hao.yunran
*2016.10.10
*dial GPRS, if successful, we start LCP Config
*/
u32_t sms_update_or_ring_proc()
{
	FOTA_RQ_MSG msg;
    u32_t ret = AT_RET_OK;
	PRINTF("\r\n^_^seq011.sms_ring\r\n");
	if(pdFALSE == xSemaphoreTake(call_http_mqtt_sem, 1600U))
	{
		PRINTF(".sms_ring.take.sem.fail.\r\n");
		set_all_param_to_data_mode();
		enable_sos_sms_interrupt();
		if(ppp_gprs_info.ppp_gprs_current_seq == SEQ_010)
		{
			return AT_RET_NEXT_SEQ3;
		}
		else
		{
			return AT_RET_NEXT_SEQ4;
		}	
	}
	ret = gsm_turn_at_mode_wait_uart_ready(MODEM_MODE_SWITCH_TIMEOUT);	
	if(ret != AT_RET_OK)
	{
		gsm_incoming_call_mode_change_fail();	
		xSemaphoreGive(call_http_mqtt_sem);
		if(ppp_gprs_info.ppp_gprs_current_seq == SEQ_010)
		{
			return AT_RET_NEXT_SEQ3;
		}
		else
		{
			return AT_RET_NEXT_SEQ4;
		}
	}
	if(ppp_gprs_info.irq_for_sms_or_incoming_call == SMS_OR_INCOMING_CALL)
	{
		PRINTF("..sms.or.incoming_call..\r\n"); 
		ret = modem_at_cmd_cpms_incomingcall(&ppp_handle, AT_REPEAT_MAX, AT_TIMEOUT_MAX, WAIT_RETURN);
		//ret = modem_at_cmd_clcc(&ppp_handle, AT_REPEAT_NOR, AT_CLCC_TIMEOUT_NOR, WAIT_RETURN);
		if(ret == AT_RET_CALL_ING)
		{
			ppp_gprs_info.irq_for_sms_or_incoming_call = INCOMING_CALL_IRQ;
		}
		else if(ret == AT_RET_OK)
		{
			ppp_gprs_info.irq_for_sms_or_incoming_call = SMS_NEED_UPDATE;
		}
		else
		{
			gsm_incoming_call_mode_change_fail();	
			xSemaphoreGive(call_http_mqtt_sem);
			if(ppp_gprs_info.ppp_gprs_current_seq == SEQ_010)
			{
				return AT_RET_NEXT_SEQ3;
			}
			else
			{
				return AT_RET_NEXT_SEQ4;
			}
		}
		if((ppp_gprs_info.irq_for_sms_or_incoming_call == INCOMING_CALL_IRQ)
			&&(NOT_SUPPORT_INCOMING_CALL == ppp_gprs_apn_pap_info.incoming_call_answer_timeout))
		{
			modem_at_cmd_chup(&ppp_handle, AT_REPEAT_NOR, AT_TIMEOUT_MAX, NOT_WAIT);
			vTaskDelay(50);
			modem_at_cmd_ato(&ppp_handle, AT_REPEAT_NOR, AT_TIMEOUT_MAX, NOT_WAIT);
			PRINTF(".back_to_data_mode.\n");
			//SPKR_OFF();
			set_all_param_to_data_mode();
			enable_sos_sms_interrupt();
			xSemaphoreGive(call_http_mqtt_sem);
			if(ppp_gprs_info.ppp_gprs_current_seq == SEQ_010)
			{
				return AT_RET_NEXT_SEQ3;
			}
			else
			{
				return AT_RET_NEXT_SEQ4;
			}
		}
		else if((ppp_gprs_info.irq_for_sms_or_incoming_call == INCOMING_CALL_IRQ)
			&&(NOT_SUPPORT_INCOMING_CALL != ppp_gprs_apn_pap_info.incoming_call_answer_timeout))
		{					
			PPP_GREEN_LED_OFF();
			PPP_RED_LED_ON();
			msg.cmd = FW_SOS_INCOMING_CALL;
			xQueueSend(fota_rq_handle, &msg, 0);
			sos_incoming_call_status = CALL_ING;
			ppp_gprs_info.irq_for_sos_call = SOS_NO_CALL;
		    SPKR_ON();
			AUDIO_MSG_2G();
			GPIO_ClearPinsInterruptFlags(MODEM_SOS_GPIO, 1U << MODEM_SOS_GPIO_PIN);
			EnableIRQ(MODEM_SOS_IRQ);
			PR_INFO(".incoming_call.\r\n");	
			vTaskDelay(50);
			ret = modem_wait_incoming_call_answer_clcc(&ppp_handle);
			// 30s timeout pass, now ata call auto
			if(ret == AT_RET_CALL_DOWN_TIMER_OUT) 
			{
				gsm_incoming_call_auto_answer();
			}
			// press sos key to answer incoming call
			else if(ret == AT_RET_SOS_CALL_UP)
			{
				gsm_incoming_call_press_key_answer();
			}
			//the one who makes incoming call hands up the call
			else if(ret == AT_RET_CALL_DOWN)
			{
				gsm_incoming_call_hand_up_by_caller();
			}
			else
			{
				gsm_incoming_call_hand_up_default();
			}	
			defer_log_uplink();
			//SPKR_OFF();
			set_all_param_to_data_mode();
			enable_sos_sms_interrupt();
			xSemaphoreGive(call_http_mqtt_sem);
			if(ppp_gprs_info.ppp_gprs_current_seq == SEQ_010)
			{
				return AT_RET_NEXT_SEQ3;
			}
			else
			{
				return AT_RET_NEXT_SEQ4;
			}
		}
		else if(ppp_gprs_info.irq_for_sms_or_incoming_call == SMS_NEED_UPDATE)
		{			
			PRINTF("..this sms..\r\n");
			#if 0
			ret = modem_at_cmd_cpms(&ppp_handle, AT_REPEAT_MAX, AT_TIMEOUT_MAX, WAIT_RETURN);
			if(ret != AT_RET_OK)
			{
			
			}	
			vTaskDelay(60);
			//get apn from sms with new sms, if get it, turn to reset km. if get it fail, continue
			ret = modem_get_gprs_apn();
			if(ret == AT_RET_OK)
			{	
				//PRINTF("..this sms1..0x%x\r\n",seq_func_g);
				ret = gsm_sms_apn_analyse();
			}	
			//check sms for sos
			else
			{
				PR_INFO("..update_sos..\r\n");
				//UART_RTOS_Receive_NoBlocking(&ppp_handle, buf_ppp, sizeof(buf_ppp));
				ret = modem_get_sos_call_numb();
				if(ret == AT_RET_OK)
				{									
					ret = gsm_sms_sos_analyse();
				}	
				else
				{
					ret = gsm_sms_process_default();
				}
			}
			#endif
		}
		else 
		{
			PRINTF("..read sms fail..\r\n");
		}		
		defer_log_uplink();
		set_all_param_to_data_mode();
		enable_sos_sms_interrupt();
		xSemaphoreGive(call_http_mqtt_sem);
		if(ppp_gprs_info.ppp_gprs_current_seq == SEQ_010)
		{
			return AT_RET_NEXT_SEQ3;
		}
		else
		{
			return AT_RET_NEXT_SEQ4;
		}
	}
	else
	{
		defer_log_uplink();
		set_all_param_to_data_mode();
		enable_sos_sms_interrupt();
		xSemaphoreGive(call_http_mqtt_sem);
		if(ppp_gprs_info.ppp_gprs_current_seq == SEQ_010)
		{
			return AT_RET_NEXT_SEQ3;
		}
		else
		{
			return AT_RET_NEXT_SEQ4;
		}

	}
}

/*
*hao.yunran
*2016.10.10
*dial GPRS, if successful, we start LCP Config
*/
u32_t sos_call_proc()
{
	FOTA_RQ_MSG msg;
    u32_t ret = AT_RET_OK;
	PRINTF("\r\n^_^seq012.sos call\r\n");
	
	start_play_plan(AUDIO_SOS_CALL);
	if(pdFALSE == xSemaphoreTake(call_http_mqtt_sem, 1600U))
	{
		PRINTF("..sos take sem fail......\r\n");
		set_all_param_to_data_mode();
		enable_sos_sms_interrupt();
		if(ppp_gprs_info.ppp_gprs_current_seq == SEQ_010)
		{
			return AT_RET_NEXT_SEQ3;
		}
		else
		{
			return AT_RET_NEXT_SEQ4;
		}	
	}

	ret = gsm_turn_at_mode_wait_uart_ready(MODEM_MODE_SWITCH_TIMEOUT);	
	PRINTF("..uart sta..%d\r\n",ret);
	if(ret == AT_RET_FAIL)
	{
		gsm_sos_call_mode_change_fail();	
		xSemaphoreGive(call_http_mqtt_sem);
		if(ppp_gprs_info.ppp_gprs_current_seq == SEQ_010)
		{
			return AT_RET_NEXT_SEQ3;
		}
		else
		{
			return AT_RET_NEXT_SEQ4;
		}	
	}
	if(ret == AT_RET_SOS_CALL_DOWN)
	{
		gsm_sos_call_hand_up_during_wait_uart();		
		xSemaphoreGive(call_http_mqtt_sem);
		if(ppp_gprs_info.ppp_gprs_current_seq == SEQ_010)
		{
			return AT_RET_NEXT_SEQ3;
		}
		else
		{
			return AT_RET_NEXT_SEQ4;
		}	
		
	}
	
	if(ppp_gprs_info.irq_for_sos_call == SOS_CALL_IRQ)
	{	
		PR_INFO("..sos_call_ing..\r\n");
		SPKR_ON();
		AUDIO_MSG_2G();
		vTaskDelay(50);
		msg.cmd = FW_SOS_INCOMING_CALL;
		xQueueSend(fota_rq_handle, &msg, 0);
		sos_incoming_call_status = CALL_ING;
		ret = modem_at_cmd_sos(&ppp_handle, AT_REPEAT_MAX, AT_TIMEOUT_MAX, WAIT_RETURN);
		GPIO_ClearPinsInterruptFlags(MODEM_SOS_GPIO, 1U << MODEM_SOS_GPIO_PIN);
		EnableIRQ(MODEM_SOS_IRQ);
		/*atd sos call is calling*/
		if(ret == AT_RET_OK)
		{
			/* wait for sos call end and wait for irq of hand up */
			ret = modem_wait_sos_call_end_clcc(&ppp_handle);
			/* sos call timeout and end */
			if(ret == AT_RET_CALL_DOWN_TIMER_OUT)
			{
				gsm_sos_call_hand_up_by_timeout();
			}
			/* sos call end by press sos key */
			else if(ret == AT_RET_SOS_CALL_DOWN)
			{
				gsm_sos_call_hand_up_by_caller();
			}
			/* sos call end by anwser */
			else if(ret == AT_RET_CALL_DOWN)
			{
				gsm_sos_call_hand_up_by_answer();
			}
			else
			{
				gsm_sos_call_hand_up_default();
			}
		}
		else
		{
			gsm_sos_call_cmd_fail();
		}
		
		//PRINTF("hh..sos_call..3\n");
	}
	//PRINTF("..tt1..........%d\r\n",ppp_gprs_info.irq_for_sos_call);
	defer_log_uplink();
	//SPKR_OFF();
	set_all_param_to_data_mode();
	enable_sos_sms_interrupt();
	xSemaphoreGive(call_http_mqtt_sem);
	if(ppp_gprs_info.ppp_gprs_current_seq == SEQ_010)
	{
		return AT_RET_NEXT_SEQ3;
	}
	else
	{
		return AT_RET_NEXT_SEQ4;
	}

}

/*
*hao.yunran
*2016.10.10
*dial GPRS, if successful, we start LCP Config
*/
u32_t ppp_status_idle()
{
	u32_t ret = AT_RET_OK;
	
	PRINTF("\r\n^_^seq013..idle\r\n");
	ppp_gprs_info.ppp_gprs_current_seq = SEQ_013;
	PR_INFO("..ppp_status_idle..\r\n");
	while(1)
	{
		if(ppp_gprs_info.http_ask_ppp_to_negotiate == 1)
		{
			set_lwip_link_state(PPP_NEGOTIATE);
			return ret;
		}
		if(ppp_gprs_info.modem_need_apn_update == 1)
		{
			ppp_gprs_info.modem_need_apn_update = 0;
			//sync_gprs_info();
			ret = AT_RET_APN_UPDATE;
			return ret;
		}
		if(SOS_CALL_IRQ == ppp_gprs_info.irq_for_sos_call)
		{
			PPP_RED_LED_ON();
			PPP_GREEN_LED_OFF();
			return AT_RET_SOS_CALL_UP;
		}
		if(SMS_OR_INCOMING_CALL == ppp_gprs_info.irq_for_sms_or_incoming_call)
		{
			//PPP_RED_LED_ON();
			//PPP_GREEN_LED_OFF();
			return AT_RET_SMS_UPDATE_RING;
		}
		vTaskDelay(1000);

	}
	return ret;
}

/*
*hao.yunran
*2016.10.10
*dial GPRS, if successful, we start LCP Config
*/

u32_t seq_state_execute( SEQ_TBL  * const seq_func)
{
	u32_t result = SEQ_EX_OK;
	u32_t next_seq = NEXT_SEQ_001;
	//SEQ_TBL  *seq_func_p = seq_func;
	seq_func_g = seq_func;
	//PRINTF("....seq_func0..0x%x..0x%x\r\n",seq_func_p, seq_func);
	//PRINTF("..seq_func1....0x%x..\r\n",seq_func);
	result = seq_func->func(); 
	//PRINTF("....seq_func1..0x%x..0x%x\r\n",seq_func_p, seq_func);
	
	PRINTF("..seq ret.%d..\r\n",result);
	switch(result)
	{
		case SEQ_EX_OK:
			next_seq = NEXT_SEQ_001;
			//PRINTF("^_^next_seq.....%d\n",next_seq);
			break;

		case SEQ_EX_ERR:
			#if 0
			if(seq_func->seq_repeat_count == 0x0)
			{
				next_seq = NEXT_SEQ_002;
			}
			else
			{		
				seq_func->seq_repeat_count--;
				next_seq = NEXT_SEQ_003;
			}
			#endif
			next_seq = NEXT_SEQ_002;
			break;
		
		case SEQ_EX_REPEAT:
			break;
			
		case SEQ_EX_ERR_HTTP:
			next_seq = NEXT_SEQ_005;
			break;	
			
		case SEQ_EX_APN_UPDATE:
			next_seq = NEXT_SEQ_005;
			break;	
			
		case SEQ_EX_SMS_UPDATE_RING:
			next_seq = NEXT_SEQ_003;
			break;
			
		case SEQ_EX_SOS_CALL_UP:
			next_seq = NEXT_SEQ_004;
			break;
			
		case SEQ_EX_NEXT_SEQ3:
			next_seq = NEXT_SEQ_003;
			break;
			
		case SEQ_EX_NEXT_SEQ4:
			next_seq = NEXT_SEQ_004;
			break;
			
		case SEQ_EX_NEXT_SEQ5:
			next_seq = NEXT_SEQ_005;
			break;
			
		case SEQ_EX_PPP_DOWN:
			next_seq = NEXT_SEQ_005;
			break;
			
		case SEQ_EX_END:
			next_seq = NEXT_SEQ_005;
			break;
				
		default:

			break;
	}
	//PRINTF("^_^seq_state_execute...next_seq..%d,0x%x\n",next_seq,seq_func);
	//PRINTF("..seq ret.%d.next_seq.%d..\r\n",result,next_seq);	
	PRINTF("seq_func.0x%x.0x%x\r\n",seq_func_g, seq_func);	
	next_seq_no = seq_func_g->next_states_no[next_seq];
	//PRINTF("...next_seq_no..0x%x\n",next_seq_no);	
	return result;
}

/*
*hao.yunran
*2016.10.10
*dial GPRS, if successful, we start LCP Config
*/
u32_t ppp_state_machine(SEQ_TBL  *seq_func)
{
	u32_t result = SEQ_EX_OK; 
	//volatile SEQ_TBL  *seq_func_point = seq_func;
	while(1)
	{
		//PRINTF("..seq_func.............0x%x..next_seq_no=%d\r\n",seq_func, next_seq_no);
		result = seq_state_execute(seq_func + next_seq_no); 
		if(result == SEQ_EX_INIT_END)
		{
			break;
		}
	}
	return result;
}

/*
*hao.yunran
*2016.10.10
*dial GPRS, if successful, we start LCP Config
*/
SEQ_TBL ppp_state_func[]={
	/*                                                            next_seq1     next_seq2     next_seq3     next_seq4    next_seq5        status  repeat    curr_seq_numb*/
	/*                                                            case1-ok      case2-err     case3         case4        case5-end                                             */
    {ppp_uart_init,	 	         { {0x0} }, { SEQ_002, SEQ_001, SEQ_013, SEQ_013, SEQ_013 }, 0x0, 0x1}, /* SEQ_001 */  /* return ok/fail */
	{gprs_modem_init,		     { {0x0} }, { SEQ_003, SEQ_002, SEQ_002, SEQ_013, SEQ_013 }, 0x0, 0xF}, /* SEQ_002 */  /* return ok/fail */
	{gprs_check_sim,	         { {0x0} }, { SEQ_004, SEQ_002, SEQ_013, SEQ_013, SEQ_013 }, 0x0, 0x0}, /* SEQ_003 */  /* return ok/fail */
	{gprs_set_sms_storage,       { {0x0} }, { SEQ_005, SEQ_002, SEQ_013, SEQ_013, SEQ_013 }, 0x0, 0x0}, /* SEQ_004 */  /* return ok/fail */
	{gprs_get_gprs_sign_quility, { {0x0} }, { SEQ_006, SEQ_002, SEQ_013, SEQ_013, SEQ_013 }, 0x0, 0x0}, /* SEQ_005 */  /* return ok/fail */
	{gprs_get_gsm_status,        { {0x0} }, { SEQ_007, SEQ_002, SEQ_013, SEQ_013, SEQ_013 }, 0x0, 0x0}, /* SEQ_006 */  /* return ok/fail */
	{gprs_init_gprs,	         { {0x0} }, { SEQ_008, SEQ_002, SEQ_007, SEQ_013, SEQ_013 }, 0x0, 0x1}, /* SEQ_007 */  /* return ok/fail */
	{gprs_dialog_gprs,	         { {0x0} }, { SEQ_009, SEQ_002, SEQ_008, SEQ_013, SEQ_013 }, 0x0, 0x1}, /* SEQ_008 */  /* return ok/fail */
	{ppp_protoc_negotiate,       { {0x0} }, { SEQ_010, SEQ_002, SEQ_008, SEQ_009, SEQ_008 }, 0x0, 0x0}, /* SEQ_009 */  /* return ok/fail/fail_http */
	{ppp_packet_proc,	         { {0x0} }, { SEQ_011, SEQ_013, SEQ_011, SEQ_012, SEQ_007 }, 0x0, 0x0}, /* SEQ_010 */  /* return ok/fail/apn_update/sms_ring/sos_call */
	{sms_update_or_ring_proc,	 { {0x0} }, { SEQ_010, SEQ_010, SEQ_010, SEQ_013, SEQ_002 }, 0x0, 0x0}, /* SEQ_011 */  /* return next_seq3/next_seq4 */
	{sos_call_proc,			     { {0x0} }, { SEQ_010, SEQ_010, SEQ_010, SEQ_013, SEQ_002 }, 0x0, 0x0}, /* SEQ_012 */  /* return next_seq3/next_seq4 */
	{ppp_status_idle,	         { {0x0} }, { SEQ_002, SEQ_013, SEQ_011, SEQ_012, SEQ_002 }, 0x0, 0x0}, /* SEQ_013 */  /* return ok/fail/apn_update/sms_ring/sos_call */
};

/*
*hao.yunran
*2016.10.10
*dial GPRS, if successful, we start LCP Config
*/
void ppp_uart_task(void *pvParameters)
{
    PRINTF("pppInit\r\n");

	ppp_state_machine(ppp_state_func);

}

/*
*hao.yunran
*2017.1.19
*dial GPRS, if successful, we start LCP Config
*/
void hbb_power_status_task(void *pvParameters)
{
	u32_t Adc16_Value = 0;
	u32_t bat_volt_percent = 0;
	u32_t nPG_Value = 0;
	u32_t nCHG_Value = 0;
	char dc_v_data[16]="10201/vt:90";
	char acdc_data[12]="10302/s:AC";
	
    PRINTF("hbb_power_status_task\r\n");	
	OVERVOLTAGE_INIT();
	Power_HighV_Irq_Init();	
	BAT_nPG_INIT();
	BAT_nCHG_INIT();
	BAT_adc_init();
	Power_LowV_Init();
	while(1)
	{
		if(aging_test_for_ppp != 1)
		{
			if((ppp_gprs_info.ppp_gprs_current_seq != SEQ_010)&&(ppp_gprs_info.ppp_gprs_current_seq != SEQ_011))
			{
				ppp_gprs_info.ppp_gprs_timer_counter++;
				if(ppp_gprs_info.ppp_gprs_timer_counter > K24_STATUS_COUNT_MAX)
				{
					ppp_gprs_info.ppp_gprs_current_status = K24_STATUS_DEAD;
					PR_INFO("..ppp_status_dead..\r\n");
				}
			}
			else
			{
				ppp_gprs_info.ppp_gprs_timer_counter = 0;
				ppp_gprs_info.ppp_gprs_current_status = K24_STATUS_LIVE;
			}
			PRINTF("1..\r\n");

			if(ppp_gprs_info.ppp_uart_old_counter == ppp_gprs_info.ppp_uart_current_counter)
			{
				ppp_gprs_info.ppp_uart_timer_counter++;
				if(ppp_gprs_info.ppp_uart_timer_counter > K24_UART_STATUS_COUNT_MAX)
				{
					ppp_gprs_info.ppp_uart_current_status = K24_UART_STATUS_DEAD;
					PR_INFO("..ppp_uart_status_dead..\r\n");
				}

			}
			else
			{
				ppp_gprs_info.ppp_uart_old_counter = ppp_gprs_info.ppp_uart_current_counter;
				ppp_gprs_info.ppp_uart_timer_counter = 0;

			}
		}
		nPG_Value = BAT_nPG_READ();
		if(nPG_Value == 0x0)
		{	
			acdc_data[8]='D';
			Adc16_Value = BAT_adc_read();
			if(Adc16_Value < BAT_ADC_NOR_VALUE)
			{
				bat_volt_percent = (Adc16_Value*100)/BAT_ADC_NOR_VALUE;
				if(Adc16_Value < BAT_ADC_LOW_VALUE)
				{
					
					dc_v_data[9]='8';	
					if(Adc16_Value < BAT_ADC_SHUT_VALUE)
					{
						dc_v_data[10]='7';	
					}
					else
					{
						dc_v_data[10]='8';	
					}
					dc_v_data[11]='\0';
					bat_adc_counter++;
					if(bat_adc_counter > 1)
					{
						send_bat_status(WARNING,6,dc_v_data);
						bat_volt_low_flag = 1;
					}
					if(Adc16_Value < BAT_ADC_SHUT_VALUE)
					{
						dc_v_data[9]='8';			
						dc_v_data[10]='7';	
						dc_v_data[11]='\0';
						bat_adc_shut_counter++;
						if(bat_adc_shut_counter > 1)
						{
							PRINTF("..need shut down system..\r\n");			
							POWER_OFF_GPIO_INIT();
							POWER_OFF();
						}					
					}
				}
				else
				{
					bat_adc_counter = 0;
					bat_adc_shut_counter = 0;
					bat_volt_low_flag = 0;
					dc_v_data[10] = (bat_volt_percent%10) + '0';
					dc_v_data[9] = (bat_volt_percent/10) + '0';
					dc_v_data[11]='\0';
				}
				PRINTF("..dc_v_data:%s\r\n",dc_v_data);
				//send_bat_status(10,6,dc_v_data);
				PRINTF("..Adc16_Value..0x%x\r\n",Adc16_Value);		
			}
			else
			{
				bat_adc_counter = 0;
				bat_adc_shut_counter = 0;
				bat_volt_low_flag = 0;			
				dc_v_data[9]='1';
				dc_v_data[10]='0';
				dc_v_data[11]='0';			
				PRINTF("..dc_v_data:%s\r\n",dc_v_data);
				PRINTF("..Adc16_Value..0x%x\r\n",Adc16_Value);		
			}			
		}
		else
		{		
		
			acdc_data[8]='A';
		}
		if(power_ac_dc_status != nPG_Value)
		{
			power_ac_dc_status = nPG_Value;	
			send_bat_status(WARNING,6,acdc_data);
		}
		PRINTF("..acdc_data:%s\r\n",acdc_data);
		//send_bat_status(10,6,acdc_data);
		PRINTF("..nPG_Value..0x%x\r\n",nPG_Value);

		nCHG_Value = BAT_nCHG_READ();
		PRINTF("..nCHG_Value..0x%x\r\n",nCHG_Value);
		#if 0
		currentCNR = Power_LowV_Read();
		if(currentCNR == 0x0)
		{
			
		}
		else
		{

		}
		//send_bat_status(WARNING,6,ac_low_v_data);
		PRINTF("..currentCNR..0x%x\r\n",currentCNR);
		#endif
		vTaskDelay(HBB_POWER_CHECK_INTERVAL);
	}

}



#endif










