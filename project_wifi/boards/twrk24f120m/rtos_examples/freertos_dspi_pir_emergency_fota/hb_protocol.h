/*
 * HB_protocol.h
 *
 *  Created on: 2015/7/18
 *      Author: zhanghong
 */

#ifndef INCLUDE_HB_PROTOCOL_H_
#define INCLUDE_HB_PROTOCOL_H_

#include "FreeRTOS.h"
#include "queue.h"

#include "message.h"
#include "fsl_debug_console.h"


#define PACKET_BUFFER_LENGTH 24 //it should be 21, not define 24 for 4-byte alignment
#define PACKET_CONTENT_LENGTH 21
#define PACKET_CRC_LENGTH 18

#define MAX_SENSOR_COUNT 16
#define SENSOR_POLLING_INTERVAL       3600000
#define SENSOR_ALARM_POLLING_INTERVAL 10000


#define UART_RECEIVE_START    0
#define UART_RECEIVE_COMPLETE 3

   
typedef struct
{
  uint32_t fwv;
  uint32_t lc_addr;
  uint32_t rm_addr;
  uint32_t len;
  
} otah_t;

/*	<Watch dog enable>
 *
 * 	brief:Enable watch dog function.
 *
 * 	Note :Watch dog can not be disabled if this function is enabled.
 * 		  Due to 1Khz LPO clock,the timeout of watch dog is 1024 ms.
 * 		  When CPU goes into sleep mode(except VLLSx),watch dog counter
 * 		  re-initialized to zero and stop.When CPU exits from sleep,counter
 * 		  started from zero.
 * 		  This function is ignored in DEBUG mode.
 */

#define WTDG_enable() 	{\
		SIM_COPC |= 0x0C;\
}

/*  <Watch dog counter clear>
 * brief:clear watch dog counter.
 *
 */
#define WTDG_clear() 	{\
		SIM_SRVCOP = 0x55;\
		SIM_SRVCOP = 0xAA;\
}

#define PORT_A 0
#define PORT_B 1
#define PORT_OUTPUT 1
#define PORT_INPUT 0
#define Pull_enable PORT_PDD_PULL_ENABLE
#define Pull_disable PORT_PDD_PULL_DISABLE
#define Pin_ALT(x) (x << PORT_PCR_MUX_SHIFT)
#define PORT_REGISTER(x) (x?PORTB_BASE_PTR:PORTA_BASE_PTR)
#define FGPIO_REGISTER(x) (x?FPTB_BASE_PTR:FPTA_BASE_PTR)

/*	<Pin lite configuration>
 *
 * 	brief:configure setting upon IO with direction/pull resister/muxing mode
 *
 *	parameter:
 *	port :pin domain defined in macro PORT_A/PORT_B
 *	pin  :pin number in port domain.
 *  dir  :pin direction defined in macro PORT_OUTPUT/PORT_INPUT.
 * 	mux  :pin muxing mode.Please refer to KL02 chip spec on page 36.
 *		  This parameter was defined in macro Pin_ALT(x).
 *	pull :Pull resister enable/disable.
 *		  This parameter was defined in marco Pull_enable/Pull_disable.
 *	example:configure PTB5 to GPIO input and enable pullup resister.
 *			PIN_PORT_CONFIG(PORT_B,5,PORT_INPUT,Pin_ALT(1),Pull_enable)
 */
#define PIN_PORT_CONFIG(port,pin,dir,mux,pull)  {\
		FGPIO_REGISTER(port)->PDDR = (uint32_t)(FGPIO_REGISTER(port)->PDDR&\
		 (~(uint32_t)(1 << pin)) | ((uint32_t)(dir << pin)));\
		PORT_REGISTER(port)->PCR[pin] = (uint32_t)(PORT_REGISTER(port)->PCR[pin]&\
			(~(uint32_t)PORT_PCR_MUX_MASK) & (~(uint32_t)PORT_PCR_ISF_MASK)&\
			(~(uint32_t)PORT_PCR_PE_MASK) | (uint32_t)pull | (uint32_t)mux);\
}

#define MCU_release_SPI() {\
	PIN_PORT_CONFIG(PORT_B,0,PORT_INPUT,Pin_ALT(1),Pull_enable);\
	PIN_PORT_CONFIG(PORT_A,7,PORT_INPUT,Pin_ALT(1),Pull_enable);\
	PIN_PORT_CONFIG(PORT_A,5,PORT_INPUT,Pin_ALT(1),Pull_enable);\
	PIN_PORT_CONFIG(PORT_B,7,PORT_INPUT,Pin_ALT(1),Pull_enable);\
	PIN_PORT_CONFIG(PORT_B,6,PORT_INPUT,Pin_ALT(1),Pull_enable);\
}

extern uint32_t local_seq;
#ifdef DEBUG
extern int enable_rf_resp;
#endif
extern volatile int uart_receive_state;
extern uint8_t uart_cmd;
extern uint8_t swi;
#if BUILD_SENSOR_HUB
extern UART_HUB_MSG uart_msg;
#else
extern UART_SENSOR_MSG uart_msg;
#endif
extern RF_MSG send_packet;
extern RF_MSG recv_packet;

extern uint32_t serial_n;
extern const uint32_t fw_v_major;
extern const uint32_t fw_v_minor;

void init_state();
void init_A7139();
uint16_t crc_cal(uint8_t* ptr, uint32_t len);

void sleep_with_led_out();
void lp_sleep_for_ms(int ms);
void lp_sleep_for_s(int s);
void random_delay();

extern const uint32_t battery[];
extern uint16_t saved_lptmr_ms;
extern uint16_t check_batt_count;
extern uint16_t pir_int_count;
extern uint8_t  pir_que[];
extern uint8_t is_battery_too_low;
extern uint16_t rf_rx_standby;
extern uint16_t rf_rx_standby_time;
extern uint8_t RX_TIMEOUT_FLAG;
extern uint8_t RX_5MS_FLAG;
extern uint8_t RX_WAKEUP_FLAG;
extern SENSOR_STATUS_STORAGE_BLOCK sensor_status[MAX_SENSOR_COUNT];

extern inline int get_alarm_status(int index)
{
	return sensor_status[index].send_message.alarm_status;
}

int sensor_get_index_id(uint32_t id);

int check_boot_battery();
int check_battery();
void rf_wait();
void rf_receive();
void rf_send_then_wait(RF_MSG* rf_msg);
void rf_send_then_receive(RF_MSG* rf_msg);
void rf_send_then_receive_no_preamble(RF_MSG* rf_msg);
uint8_t rf_check_message();
void rf_send_image(otah_t);
void rf_recv_image();
void rf_reply_sensor_info_then_wait();
void cancel_security(void);
uint8_t enter_security(void);
uint8_t uart_check_message();
void uart_receive_message(void* buf, uint16_t count);
void uart_send_message(void* buf, uint32_t count);
void debug_printf(const char * fmt,...);
#if BUILD_SENSOR_HUB
int  uart_recv_image();
#else
void  uart_recv_image();
#endif
#ifdef DEBUG
#define DBG_MESSAGE(fmt, ...) PRINTF(fmt, ##__VA_ARGS__)
#else
#define DBG_MESSAGE(...)
#endif

void handle_preamble_timeout();
void handle_preamble_start();

extern QueueHandle_t log_msg_queue;
extern QueueHandle_t rx_msg_queue;
extern QueueHandle_t tx_sl_queue;
extern QueueHandle_t rx_sl_queue;
extern QueueHandle_t tx_plc_queue;
extern QueueHandle_t rx_plc_queue;

#endif /* INCLUDE_HB_PROTOCOL_H_ */
