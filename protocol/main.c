/* ###################################################################
**     Filename    : main.c
**     Project     : smart_sensor
**     Processor   : MKL02Z32VFK4
**     Version     : Driver 01.01
**     Compiler    : GNU C Compiler
**     Date/Time   : 2015-07-04, 20:51, # CodeGen: 0
**     Abstract    :
**         Main module.
**         This module contains user's application code.
**     Settings    :
**     Contents    :
**         No public methods
**
** ###################################################################*/
/*!
** @file main.c
** @version 01.01
** @brief
**         Main module.
**         This module contains user's application code.
*/         
/*!
**  @addtogroup main_module main module documentation
**  @{
*/         
/* MODULE main */


/* Including needed modules to compile this module/procedure */

/* User includes (#include below this line is not maintained by Processor Expert)*/

#include <stddef.h>
/* Kernel includes. */
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "timers.h"
#include "fsl_device_registers.h"
#include "fsl_debug_console.h"
#include "board.h"
#include "semphr.h"
#include "message.h"
#include "event_groups.h"

#include "pin_mux.h"
#include "clock_config.h"
#include "SPI_A7139.h"
#include "uplink_protocol.h"
#include "spiflash_config.h"
#include "fota.h"
#include "hb_protocol.h"
#include "uart_ppp.h"
#include "log_task.h"
#include "rtc_fun.h"

#include "mqtt_task.h"

#ifdef PLC
#include "uart_plc.h"
#include "plc_if.h"
#include "plc_k24_api.h"
#endif

//#ifdef WIFI_MODULE
#include"wifi_module.h"
//#endif
xTaskHandle pvCreatedTask_Http_client_netconn;
xTaskHandle pvCreatedTask_Rf_get_message_task;
xTaskHandle pvCreatedTask_Sensor_status_task;
xTaskHandle pvCreatedTask_Fota_task;
xTaskHandle pvCreatedTask_Uart_task;
xTaskHandle pvCreatedTask_Hb_task;
xTaskHandle pvCreatedTask_log_task;
xTaskHandle pvCreatedTask_wifi_task;
xTaskHandle pvCreatedTask_wifi_key_check_task;
xTaskHandle pvCreatedTask_wifi_led_status_task;
xTaskHandle pvCreatedTask_Ppp_Uart_task;
xTaskHandle pvCreatedTask_KickWTC;
xTaskHandle pvCreatedTask_hbb_power_status_task;
xTaskHandle pvCreatedTask_plc_get_message_task;
xTaskHandle pvCreatedTask_plc_status_task;
xTaskHandle pvCreatedTask_plc_bridge_task;
xTaskHandle pvCreatedTask_plc_fota_task;
xTaskHandle pvCreatedTask_mqtt_task;

void sensor_status_task(void *pvParameters);
void uart_task(void *pvParameters);
void demo_HubInfoRW_task(void *pvParameters);
void demo_HubUpLink_task(void *pvParameters);
void ppp_uart_task(void *pvParameters);
void hbb_power_status_task(void *pvParameters);
void rf_get_message_task(void *pvParameters);
void DrvPlcFsm_task(void *pvParameters);
void plc_get_msg_task(void *pvParameters);
#ifdef BRIDGE
void plc_fota_task(void *pvParameters);
#else
void fota_task(void *pvParameters);
#endif
void hb_task(void *pvParameters);
void http_client_netconn_thread(void *arg);
void mqtt_task(void *pvParameters);
void log_task(void *pvParameters);
void WatchdogTask( void *pvParameters );
void wifi_task(void *pvParameters);
void wifi_key_check_task(void *pvParameters);
void wifi_led_status_task(void *pvParameters);
void user_check_sys_task(void * pvParameters);
void audio_feed_task(void * pvParameters);
void power_key_task(void *);
#ifdef PRODUCE_TEST
void at_run_task(void *p);
void at_uart_task(void *pvParameters);
#endif

#ifdef WIFI_MODULE

 QueueHandle_t wifi_rssi_queue;
 QueueHandle_t wifi_ver_info_queue;
 QueueHandle_t wifi_config_info_queue;

QueueHandle_t check_wifi_con_state_queue = NULL;
QueueHandle_t get_wifi_queue = NULL;
SemaphoreHandle_t wifi_send_buf_sem_mutex = NULL;
SemaphoreHandle_t wifi_parse_url_sem_mutex = NULL;
QueueHandle_t wifi_smartconifg_queue = NULL;
QueueHandle_t wifi_parse_url_queue[DNS_SOCKET_MAX] = {NULL};
QueueHandle_t wifi_socket_discon_queue[WIFI_SOCKET_MAX] = {NULL};
QueueHandle_t wifi_socket_con_queue[WIFI_SOCKET_MAX] = {NULL};
QueueHandle_t wifi_socket_write_queue[WIFI_SOCKET_MAX] = {NULL};
QueueHandle_t wifi_socket_read_queue[WIFI_SOCKET_MAX] = {NULL};


#endif

QueueHandle_t log_msg_queue = NULL;
QueueHandle_t rx_msg_queue = NULL;
QueueHandle_t tx_sl_queue = NULL;
QueueHandle_t rx_sl_queue = NULL;
QueueHandle_t uplink_q_handle = NULL;
QueueHandle_t plc_pkgs_queue;
QueueHandle_t plc_status_queue;
#ifdef PLC
QueueHandle_t plc_msg_queue = NULL;
QueueHandle_t plc_data_queue = NULL;
#endif
#ifdef BRIDGE
QueueHandle_t plc_to_bridge_queue = NULL;
QueueHandle_t plc_fota_queue = NULL;
#endif
#ifdef MQTT
QueueHandle_t mqtt_send_q_handle = NULL;
#endif

QueueHandle_t log_rq_handle = NULL;
EventGroupHandle_t  hbbeventgroup = NULL;
SemaphoreHandle_t http_mqtt_socket_sem = NULL;
EventGroupHandle_t PowerKeyEventGroup = NULL;

void system_sync_init(void)
{
    rf_message_sem = xSemaphoreCreateBinary();
    if (rf_message_sem == NULL)
    {
        PRINTF("xSemaphore_producer creation failed.\r\n");
        vTaskSuspend(NULL);
    }
	xSemaphoreGive(rf_message_sem);

	rx_msg_queue = xQueueCreate(5, sizeof(RF_MSG));
#ifndef BRIDGE
    hb_task_sem = xSemaphoreCreateBinary();
    if (hb_task_sem == NULL)
    {
        PRINTF("hb_task_sem xSemaphore_producer creation failed.\r\n");
        vTaskSuspend(NULL);
    }
    uplink_q_handle = xQueueCreate(UPLINK_Q_LENGTH,sizeof(UPLINK_Q_MSG));
    if(uplink_q_handle == NULL)
    	PRINTF("can't Create uplink queue\r\n");
    fota_rq_handle = xQueueCreate(FOTA_RQ_LENGTH,sizeof(FOTA_RQ_MSG));
	if(fota_rq_handle == NULL)
		PRINTF("can't Create fota_rq queue\r\n");

#endif
#ifdef WIFI_MODULE

	  wifi_rssi_queue =  xQueueCreate(1, sizeof(uint8_t));
	  wifi_ver_info_queue =  xQueueCreate(1, sizeof(wifi_ver_info));
	  wifi_config_info_queue =  xQueueCreate(1, sizeof(struct station_config));

	check_wifi_con_state_queue = xQueueCreate(1, sizeof(wifi_cmd_msg_t));
	get_wifi_queue = xQueueCreate(2,sizeof(wifi_status_msg_t));
	wifi_send_buf_sem_mutex = xSemaphoreCreateMutex();
	wifi_parse_url_sem_mutex = xSemaphoreCreateMutex();
	wifi_smartconifg_queue = xQueueCreate(1,sizeof(wifi_cmd_msg_t));
	for(int i = 0; i < WIFI_SOCKET_MAX; i++)
	{
		wifi_parse_url_queue[i] =  xQueueCreate(1,sizeof(PARSE_URL_ACK_T));
		wifi_socket_discon_queue[i] =  xQueueCreate(1,sizeof(WIFI_DISCON_T));
		wifi_socket_con_queue[i] = xQueueCreate(1,sizeof(WIFI_CON_T));
		wifi_socket_write_queue[i] = xQueueCreate(1,sizeof(WIFI_DISCON_T));
		wifi_socket_read_queue[i] =  xQueueCreate(1,sizeof(WIFI_DISCON_T));
	}



#endif

#ifdef PLC
	plc_status_queue  = xQueueCreate(1, sizeof(uint8_t *));
    plc_pkgs_queue      = xQueueCreate(3, sizeof(uint8_t *));
	plc_msg_queue = xQueueCreate(PLC_Q_LENGTH, PLC_PACKAGE_LEN);
	plc_data_queue = xQueueCreate(PLC_DATA_Q_LENGTH, sizeof(uint8_t *));
	plc_slist_back_sem = xSemaphoreCreateBinary();
    if (plc_slist_back_sem == NULL)
    {
        PRINTF("plc_slist_back_sem creation failed.\r\n");
        vTaskSuspend(NULL);
    }
    plc_sensor_query_back_sem = xSemaphoreCreateBinary();
    if (plc_sensor_query_back_sem == NULL)
    {
        PRINTF("plc_sensor_query_back_sem creation failed.\r\n");
        vTaskSuspend(NULL);
    }

    BridgeFwReqAckEventGroup = xEventGroupCreate();
    if (BridgeFwReqAckEventGroup == NULL)
		PRINTF("can't Create BridgeFwReqAckEventGroup\r\n");

#endif
#ifdef BRIDGE
	plc_to_bridge_queue = xQueueCreate(PLC_Q_LENGTH, PLC_PACKAGE_LEN);
	plc_fota_queue = xQueueCreate(PLC_Q_LENGTH, PLC_PACKAGE_LEN);

	plc_fota_block_sem = xSemaphoreCreateBinary();
	if (plc_fota_block_sem == NULL)
	{
	    PRINTF("plc_fota_block_sem  creation failed.\r\n");
	    vTaskSuspend(NULL);
	}

#endif
#ifdef MQTT
    mqtt_send_q_handle = xQueueCreate(MQTT_SEND_Q_LENGTH,sizeof(MQTT_SEND_Q_MSG));
        if(mqtt_send_q_handle == NULL)
        	PRINTF("can't Create mqtt queue\r\n");
#endif

	log_rq_handle = xQueueCreate(MAX_SENSOR_COUNT,sizeof(LOG_RQ_MSG));
	if(log_rq_handle == NULL)
		PRINTF("can't Create log_rq_handle queue\r\n");
#ifndef BRIDGE
	ppp_gprs_info.ppp_gprs_queue = xQueueCreate(PPP_GPRS_RQ_LENGTH,sizeof(ppp_gprs_rq_msg_conf));
	if(ppp_gprs_info.ppp_gprs_queue == NULL)
		PRINTF("can't Create ppp_gprs_queue\r\n");
	ppp_gprs_info.ppp_http_mqtt_sem = xSemaphoreCreateMutex();
	if(ppp_gprs_info.ppp_http_mqtt_sem == NULL)
		PRINTF("can't Create ppp_gprs_at_sem\r\n");

	call_http_mqtt_sem= xSemaphoreCreateMutex();
	if(call_http_mqtt_sem == NULL)
		PRINTF("can't Create call_http_mqtt_sem\r\n");
#ifdef WIFI_MODULE		
	http_mqtt_socket_sem= xSemaphoreCreateMutex();
	if(http_mqtt_socket_sem == NULL)
		PRINTF("can't Create http_mqtt_socket_sem\r\n");
#endif
	
#endif
    hbbeventgroup = xEventGroupCreate();
    if (hbbeventgroup == NULL)
		PRINTF("can't Create hbbeventgroup\r\n");

    PowerKeyEventGroup = xEventGroupCreate();
    if (PowerKeyEventGroup == NULL)
		PRINTF("can't Create PowerKeyEventGroup\r\n");
}

int main(void)
{

    /* Init the boards */
	BOARD_InitPins();
    BOARD_BootClockRUN();
    BOARD_InitDebugConsole();

#ifdef    CPU_MK24FN256VDC12
	#ifndef WIFI_MODULE
    BQ25050_nPG_Irq_Init();
    BQ25050_MaxOutputSet();
	#endif
#endif

    rtc_init();
    system_sync_init();
    PRINTF("%s %d\r\n", __func__, __LINE__);

    SPI_A7139_INIT();
  	spi_flash_init();
    FLASH_RTOS_Init(&pflash_handle);

#ifdef	PRODUCE_TEST
    get_hbb_info();

    xTaskCreate(at_uart_task, "At_uart_task", configMINIMAL_STACK_SIZE * 32, NULL, tskIDLE_PRIORITY + 8, &pvCreatedTask_Uart_task);
    xTaskCreate(audio_feed_task, "Audio_feed_task", configMINIMAL_STACK_SIZE*6, NULL, tskIDLE_PRIORITY + 2, NULL);
//    xTaskCreate(rf_get_message_task, "Rf_get_message_task", configMINIMAL_STACK_SIZE * 2, NULL, tskIDLE_PRIORITY + 9, NULL);
    xTaskCreate(at_run_task, "At_run_task", configMINIMAL_STACK_SIZE * 32, NULL, tskIDLE_PRIORITY + 6, NULL);
#else

#ifndef BRIDGE
	 get_hbb_info();

	//httpclient_init();
#ifdef MQTT
    xTaskCreate(mqtt_task, "Mqtt_task", configMINIMAL_STACK_SIZE * 8, NULL, tskIDLE_PRIORITY + 7, &pvCreatedTask_mqtt_task);
#endif
	xTaskCreate(http_client_netconn_thread, "Http_client_netconn_task", configMINIMAL_STACK_SIZE * 25, NULL, tskIDLE_PRIORITY + 7, &pvCreatedTask_Http_client_netconn);
	xTaskCreate(fota_task, "Fota_task", configMINIMAL_STACK_SIZE * 18, NULL, tskIDLE_PRIORITY + 2, &pvCreatedTask_Fota_task);
	xTaskCreate(hb_task, "Hb_task", configMINIMAL_STACK_SIZE * 2, NULL, tskIDLE_PRIORITY + 1, &pvCreatedTask_Hb_task);
    xTaskCreate(uart_task, "Uart_task", configMINIMAL_STACK_SIZE * 8, NULL, tskIDLE_PRIORITY + 5, &pvCreatedTask_Uart_task);
#endif
    xTaskCreate(rf_get_message_task, "Rf_get_message_task", configMINIMAL_STACK_SIZE * 2, NULL, tskIDLE_PRIORITY + 8, &pvCreatedTask_Rf_get_message_task);
    xTaskCreate(sensor_status_task, "Sensor_status_task", configMINIMAL_STACK_SIZE * 8, NULL, tskIDLE_PRIORITY + 3, &pvCreatedTask_Sensor_status_task);
    xTaskCreate(log_task, "Log_task", configMINIMAL_STACK_SIZE * 3, NULL, tskIDLE_PRIORITY + 1, &pvCreatedTask_log_task);
#ifdef PLC

    xTaskCreate(DrvPlcFsm_task, "Drv_Plc_Fsm_task", configMINIMAL_STACK_SIZE * 3, NULL, tskIDLE_PRIORITY + 7, NULL);       //状态机优先级需要高于TX
    xTaskCreate(plc_get_msg_task, "Plc_get_msg_task", configMINIMAL_STACK_SIZE * 4, NULL, tskIDLE_PRIORITY + 7, NULL);
    xTaskCreate(plc_get_message_task, "plc_get_message_task", configMINIMAL_STACK_SIZE * 9, NULL, tskIDLE_PRIORITY + 8, &pvCreatedTask_plc_get_message_task);
    xTaskCreate(plc_status_task, "plc_status_task", configMINIMAL_STACK_SIZE * 8, NULL, tskIDLE_PRIORITY + 5, &pvCreatedTask_plc_status_task);
#ifdef BRIDGE
    xTaskCreate(plc_bridge_task, "plc_bridge_task", configMINIMAL_STACK_SIZE*8, NULL, tskIDLE_PRIORITY + 3, &pvCreatedTask_plc_bridge_task);
    xTaskCreate(plc_fota_task, "plc_fota_task", configMINIMAL_STACK_SIZE * 8, NULL, tskIDLE_PRIORITY + 4, &pvCreatedTask_plc_fota_task);
#endif
#endif

#ifdef CPU_MK24FN256VDC12

#ifdef WIFI_MODULE
    xTaskCreate(wifi_task, "Wifi_task", configMINIMAL_STACK_SIZE * 20, NULL, tskIDLE_PRIORITY + 8, pvCreatedTask_wifi_task);
	xTaskCreate(wifi_key_check_task, "wifi_key_check_task", configMINIMAL_STACK_SIZE * 2, NULL, tskIDLE_PRIORITY + 8, pvCreatedTask_wifi_key_check_task);
	xTaskCreate(wifi_led_status_task, "wifi_led_status_task", configMINIMAL_STACK_SIZE * 2, NULL, tskIDLE_PRIORITY + 5, pvCreatedTask_wifi_key_check_task);
#endif

#ifndef BRIDGE
#ifndef WIFI_MODULE
 	xTaskCreate(ppp_uart_task, "Ppp_uart_task", configMINIMAL_STACK_SIZE * 13, NULL, tskIDLE_PRIORITY + 8, &pvCreatedTask_Ppp_Uart_task);
	xTaskCreate(hbb_power_status_task, "hbb_power_status_task", configMINIMAL_STACK_SIZE * 3, ( void * ) NULL, tskIDLE_PRIORITY + 5, &pvCreatedTask_hbb_power_status_task );
	xTaskCreate(audio_feed_task, "Audio_feed_task", configMINIMAL_STACK_SIZE*6, NULL, tskIDLE_PRIORITY + 2, NULL);
#endif
#endif
	xTaskCreate( WatchdogTask, "KickWTC", configMINIMAL_STACK_SIZE * 2, ( void * ) NULL, tskIDLE_PRIORITY + 9, &pvCreatedTask_KickWTC );
	xTaskCreate( power_key_task, "Power_key_task", configMINIMAL_STACK_SIZE * 2, ( void * ) NULL, tskIDLE_PRIORITY + configMAX_PRIORITIES - 1, NULL );
#endif

	xTaskCreate(user_check_sys_task, "User_check_sys_task", configMINIMAL_STACK_SIZE * 3, NULL, tskIDLE_PRIORITY + configMAX_PRIORITIES - 1, NULL);

#endif	/* end nodefine PRODUCE_TEST */

    vTaskStartScheduler();

    for(;;){}
  /*** Processor Expert end of main routine. DON'T WRITE CODE BELOW!!! ***/
} /*** End of main routine. DO NOT MODIFY THIS TEXT!!! ***/

UBaseType_t uxHighWaterMarkUser;
//char profiling_buf_pre[512];
void user_check_sys_task(void * pvParameters)
{
	char profiling_buf_timeslice[512];

	while(1)
	{
//		PRINTF("------------------------------------\r\n");
//#ifndef BRIDGE
//		uxHighWaterMarkUser = uxTaskGetStackHighWaterMark( pvCreatedTask_Http_client_netconn);
//		PRINTF("Http_client_netconn_task: %d.\r\n", uxHighWaterMarkUser*4);
//		uxHighWaterMarkUser = uxTaskGetStackHighWaterMark( pvCreatedTask_Fota_task);
//		PRINTF("Fota_task: %d.\r\n", uxHighWaterMarkUser*4);
//		uxHighWaterMarkUser = uxTaskGetStackHighWaterMark( pvCreatedTask_Hb_task);
//		PRINTF("Hb_task: %d.\r\n", uxHighWaterMarkUser*4);
//#endif
//#ifdef MQTT
//		uxHighWaterMarkUser = uxTaskGetStackHighWaterMark( pvCreatedTask_mqtt_task);
//		PRINTF("mqtt_task: %d.\r\n", uxHighWaterMarkUser*4);
//#endif
//		uxHighWaterMarkUser = uxTaskGetStackHighWaterMark( pvCreatedTask_Rf_get_message_task);
//		PRINTF("Rf_get_message_task: %d.\r\n", uxHighWaterMarkUser*4);
//		uxHighWaterMarkUser = uxTaskGetStackHighWaterMark( pvCreatedTask_Sensor_status_task);
//		PRINTF("Sensor_status_task: %d.\r\n", uxHighWaterMarkUser*4);
//#ifdef PLC
//		uxHighWaterMarkUser = uxTaskGetStackHighWaterMark( pvCreatedTask_plc_get_message_task);
//		PRINTF("plc_get_message_task: %d.\r\n", uxHighWaterMarkUser*4);
//		uxHighWaterMarkUser = uxTaskGetStackHighWaterMark( pvCreatedTask_plc_status_task);
//		PRINTF("plc_status_task: %d.\r\n", uxHighWaterMarkUser*4);
//#endif
//#ifdef BRIDGE
//		uxHighWaterMarkUser = uxTaskGetStackHighWaterMark( pvCreatedTask_plc_bridge_task);
//		PRINTF("plc_bridge_task: %d.\r\n", uxHighWaterMarkUser*4);
//		uxHighWaterMarkUser = uxTaskGetStackHighWaterMark( pvCreatedTask_plc_fota_task);
//		PRINTF("plc_bridge_task: %d.\r\n", uxHighWaterMarkUser*4);
//#endif
//		uxHighWaterMarkUser = uxTaskGetStackHighWaterMark( pvCreatedTask_Uart_task);
//		PRINTF("Uart_task: %d.\r\n", uxHighWaterMarkUser*4);
//		uxHighWaterMarkUser = uxTaskGetStackHighWaterMark( pvCreatedTask_log_task);
//		PRINTF("Log_task: %d.\r\n", uxHighWaterMarkUser*4);
//#ifdef WIFI_MODULE
//		uxHighWaterMarkUser = uxTaskGetStackHighWaterMark( pvCreatedTask_wifi_task);
//		PRINTF("Wifi_task: %d.\r\n", uxHighWaterMarkUser*4);
//#endif
//#ifdef CPU_MK24FN256VDC12
//#ifndef BRIDGE
//		uxHighWaterMarkUser = uxTaskGetStackHighWaterMark( pvCreatedTask_Ppp_Uart_task);
//		PRINTF("Ppp_uart_task: %d.\r\n", uxHighWaterMarkUser*4);
//		uxHighWaterMarkUser = uxTaskGetStackHighWaterMark( pvCreatedTask_Gsm_irq_handle_task);
//		PRINTF("Gsm_irq_handle_task: %d.\r\n", uxHighWaterMarkUser*4);
//#endif
//		uxHighWaterMarkUser = uxTaskGetStackHighWaterMark( pvCreatedTask_KickWTC);
//		PRINTF("KickWTC: %d.\r\n", uxHighWaterMarkUser*4);
//#endif


	    vTaskGetRunTimeStats1(profiling_buf_timeslice);
	    if (strlen(profiling_buf_timeslice) > 5)
	    {
			PRINTF("Run-time statistics\r\n");
			PRINTF("Task		Abs Time	%%Time\r\n");
			PRINTF("%s\r\n", profiling_buf_timeslice);


			PRINTF("------------------------------------\r\n");
			vTaskList(profiling_buf_timeslice);
			PRINTF("Task List\r\n");
			PRINTF("Task		State	Pri 	WtrMark	Number\r\n");
		    PRINTF("%s\r\n", profiling_buf_timeslice);
	    }

	    vTaskDelay(5000);
	}
}

void vApplicationStackOverflowHook( TaskHandle_t xTask, char *pcTaskName )
{
	PRINTF("\r\n!!!%s StackOverflow!!!\r\n", pcTaskName);
	#ifndef WIFI_MODULE
	PPP_GREEN_LED_OFF();
	#else
	WIFI_GREEN_LED_OFF();
	#endif
	while(1) {
		ms_delay(200);
	#ifndef WIFI_MODULE
		PPP_RED_LED_TOGGLE();
		#else
		WIFI_RED_LED_TOGGLE();
	#endif
	}
}

