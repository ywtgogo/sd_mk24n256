#ifndef UART_PPP_H
#define UART_PPP_H
#include "semphr.h"
#include "timers.h"

#include "pppif.h"
#include "ip_addr.h"

#define K24_ADC16_BASE 					ADC0
#define K24_ADC16_CHANNEL_GROUP    		0U
#define K24_ADC16_USER_CHANNEL    		8U
#define HBB_POWER_CHECK_INTERVAL        120000
#define PPP_GPRS_RQ_LENGTH 				16
#define PPP_RELINK_MAX 					8
#define PPP_NEGOTIATE_OK 				1
#define PPP_NEGOTIATE_FAIL 				0
#define PPP_NEGOTIATE_START 			0
#define PPP_NEGOTIATE_END 				1
#define HTTP_ASK_PPP_START_NEGOT     	1
#define HTTP_ASK_PPP_STOP_NEGOT 		0
#define PPP_DIAL_TIMEOUT 		        45000//ms
#define PPP_BUFFER_LEN 		        	1600
#define K24_STATUS_COUNT_MAX 		    6
#define K24_STATUS_LIVE 		       	0
#define K24_STATUS_DEAD 		        1
#define K24_UART_CURRENT_COUNT_MAX 		0xfffffff0
#define K24_UART_STATUS_COUNT_MAX 		36
#define K24_UART_STATUS_LIVE 		    0
#define K24_UART_STATUS_DEAD 		    1
#define K24_UART_STATUS_SEND 		    2
#define MODEM_MODE_SWITCH_TIMEOUT 		3
#define BAT_ADC_NOR_VALUE 		        0xCD0  //4.2---2.857
#define BAT_ADC_LOW_VALUE 		        0xC36  //3.7---2.517
#define BAT_ADC_SHUT_VALUE 		        0xC0D  //3.65---2.483
#define NOT_SUPPORT_INCOMING_CALL       0
#define NO_CALL       					0
#define CALL_ING       					1

enum sms_incoming_call_status
{
	NO_SMS_OR_INCOMING_CALL = 0, // 
	SMS_OR_INCOMING_CALL = 1, // 
	INCOMING_CALL_IRQ, // 
	SMS_NEED_UPDATE, // 
};

enum sos_call_status
{
	SOS_NO_CALL = 0, // 
	SOS_CALL_IRQ, // 
	SOS_CALL_SET_UP, // 
	SOS_CALL_HAND_UP, // 
};

enum ppp_gprs_cmd
{
	PPP_GPRS_SMS_UPDATE = 1, // 
	PPP_GPRS_SOS_CALL, // 
	PPP_GPRS_SOS_CALL_HAND_UP = 3, // 
	PPP_GPRS_GET_INFO, // 
	PPP_GPRS_GET_CSQ = 5,  // 
	PPP_GPRS_GET_SMS,  // 
	PPP_GPRS_GET_CREG = 7,  // 
	PPP_GPRS_GET_HIGHV = 8,  // 
};

typedef struct
{
	//enum ppp_gprs_cmd cmd;
	u32_t cmd;

}ppp_gprs_rq_msg_conf;

typedef struct _ppp_gprs_info_config
{
    SemaphoreHandle_t ppp_http_mqtt_sem;                /*  */
    TimerHandle_t ppp_gprs_timer;                /* */	
	QueueHandle_t	ppp_gprs_queue;
	u32_t ppp_relink_count;
	u32_t require_ac_adc_interval;
	u32_t sms_storage_type;
	u32_t sms_max_number_me;
	u32_t sms_current_number_me;
	u32_t sms_max_number_sm;
	u32_t sms_current_number_sm;
	u32_t signal_quility;
	u32_t ppp_gprs_timer_counter;
	u32_t ppp_gprs_current_seq;
	u32_t ppp_gprs_current_status;
	u32_t ppp_uart_current_counter;
	u32_t ppp_uart_old_counter;
	u32_t ppp_uart_timer_counter;
	u32_t ppp_uart_current_status;
	u32_t ppp_negotiate_result;
	u32_t ppp_negot_start_or_end;
	u32_t http_ask_ppp_to_negotiate;
	u32_t modem_need_at_mode;
	u32_t modem_need_apn_update;
	u32_t modem_need_sos_update;
	u32_t irq_for_sms_or_incoming_call;
	u32_t irq_for_sos_call;
	u32_t ppp_modem_init_flag;
	u32_t incoming_call_answer_timeout;
	ip_addr_t ppp_netif_ipaddr;
	ip_addr_t ppp_netif_netmask;
	ip_addr_t ppp_netif_gw;
	ip_addr_t ppp_netif_dns1;
	ip_addr_t ppp_netif_dns2;
	CHAR gprs_apn[32];
	CHAR ppp_pap_user[32];
	CHAR ppp_pap_passwd[32];
	CHAR sos_call[32];
}ppp_gprs_info_conf;
typedef struct _ppp_gprs_apn_pap_info_config
{
    CHAR gprs_apn[32];
	CHAR ppp_pap_user[32];
	CHAR ppp_pap_passwd[32];
	CHAR sos_call[32];
	u32_t incoming_call_answer_timeout;
	u32_t sms_max_number_me;
	u32_t sms_curr_number_me;
	u32_t pad;
}ppp_gprs_apn_pap_info_conf
#ifdef __GNUC__
	__attribute__((aligned(8)))
#endif
;

u32_t ppp_check_status(void);
u32_t update_incoming_call_auto_answer_timeout(u32_t timeout);
u32_t update_sos_call_number(CHAR* sos_number);
u32_t BAT_adc_read();
extern ppp_gprs_info_conf ppp_gprs_info;
extern ppp_gprs_apn_pap_info_conf ppp_gprs_apn_pap_info;
extern SemaphoreHandle_t call_http_mqtt_sem;				/*	*/

#endif
