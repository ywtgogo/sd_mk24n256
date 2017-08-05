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
#include "A7139.h"

#include "rtc_fun.h"
#include "pin_mux.h"
#include "clock_config.h"
#include "simple_console.h"
#include "uplink_protocol.h"
#include "message.h"
#include "hb_protocol.h"
#include "lwip/netif.h"
#include "log_task.h"
#include "rtc_fun.h"
#include "uart_ppp.h"
#include "pppif.h"
#include <string.h>
#include <stdlib.h>
#include "version.h"
#include "mqtt_task.h"
#ifdef PLC
#include "uart_plc.h"
#include "plc_if.h"
#endif
#include "http_client.h"
#include "hb_protocol.h"

#include "fsl_rtc.h"
#include "wifi_module.h"
#ifndef WIFI_MODULE
#include "audio_play.h"
#endif

const char *to_send = "# ";
//const char *send_ring_overrun = "\r\nRing buffer overrun!\r\n";
//const char *send_hardware_overrun = "\r\nHardware buffer overrun!\r\n";
extern const char *send_ring_overrun;
extern const char *send_hardware_overrun;
extern uart_rtos_handle_t ppp_handle;
extern u32_t sim_status;

#ifdef MQTT
extern QueueHandle_t mqtt_send_q_handle;
#endif
char recv_buffer[CONFIG_SYS_CBSIZE + 1];
static char lastcommand[CONFIG_SYS_CBSIZE + 1] = { 0, };

uart_rtos_handle_t console_handle;
struct _uart_handle t_console_handle;

uint8_t console_background_buffer[128];


struct rtos_uart_config console_config = {
    .baudrate = BOARD_DEBUG_UART_BAUDRATE,
    .parity = kUART_ParityDisabled,
    .stopbits = kUART_OneStopBit,
    .buffer = console_background_buffer,
    .buffer_size = sizeof(console_background_buffer),
};

static int do_test_rf(struct test_tbl_s *cmdtp, int flag, int argc,
                       char * const argv[])
{

    	RF_MSG rf_msg_test;
    	uint32_t sid;
    	err_t err;
    	LONG_ID96 lid;

        PRINTF("%s\r\n", __func__);

    	memset(&rf_msg_test, 0, sizeof(rf_msg_test));

    	lid.idh = HBB_LONG_IDH;
    	lid.idm = HBB_LONG_IDM;
    	lid.id = 0x30020079;//0x40040157;//0x30020079;
    	install_sensor(&lid, sid_to_devicetype(lid.id));

    	rf_msg_test.cmd = RF_CMD_GENERIC_REQUEST;

    	for (int i = 0; i < 5; i++)
    	{
			PRINTF("rf send test packet %d\r\n", i);

    		err = hb_do_rf_send_callback(lid.id, &rf_msg_test);

			if (err != 0)
			{
	    		PRINTF("rf test timeout %d\r\n", i);
				cmdtp->testresult = 2;
			}
    	}

    	if (cmdtp->testresult == 2)
    		PRINTF("rf test fail\r\n");
    	else
    	{
    		cmdtp->testresult = 1;
    		PRINTF("rf test pass %d\r\n");
    	}
        return 0;
}

static int do_show_rf(struct test_tbl_s *cmdtp, int flag, int argc,
                       char * const argv[])
{
		uint32_t ID;
        PRINTF("%s\r\n", __func__);

        ID = A7139_GetCID();
        PRINTF("\r\nA7139_ID: 0x%x \r\n", kxx_htonl(ID));
        return 0;
}

static int do_test_wifi(struct test_tbl_s *cmdtp, int flag, int argc,
                       char * const argv[])
{
     //   PRINTF("%s\r\n", __func__);

        return 0;
}

static int do_show_wifi(struct test_tbl_s *cmdtp, int flag, int argc,
                       char * const argv[])
{
        PRINTF("%s\r\n", __func__);
#ifdef WIFI_MODULE

        wifi_status_msg_t  wifi_status;
    	uint8_t *temp_ip;

    	get_wifi_staus(UART_CMD_GET_WIFI_STATUS,&wifi_status,2000);

        PRINTF("-------------WIFI TEST------------------\r\n\r\n");


		if(wifi_status.con_status == 0x05)
		{
			PRINTF("\r\nWIFI TEST RESULT: --PASS--\r\n\r\n");
			cmdtp->testresult = 1;

			PRINTF("WIFI_Con_State=0X%x\r\n",wifi_status.con_status);
			PRINTF("WIFI_MODE=0X%x\r\n",wifi_status.wifi_mode);

			temp_ip = (uint8_t*)&wifi_status.ip_info.ip;
			PRINTF("WIFI ip:%d.%d.%d.%d \r\n",temp_ip[0],temp_ip[1],temp_ip[2],temp_ip[3]);


			temp_ip = (uint8_t*)&wifi_status.ip_info.gw;
			PRINTF("WIFI GateWay:%d.%d.%d.%d \r\n",temp_ip[0],temp_ip[1],temp_ip[2],temp_ip[3]);

			temp_ip = (uint8_t*)&wifi_status.ip_info.nw;
			PRINTF("WIFI NetMask:%d.%d.%d.%d \r\n",temp_ip[0],temp_ip[1],temp_ip[2],temp_ip[3]);

			PRINTF("WIFI MAC:%x-%x-%x-%x-%x-%x\r\n",wifi_status.mac_addr[0],wifi_status.mac_addr[1],wifi_status.mac_addr[2],wifi_status.mac_addr[3],wifi_status.mac_addr[4],wifi_status.mac_addr[5]);

		}
		else
		{
			PRINTF("WIFI TEST RESULT: --FAILED--\r\n");
			cmdtp->testresult = 2;

		}
		PRINTF("----------------------------------------\r\n\r\n");
#endif
        return 0;
}

static int do_setup_wifi(struct test_tbl_s *cmdtp, int flag, int argc,
                       char * const argv[])
{
       // PRINTF("%s\r\n", __func__);

        return 0;
}

static int do_test_rtc(struct test_tbl_s *cmdtp, int flag, int argc,
                       char * const argv[])
{
		rtc_datetime_t date, date1;
		int count = 0;
		uint64_t time;

        PRINTF("%s\r\n", __func__);

        RTC_GetDatetime(RTC, &date);

        while(1)
        {
            RTC_GetDatetime(RTC, &date1);
            if ((date1.second - date.second) >= 1) {
            	date.second = date1.second;
            	PRINTF("rtc count %d\r\n", count);
            	count++;
            }
            if (date1.second < date.second)
            	date.second = date1.second;
            if (count > 10)
            	break;
        }

        return 0;
}

static int do_test_spinor(struct test_tbl_s *cmdtp, int flag, int argc,
                       char * const argv[])
{
		uint8_t test_pattern[128];
		uint8_t test_buf[128];
		int i;

        PRINTF("%s\r\n", __func__);

    	norflash_block_erase_ll(&flash_master_rtos_handle, SYS_LOG_ADDR_START);

    	for(i = 0; i < 128; i++)
    		test_pattern[i] = i;

        PRINTF("%s write test pattern\r\n", __func__);

    	for (i = 0; i < 128; i++)
    	{
    		if (i % 32 == 0)
    			printf("\r\n");
    		printf("%02x ", test_pattern[i]);
    	}

		printf("\r\n");

		while(true != norflash_write_data_ll(&flash_master_rtos_handle, SYS_LOG_ADDR_START, 128, test_pattern)) {
			vTaskDelay(50);
		}

		norflash_read_data_ll (&flash_master_rtos_handle, SYS_LOG_ADDR_START, 128, test_buf);

        PRINTF("%s read back data\r\n", __func__);

    	for (i = 0; i < 128; i++)
    	{
    		if (i % 32 == 0)
    			printf("\r\n");
    		printf("%02x ", test_buf[i]);
    	}
		printf("\r\n");

        PRINTF("%s verify data\r\n", __func__);

    	for (i = 0; i < 128; i++)
    	{
    		if (test_buf[i] != test_pattern[i])
    		{
        		printf("test failed\r\n");
        		cmdtp->testresult = 2;
        		return 0;
    		}
    	}

		printf("test pass\r\n");

		cmdtp->testresult = 1;

        return 0;
}

static int audio_play_plan(struct test_tbl_s *cmdtp, int flag, int argc,
                       char * const argv[])
{
#ifdef CPU_MK24FN256VDC12
#ifndef WIFI_MODULE
	start_play_plan(1);
#endif
#endif
	return 0;
}

static int do_test_sim(struct test_tbl_s *cmdtp, int flag, int argc,
                       char * const argv[])
{
#ifdef CPU_MK24FN256VDC12

	u32_t ret = AT_RET_OK;

	//ret = modem_at_cmd_ccid(&ppp_handle, AT_REPEAT_MAX, AT_TIMEOUT_MAX, WAIT_RETURN);
	if(sim_status == 0)
	{
		PRINTF("..sim_card bad!!!..\r\n");		
	}
	else
	{
		PRINTF("..sim_card ok..\r\n");	
	}
#endif
}

static int do_test_gprs(struct test_tbl_s *cmdtp, int flag, int argc,
                       char * const argv[])
{
#ifdef CPU_MK24FN256VDC12

    u32_t ret = AT_RET_OK;

	if (get_lwip_link_state() == PPP_ESTABLISHED)
	{
		PRINTF("..gprs ok..\r\n");	
	}
	else
	{
		PRINTF("..gprs bad!!!..\r\n");	
	}
#endif
}

static int do_test_bat(struct test_tbl_s *cmdtp, int flag, int argc,
                       char * const argv[])
{
#ifdef CPU_MK24FN256VDC12

	u32_t Adc16_Value = 0;
	u32_t bat_volt_percent = 0;
	u32_t nPG_Value = 0;
	u32_t nCHG_Value = 0;

	Adc16_Value = BAT_adc_read();
	if(Adc16_Value < BAT_ADC_NOR_VALUE)
	{
		bat_volt_percent = (Adc16_Value*100)/BAT_ADC_NOR_VALUE;
		PRINTF("..bat_volt..=%d\r\n",bat_volt_percent);		
	}
	else
	{
		PRINTF("..bat_volt..=3.6V\r\n");		
	}
	nPG_Value = BAT_nPG_READ();
	if(nPG_Value == 0x0)
	{	
		PRINTF("..this DC..\r\n");		
	}
	else
	{		
		PRINTF("..this AC..\r\n");		
	}
	
	nCHG_Value = BAT_nCHG_READ();
	if(nCHG_Value == 0x0)
	{	
		PRINTF("..charging..\r\n");		
	}
	else
	{		
		PRINTF("..not_charging..\r\n");		
	}
#endif
}

static int do_test_led(struct test_tbl_s *cmdtp, int flag, int argc,
                       char * const argv[])
{
#ifdef CPU_MK24FN256VDC12
	#ifndef WIFI_MODULE
	PPP_GREEN_LED_ON();
	PPP_RED_LED_ON();
	vTaskDelay(400);

	PPP_RED_LED_OFF();
	PPP_GREEN_LED_OFF();
	vTaskDelay(400);
	
	PPP_GREEN_LED_ON();
	PPP_RED_LED_ON();
	vTaskDelay(400);
	
	PPP_RED_LED_OFF();
	PPP_GREEN_LED_OFF();
	vTaskDelay(400);	
	#endif
#endif
}

struct test_tbl_s testlist[] = {
		{"rf_test", 1, 0, do_test_rf, do_show_rf, NULL},
		{"wifi_test", 1, 0, do_test_wifi, do_show_wifi, do_setup_wifi},
		{"do_test_rtc", 1, 0, do_test_rtc, NULL, NULL},
		{"do_test_spinor", 1, 0, do_test_spinor, NULL, NULL},
		{"do_test_sim", 1, 0, do_test_sim, NULL, NULL},
		{"audio_play_plan", 1, 0, audio_play_plan, NULL, NULL},
		{"do_test_gprs", 1, 0, do_test_gprs, NULL, NULL},
		{"do_test_bat", 1, 0, do_test_bat, NULL, NULL},
		{"do_test_led", 1, 0, do_test_led, NULL, NULL},
};

struct cmd_tbl_s cmdlist[] = {
		{"printenv", 1, do_printenv, "printenv show environment" },
		{"fota", 1, do_fota, "fota test" },
		{"list", 2, do_get_sensor_list, "get sensor list" },
		{"ls", 2, do_list_sensor, "list sensor" },
		{"id", 2, do_update_hbb_id, "update homebox id" },
		{"install", 2, do_install_sensor, "install sensor <short id>" },
		{"lpinstall", 2, do_install_sensor_lp, "lpinstall sensor <short id>" },
		{"getattr", 3, do_get_attr, "getattr id index" },
		{"setattr", 4, do_set_attr, "setattr id index value" },
		{"rfconfig", 2, do_update_rf_config, "0xdd00,868.650Mhz	866.001MHz//0x9405 865.001 //0X93D1 864.991 //0x93DC 864.993 //0x93EB 864.996" },
		{"ifdown", 1, do_ifdown, "netif down" },
		{"ifup", 1, do_ifup, "netif up" },
		{"updatehb", 2, do_update_hb_heartbeat, "update_hb_heartbeat" },
		{"hb", 1, do_hb_heartbeat, "send homebox heartbeat" },
		{"hsb", 1, do_heartbeat, "send homebox and sensor heartbeat" },
		{"taskstate", 1, do_get_task_state, "get task state" },
		{"hbevent", 4, do_hb_event, "send homebox event/alert/error: hbevent <status> <data> <priority>" },
		{"sevent", 4, do_sensor_event, "send sensor event/alert/error: sevent <sid> <status> <data>" },
		{"dflog", 2, do_hb_defer_log, "send defer log" },
		{"testlog", 2, do_hb_sys_log, "sys log" },
		{"sdata", 2, do_send_data, "send data" },
		{"dumplog", 1, do_hb_dump_log, "dump log" },
		{"uplog", 1, do_hb_up_log, "dump log" },
		{"erase", 2, do_hb_erase_sector, "erase sector: erase <sectornum>" },
		{"eraselog", 2, do_hb_erase_log_sector, "erase log all" },
		{"e2", 2, do_hb_erase_img2_test, "erase img2 4k" },
		{"e3", 2, do_hb_erase_img3_test, "erase img3 4k" },
		{"ein", 2, do_hb_erase_internal_test, "erase img3 4k" },
		{"erasedefer", 2, do_hb_erase_defer_sector, "erase defer area all" },
		{"eraseall", 2, do_hb_erase_spinor, "erase spinor all" },
		{"readspiflash",3,do_read_spi_flash,"read spi flash"},
		{"result", 3, do_send_fw_result, "send fw result" },
		{"reset", 1, do_reset, "reset hbb" },
		{"query", 2, do_query, "query sensorid" },
	//	{"setalarm", 2, setalarm, "alarm sensorid" },
		{"help", 1, do_help, "show help" },
		{"ppp", 1, do_ppp, "ppp check status" },
		{"scan", 1, do_scan, "scan defer area" },
		{"savehbbinfo", 1, do_save_hbb, "savehbbinfo" },
		{"erasehbbinfo", 1, do_erase_hbb, "erasehbbinfo" },
		{"showhbbinfo", 1, do_show_hbb_info, "showhbbinfo" },
		{"erasesensorlist", 1, do_erase_sensor_list, "erasesensorlist" },
		{"ppp_relink", 1, do_ppp_relink, "do_ppp_relink" },
		{"gsmsign", 1, do_gsm_sign, "do_gsm_sign" },
		{"save_apn", 1, do_save_gprs_apn, "do_save_gprs_apn" },
		{"testmode", 1, do_testmode, "enter testmode" },
#ifdef WIFI_MODULE
		{"wifi_rssi", 2, do_get_wifi_rssi, "do_get_wifi_rssi" },
		{"wifi_ver", 2, do_get_wifi_ver, "do_get_wifi_ver" },
		{"wifi_ssid", 2, do_get_wifi_ssid, "do_get_wifi_ssid" },
		{"wifi_status", 2, do_show_wifi, "do_show_wifi_connect_status" },
#endif
#ifdef PLC
		{"sync_bridge_rtc",2,do_sync_bridge_rtc_time,"do_sync_bridge_rtc_time"},
		{"query_bridge",2,do_query_bridge,"do_query_bridge"},
		{"disengage_bridge_sensor",3,do_disengage_bridge_sensor,"do_disengage_bridge_sensor"},
		{"engage_bridge_sensor",3,do_engage_bridge_sensor,"do_engage_bridge_sensor"},
		{"set_low",3,do_set_plc_pir_low_sens,"do_set_plc_pir_low_sens"},
		{"set_high",3,do_set_plc_pir_high_sens,"do_set_plc_pir_high_sens"},

		{"sent_list",2,do_sent_sensor_list_to_bridge,"do_sent_sensor_list_to_bridge"},
		{"deploy_list",2,do_deploy_sensor_list_to_bridge,"do_deploy_sensor_list_to_bridge"},

		{"plc_fw_update",3,do_plc_fw_need_update,"do_plc_bridge_need_update"},
		{"sent_img_block",4,do_plc_request_img_block_ack,"do_plc_request_img_block_ack"},
#endif
};
#ifdef PLC
#ifndef BRIDGE
static int do_sync_bridge_rtc_time(struct cmd_tbl_s *cmdtp, int flag, int argc,
							  char * const argv[])
{
	sync_bridge_rtc_time(strtoul(argv[1], NULL,16),++plc_local_seq);
	return 0;
}

static int do_query_bridge(struct cmd_tbl_s *cmdtp, int flag, int argc,
							  char * const argv[])
{
	query_bridge_sensor(strtoul(argv[1], NULL,16),++plc_local_seq);
	return 0;
}

static int do_disengage_bridge_sensor(struct cmd_tbl_s *cmdtp, int flag, int argc,
							  char * const argv[])
{
	disengage_bridge_sensor(strtoul(argv[1], NULL,16),strtoul(argv[2], NULL,16),++plc_local_seq);
	return 0;
}

static int do_engage_bridge_sensor(struct cmd_tbl_s *cmdtp, int flag, int argc,
							  char * const argv[])
{
	engage_bridge_sensor(strtoul(argv[1], NULL,16),strtoul(argv[2], NULL,16),++plc_local_seq);
	return 0;
}
static int do_set_plc_pir_low_sens(struct cmd_tbl_s *cmdtp, int flag, int argc,
							  char * const argv[])
{
	set_bridge_pir_low_sens(strtoul(argv[1], NULL,16),strtoul(argv[2], NULL,16),++plc_local_seq);
	return 0;
}
static int do_set_plc_pir_high_sens(struct cmd_tbl_s *cmdtp, int flag, int argc,
							  char * const argv[])
{
	set_bridge_pir_high_sens(strtoul(argv[1], NULL,16),strtoul(argv[2], NULL,16),++plc_local_seq);
	return 0;
}

static int do_sent_sensor_list_to_bridge(struct cmd_tbl_s *cmdtp, int flag, int argc,
							  char * const argv[])
{
	sent_sensor_list_to_bridge(strtoul(argv[1], NULL,16),++plc_local_seq);
	return 0;
}
static int do_deploy_sensor_list_to_bridge(struct cmd_tbl_s *cmdtp, int flag, int argc,
							  char * const argv[])
{
	deploy_sensor_list_to_bridge(strtoul(argv[1], NULL,16),++plc_local_seq);
	return 0;
}

static int do_plc_fw_need_update(struct cmd_tbl_s *cmdtp, int flag, int argc,
							  char * const argv[])
{
	plc_fw_need_update(strtoul(argv[1], NULL,16),strtoul(argv[2], NULL,16),++plc_local_seq);
	return 0;
}
static int do_plc_request_img_block_ack(struct cmd_tbl_s *cmdtp, int flag, int argc,
							  char * const argv[])
{
	plc_request_img_block_ack(strtoul(argv[1], NULL,16),strtoul(argv[2], NULL,16),strtoul(argv[3], NULL,10),++plc_local_seq);
	return 0;
}
#endif
#endif

static int do_gsm_sign(struct cmd_tbl_s *cmdtp, int flag, int argc,
							  char * const argv[])
{
#ifdef CPU_MK24FN256VDC12

	gprs_set_mode_to_at();
	gprs_get_gprs_sign();
#endif
	return 0;
}

static int do_ppp_relink(struct cmd_tbl_s *cmdtp, int flag, int argc,
							  char * const argv[])
{
#ifdef CPU_MK24FN256VDC12
	gprs_set_mode_to_at();
	gprs_set_gprs_down();
#endif
	return 0;
}

static int do_ppp(struct cmd_tbl_s *cmdtp, int flag, int argc,
							  char * const argv[])
{
#ifdef CPU_MK24FN256VDC12
	ppp_check_status();
#endif
	return 0;
}

static int do_scan(struct cmd_tbl_s *cmdtp, int flag, int argc,
							  char * const argv[])
{
	SECTOR_INFO_STRUCT info;

	memset(&info, 0, sizeof(info));
	scan_defer_area(&info);

	PRINTF("info w %d %d, r %d %d\r\n", info.offset_sector_num_w, info.info_num_w,
			 info.offset_sector_num_r, info.info_num_r);

	scan_log_area(&sys_log_info);

	return 0;
}

static int do_save_hbb(struct cmd_tbl_s *cmdtp, int flag, int argc,
							  char * const argv[])
{
	hbb_config *config;

	sync_hbb_info();

	config = (volatile HBB_STATUS_STORAGE_BLOCK *)HBB_INFO_CONFIG;

	PRINTF("id  %x\r\n", config->id);
	PRINTF("dt  %x\r\n", ver_info.dt);
	PRINTF("heartbeat_interval  %d\r\n", config->heartbeat_interval);
	PRINTF("fota_check_interval  %d\r\n", config->fota_check_interval);
	PRINTF("eventurl  %s\r\n", config->eventurl);

	return 0;
}

static int do_erase_hbb(struct cmd_tbl_s *cmdtp, int flag, int argc,
							  char * const argv[])
{

	taskENTER_CRITICAL();
	FLASH_RTOS_Erase_Sector(&pflash_handle, HBB_INFO_CONFIG, 1);
	taskEXIT_CRITICAL();

	return 0;
}

static int do_erase_sensor_list(struct cmd_tbl_s *cmdtp, int flag, int argc,
							  char * const argv[])
{

	taskENTER_CRITICAL();
	FLASH_RTOS_Erase_Sector(&pflash_handle, HBB_SENSOR_LIST_CONFIG, 1);
	taskEXIT_CRITICAL();

	return 0;
}

static int do_show_hbb_info(struct cmd_tbl_s *cmdtp, int flag, int argc,
							  char * const argv[])
{
	char temp[16];
	PRINTF("\r\n/--------------VERSION INFO-----------------\r\n");
	PRINTF("svn version: @%s@\r\n", ver_info.sw_info);
	PRINTF("\r\n/--------------HOMEBOX INFO-----------------\r\n");
	printf("DID: @%08X@\r\n", hbb_info.config.id);
	printf("DT: @%08X@\r\n", ver_info.dt);
	printf("Longid: @%08X%08X%08X@\r\n", hbb_info.config.longidh, hbb_info.config.longidm, hbb_info.config.id);
	get_version_string(ver_info.swver, temp, sizeof(temp));
	PRINTF("Firmware version: @%s@\r\n", temp);
	get_version_string(ver_info.hwver, temp, sizeof(temp));
	PRINTF("Hardware version: @%s@\r\n", temp);
	PRINTF("Alart url: @%s@\r\n", hbb_info.config.alerturl);
	PRINTF("Event url: @%s@\r\n", hbb_info.config.eventurl);
	PRINTF("Fota url: @%s@\r\n", hbb_info.config.fotaurl);
	PRINTF("Control url: @%s@\r\n", hbb_info.config.controlurl);
	PRINTF("Mqtt address: @%s@\r\n", hbb_info.config.mqtt_address);
	PRINTF("Mqtt port: @%d@\r\n", hbb_info.config.mqtt_port);
	PRINTF("Heartbeat interval: @%d minutes@\r\n", hbb_info.config.heartbeat_interval/60000);
	PRINTF("Fota check interval: @%d hours@\r\n", hbb_info.config.fota_check_interval/3600000);
	PRINTF("Aes key: @%s@\r\n", hbb_info.config.aeskey);
	PRINTF("Aes ivv: @%s@\r\n", hbb_info.config.aesivv);
	PRINTF("Aes key version: @%s@\r\n", hbb_info.config.key_version);
#ifdef CPU_MK24FN256VDC12
	PRINTF("Sos number: @%s@\r\n", ppp_gprs_apn_pap_info.sos_call);
	PRINTF("Auto answer: @%d@\r\n", ppp_gprs_apn_pap_info.incoming_call_answer_timeout);	
	PRINTF("gprs_apn: @%s@\r\n", ppp_gprs_apn_pap_info.gprs_apn);
#endif
	return 0;
}

static int do_save_gprs_apn(struct cmd_tbl_s *cmdtp, int flag, int argc,
							  char * const argv[])
{
	ppp_gprs_apn_pap_info_conf *config;

	sync_gprs_info();

	config = (volatile ppp_gprs_apn_pap_info_conf *)0x3e800;

	PRINTF("gprs_apn  %s\r\n", config->gprs_apn);

	return 0;
}

#ifdef WIFI_MODULE
static int do_get_wifi_rssi(struct cmd_tbl_s *cmdtp, int flag, int argc,
							  char * const argv[])
{
	s32_t rssi_level;
	rssi_level = wifi_get_rssi(3000);
	printf("wifi rssi: %d\r\n", rssi_level);
	return 0;
}

static int do_get_wifi_ver(struct cmd_tbl_s *cmdtp, int flag, int argc,
							  char * const argv[])
{
	wifi_ver_info ver_info;
	wifi_get_ver_info(&ver_info,3000);
	PRINTF("wifi version fw:%x--info:%s\r\n",ver_info.swver,ver_info.sw_info);
	return 0;
}
static int do_get_wifi_ssid(struct cmd_tbl_s *cmdtp, int flag, int argc,
							  char * const argv[])
{
	struct station_config config;
	wifi_get_config_info(&config,3000);
	PRINTF("wifi ssid:%s\r\n", config.ssid);
	return 0;
}

#endif





static int do_send_fw_result(struct cmd_tbl_s *cmdtp, int flag, int argc,
							  char * const argv[])
{
	UPLINK_Q_MSG uplink_get_dev_msg;
	PRINTF("argv1 is rid, argv2 is rc.\r\n");
	PRINTF("Send fw result.....\r\n");

	uplink_get_dev_msg.cmd = UPLINK_FW_UPD_RESULT;
	uplink_get_dev_msg.fw_upd_result.rid = strtoul(argv[1], NULL,10);
	uplink_get_dev_msg.fw_upd_result.rc = strtoul(argv[2], NULL,10);
	if(uplink_get_dev_msg.fw_upd_result.rc == PART_OF_DEV_FAIL){
		PRINTF(" unsupported the RC of PART_OF_DEV_FAIL!\r\n");
		return 0;
	}

	xQueueSend(uplink_q_handle, &uplink_get_dev_msg, portMAX_DELAY);
	return 0;
}
static int do_get_sensor_list(struct cmd_tbl_s *cmdtp, int flag, int argc,
							  char * const argv[])
{
	UPLINK_Q_MSG uplink_get_dev_msg;
	PRINTF("Get sensor list........\r\n");
	uplink_get_dev_msg.cmd = UPLINK_GET_SUB_DEVICES;
	uplink_get_dev_msg.sub_dev_req.pos = strtoul(argv[1], NULL,10);
	if(uplink_get_dev_msg.sub_dev_req.pos <= 0 || uplink_get_dev_msg.sub_dev_req.pos >= 256){
		PRINTF("The argument of list is invalid!\r\n");
		return 0;
	}
	xQueueSend(uplink_q_handle, &uplink_get_dev_msg, portMAX_DELAY);
	return 0;
}

static int do_list_sensor(struct cmd_tbl_s *cmdtp, int flag, int argc,
							  char * const argv[])
{
	printf("homebox id: %X##\r\n", hbb_info.config.id);
	PRINTF("sensor list+++++++++++++++++++++\r\n");
	for(int i=0; i<MAX_SENSOR_COUNT; i++)
		if (sensor_status[i].send_message.sn_dest != 0)
		{
			printf("sensor id %d: %X -> %X dt %08x rssi %d alarm %d uplink %d timer %d %d##\r\n", i,
					sensor_status[i].send_message.sn_dest,
					sensor_status[i].send_message.sn_mitt,
					sensor_status[i].device_type,
					sensor_status[i].send_message.rssi,
					sensor_status[i].send_message.alarm_status,
					sensor_status[i].last_uplink_status,
					xTimerIsTimerActive(sensor_status[i].alarm_timers),
					sensor_status[i].x_tick);
			print_attr(&sensor_status[i]);
			print_todo_work(&sensor_status[i]);
		}

	return 0;
}

extern struct netif fsl_netif0;
static int do_ifdown(struct cmd_tbl_s *cmdtp, int flag, int argc,
							  char * const argv[])
{
	PRINTF("%s\r\n", __func__);
    //netif_set_down(&fsl_netif0);

	return 0;
}

static int do_ifup(struct cmd_tbl_s *cmdtp, int flag, int argc,
							  char * const argv[])
{
	PRINTF("%s\r\n", __func__);
    //netif_set_up(&fsl_netif0);

	return 0;
}

static int do_update_hb_heartbeat(struct cmd_tbl_s *cmdtp, int flag, int argc,
							  char * const argv[])
{
	update_hb_heartbeat(strtoul(argv[1], NULL,10));
	return 0;
}

static int do_update_hbb_id(struct cmd_tbl_s *cmdtp, int flag, int argc,
							  char * const argv[])
{
	LONG_ID96 lid = {0};
	char temp_str[9];
	if(strlen(argv[1]) != 24)
	{
		printf("the id length is error\r\n");
		return 0;
	}
	strncpy(temp_str, argv[1], 8);
	lid.idh = strtoul(temp_str, NULL,16);
	strncpy(temp_str, argv[1]+8, 8);
	lid.idm = strtoul(temp_str, NULL,16);
	strncpy(temp_str, argv[1]+8*2, 8);
	lid.id = strtoul(temp_str, NULL,16);
	update_hbb_id(&lid);
	NVIC_SystemReset();
	return 0;
}

static int do_install_sensor(struct cmd_tbl_s *cmdtp, int flag, int argc,
							  char * const argv[])
{
	LONG_ID96 lid = {0};
	if(strlen(argv[1]) != 8)
	{
		printf("the id length is error\r\n");
		return 0;
	}
	lid.idh = HBB_LONG_IDH;
	lid.idm = HBB_LONG_IDM;
	lid.id = strtoul(argv[1], NULL,16);
	install_sensor(&lid, sid_to_devicetype(lid.id));
	return 0;
}

int install_sensor_test(uint32_t sid, uint32_t dt)
{
	LONG_ID96 lid = {0};

	lid.idh = HBB_LONG_IDH;
	lid.idm = HBB_LONG_IDM;
	lid.id = sid;
	install_sensor(&lid, dt);
	return 0;
}

int do_install_sensor_lp(struct cmd_tbl_s *cmdtp, int flag, int argc,
							  char * const argv[])
{
	LONG_ID96 lid = {0};
	uint32_t dt;
	if(strlen(argv[1]) != 8)
	{
		printf("the id length is error\r\n");
		return 0;
	}
	lid.idh = HBB_LONG_IDH;
	lid.idm = HBB_LONG_IDM;
	lid.id = strtoul(argv[1], NULL,16);
	dt = sid_to_devicetype(lid.id);
	install_sensor(&lid, dt | 0x3);
	return 0;
}

static int do_get_attr(struct cmd_tbl_s *cmdtp, int flag, int argc,
							  char * const argv[])
{
	LONG_ID96 lid = {0};
	int index = 0;

	if(strlen(argv[1]) != 8)
	{
		printf("the id length is error\r\n");
		return 0;
	}

	lid.idh = HBB_LONG_IDH;
	lid.idm = HBB_LONG_IDM;
	lid.id = strtoul(argv[1], NULL,16);
	index = strtoul(argv[2], NULL,10);

	user_get_sensor_attribute(lid.id, index);

	return 0;
}

static int do_set_attr(struct cmd_tbl_s *cmdtp, int flag, int argc,
							  char * const argv[])
{
	LONG_ID96 lid = {0};
	int index = 0;
	uint32_t value = -1;
	struct sensor_attribute arg;

	arg.index = 17;
	arg.value = 30000;

	if(strlen(argv[1]) != 8)
	{
		printf("the id length is error\r\n");
		return 0;
	}

	lid.idh = HBB_LONG_IDH;
	lid.idm = HBB_LONG_IDM;
	lid.id = strtoul(argv[1], NULL,16);
	index = strtoul(argv[2], NULL,10);
	value = strtoul(argv[3], NULL,10);

	user_set_sensor_attribute(lid.id, index, value);

	return 0;
}

static int do_hb_erase_img2_test(struct cmd_tbl_s *cmdtp, int flag, int argc,
                       char * const argv[])
{

	norflash_sector_erase_ll(&flash_master_rtos_handle, FOTA_HBB_IMG1_ADDR_START + 0x2000);

	return 0;
}

static int do_hb_erase_img3_test(struct cmd_tbl_s *cmdtp, int flag, int argc,
                       char * const argv[])
{

	norflash_sector_erase_ll(&flash_master_rtos_handle, FOTA_HBB_IMG2_ADDR_START + 0x2000);

	return 0;
}

static int do_hb_erase_internal_test(struct cmd_tbl_s *cmdtp, int flag, int argc,
                       char * const argv[])
{

	taskENTER_CRITICAL();
	FLASH_RTOS_Erase_Sector(&pflash_handle, 0x3F000, 1);
	taskEXIT_CRITICAL();

	return 0;
}

static int do_update_rf_config(struct cmd_tbl_s *cmdtp, int flag, int argc,
							  char * const argv[])
{
	uint16_t freq;
	uint32_t ID;

	freq = strtoul(argv[1], NULL,16);

	A7139_StrobeCmd(0x55);
	A7139_Init(freq);
	A7139_SetCID(0x78877887);
	ID = A7139_GetCID();
	PRINTF("\r\nA7139_ID: 0x%x \r\n", kxx_htonl(ID));
	A7139_StrobeCmd(CMD_STBY);
	vTaskDelay(10);
	A7139_SetGIO2_Func(e_GIOS_FSYNC);
	A7139_StrobeCmd(CMD_RX);

	if( xSemaphoreTake( hbb_info.xSemaphore, 1000) == pdTRUE)
	{
		hbb_info.config.freq = freq;
		xSemaphoreGive(hbb_info.xSemaphore);
		sync_hbb_info();
	} else {
		PRINTF("save hbb config failed\r\n");
	}

	return 0;
}

static int do_reset(struct cmd_tbl_s *cmdtp, int flag, int argc,
							  char * const argv[])
{
	NVIC_SystemReset();

	return 0;
}

static int do_printenv(struct cmd_tbl_s *cmdtp, int flag, int argc,
                       char * const argv[])
{
        PRINTF("print Environment to...\r\n");

        return 0;
}

static int do_help(struct cmd_tbl_s *cmdtp, int flag, int argc,
                       char * const argv[])
{
		struct cmd_tbl_s *cmdp;

		PRINTF("help\r\n");

		for (cmdp = cmdlist; cmdp != cmdlist + sizeof(cmdlist)/sizeof(cmdlist[0]); cmdp++) {
				PRINTF("%s - %s\r\n", cmdp->name, cmdp->usage);
		}

		return 0;
}

static int do_hb_heartbeat(struct cmd_tbl_s *cmdtp, int flag, int argc,
                       char * const argv[])
{
	hbb_send_uplink_event(hbb_info.config.id, INFO);

	return 0;
}

static int do_heartbeat(struct cmd_tbl_s *cmdtp, int flag, int argc,
                       char * const argv[])
{
	xSemaphoreGive(hb_task_sem);

	return 0;
}

static int do_get_task_state(struct cmd_tbl_s *cmdtp, int flag, int argc,
                       char * const argv[])
{
	char profiling_buf_timeslice[512];

	vTaskGetRunTimeStats(profiling_buf_timeslice);
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

	return 0;
}

static int do_hb_event(struct cmd_tbl_s *cmdtp, int flag, int argc,
                       char * const argv[])
{
	uint32_t status = 20;
	uint32_t data = 0;
	int p = 6;

	status = strtoul(argv[1], NULL,10);
	data = strtoul(argv[2], NULL,10);
	p = strtoul(argv[3], NULL,10);

	hbb_send_uplink_data(hbb_info.config.id, status,  p, "%d", data);

	return 0;
}

static int do_sensor_event(struct cmd_tbl_s *cmdtp, int flag, int argc,
                       char * const argv[])
{
	uint32_t sid = 0;
	uint32_t status = 10;
	char *data;

	sid = strtoul(argv[1], NULL,16);
	status = strtoul(argv[2], NULL,10);
	data = argv[3];

	sensor_send_uplink_data(sid, status,  1, "%s", data);

	return 0;
}


static int do_hb_defer_log(struct cmd_tbl_s *cmdtp, int flag, int argc,
                       char * const argv[])
{
	UPLINK_Q_MSG uplink_msg;
	int msgcnt = strtoul(argv[1], NULL,10);

	memset(&uplink_msg, 0, sizeof(uplink_msg));
	uplink_msg.event.slid.id = hbb_info.config.id;
	uplink_msg.cmd = UPLINK_SEND_EVENT;
	uplink_msg.event.device_type = ver_info.dt;
	uplink_msg.event.status = INFO;
	get_current_systime(&uplink_msg.event.ts);

	if (msgcnt > 0) {
		for (int i = 0; i < msgcnt; i++)
			save_defer_log(&uplink_msg);
	} else
		defer_log_uplink();

	return 0;
}

static int do_hb_sys_log(struct cmd_tbl_s *cmdtp, int flag, int argc,
                       char * const argv[])
{
	int msgcnt = strtoul(argv[1], NULL,10);

	if (msgcnt > 0)
	{
		print_log("http\1" ERROR_CODE HBLOG_ERROR "http url parse err.\r\n", HTTP_PARSE);
		print_log("http\1" ERROR_CODE HBLOG_ERROR "Not connected.\r\n", HTTP_CONN);
		print_log("http\1" ERROR_CODE HBLOG_ERROR "Connection closed.\r\n", HTTP_CLSD);
		print_log("http\1" ERROR_CODE HBLOG_ERROR "Connection reset.\r\n", HTTP_RST);
		print_log("http\1" ERROR_CODE HBLOG_ERROR "Connection aborted.\r\n", HTTP_ABRT);
		print_log("http\1" ERROR_CODE HBLOG_ERROR "Timeout.\r\n", HTTP_TIMEOUT);
		print_log("http\1" ERROR_CODE HBLOG_ERROR "error 404 Not found !\r\n", ERR_404);
		print_log("http\1" ERROR_CODE HBLOG_ERROR "error 403 refuse denied!\r\n", ERR_403);
		print_log("http\1" NOERROR_CODE HBLOG_WARNING "The fota client request data invalid.\r\n" );
		print_log("http\1" NOERROR_CODE HBLOG_WARNING "The upgraded device can not found in database!\r\n");
		print_log("http\1" NOERROR_CODE HBLOG_WARNING "Signature key can't get from database!\r\n");
		print_log("http\1" NOERROR_CODE HBLOG_WARNING "The signature key doesn't match!\r\n");
		print_log("http\1" NOERROR_CODE HBLOG_WARNING "The img need to download does not exist!\r\n");

//
//		print_log("m:\"spi\"," LOG_INFO "d:" "\"NorFlash Read ID Fialed\r\n");
//		print_log("m:\"ppp\"," LOG_INFO "d:" "\"..km init..\r\n");
//
//		print_log("m:\"ppp\"," LOG_INFO "d:" "\"PPP is dead. try to negotitate!\r\n");
//		print_log("m:\"ppp\"," LOG_INFO "d:" "\"PPP is negotitating! wait\r\n");
//		print_log("m:\"ppp\"," LOG_INFO "d:" "\"PPP error!\r\n");
//		print_log("m:\"ppp\"," LOG_INFO "d:" "\"..ppp_status_dead..\r\n");
//		print_log("m:\"ppp\"," LOG_INFO "d:" "\"..ppp_uart_status_dead..\r\n");
//		print_log("m:\"ppp\"," LOG_INFO "d:" "\"..update_apn..\r\n");
//		print_log("m:\"ppp\"," LOG_INFO "d:" "\"..update_sos..\r\n");
//		print_log("m:\"ppp\"," LOG_INFO "d:" "\"..ppp_nego_up..\r\n");
//		print_log("m:\"ppp\"," LOG_INFO "d:" "\"..ppp_nego_down..\r\n");
//		print_log("m:\"ppp\"," LOG_INFO "d:" "\"..ppp_down..\r\n");
//		print_log("m:\"ppp\"," LOG_INFO "d:" "\"..ppp_down_dcd..\r\n");
//		print_log("m:\"ppp\"," LOG_INFO "d:" "\"..ppp_status_idle..\r\n");
//		print_log("m:\"ppp\"," LOG_INFO "d:" "\".incoming_call.\r\n");
//		print_log("m:\"ppp\"," LOG_INFO "d:" "\"..update_sos..\r\n");
//		print_log("m:\"ppp\"," LOG_INFO "d:" "\"..sos_call_ing..\r\n");


	}

	return 0;
}

static int do_send_data(struct cmd_tbl_s *cmdtp, int flag, int argc,
                       char * const argv[])
{
#ifdef MQTT
	MQTT_SEND_Q_MSG send_data;
	send_data.send_cmd = MQTT_SEND_DATA;
	//send_data.send_data.data_info = (void *)&get_data_info;
	xQueueSend(mqtt_send_q_handle, (MQTT_SEND_Q_MSG *)&send_data, portMAX_DELAY);
#endif
	return 0;
}

static int do_hb_erase_sector(struct cmd_tbl_s *cmdtp, int flag, int argc,
                       char * const argv[])
{
	int sector = strtoul(argv[1], NULL,10);

	erase_log_sector(sector);

	scan_log_area(&sys_log_info);

	return 0;
}

static int do_hb_erase_log_sector(struct cmd_tbl_s *cmdtp, int flag, int argc,
                       char * const argv[])
{
	int sector;

	for (sector = 0; sector < SYS_LOG_SECTOR_COUNT; sector++)
		erase_log_sector(sector);

	scan_log_area(&sys_log_info);

	return 0;
}

static int do_hb_erase_defer_sector(struct cmd_tbl_s *cmdtp, int flag, int argc,
                       char * const argv[])
{
	int i;

	for (i = 0; i < DEFER_UPLINK_SECTOR_COUNT; i++) {

		erase_defer_sector(i);
	}
	scan_defer_area(&sector_info);

	return 0;
}

static int do_hb_erase_spinor(struct cmd_tbl_s *cmdtp, int flag, int argc,
                       char * const argv[])
{
	norflash_chip_erase_ll(&flash_master_rtos_handle);

	return 0;
}

static int do_read_spi_flash(struct cmd_tbl_s *cmdtp, int flag, int argc,
        char * const argv[])
{
	uint8_t tmp_data[1024];
	uint32_t len = strtoul(argv[2], NULL, 16);
	norflash_read_data_ll (&flash_master_rtos_handle, strtoul(argv[1], NULL, 16), len, tmp_data);
	for(uint32_t i =0;i<len;i++)
		PRINTF("%x ",tmp_data[i]);

}

static int do_hb_dump_log(struct cmd_tbl_s *cmdtp, int flag, int argc,
					   char * const argv[])
{
	dump_log(0, 0);
	return 0;
}

static int do_hb_up_log(struct cmd_tbl_s *cmdtp, int flag, int argc,
					   char * const argv[])
{
	uplink_log();
	return 0;
}

static int do_fota(struct cmd_tbl_s *cmdtp, int flag, int argc,
                       char * const argv[])
{
        PRINTF("fota\r\n");

    	/**/
//    	UPLINK_Q_MSG uplink_fw_req_msg;
//    	memset(&uplink_fw_req_msg, 0, sizeof(uplink_fw_req_msg));
//    	uplink_fw_req_msg.cmd = FW_TIMING_CHECK_FOTA;
//    	uplink_fw_req_msg.fw_req.bid = 0;
//    	 while(uplink_q_handle == NULL)
//    	    	vTaskDelay( 500 / portTICK_RATE_MS );
//    	xQueueSend(uplink_q_handle, &uplink_fw_req_msg, portMAX_DELAY);
		FOTA_RQ_MSG msg;
		msg.cmd = FW_TIMING_CHECK_FOTA;
		xQueueSend(fota_rq_handle, &msg, 2000);
        /**/

        return 0;
}

static int do_query(struct cmd_tbl_s *cmdtp, int flag, int argc,
                       char * const argv[])
{
    	RF_MSG rf_msg_test;
    	uint32_t sid;
    	err_t err;

    	memset(&rf_msg_test, 0, sizeof(rf_msg_test));
    	sid = strtoul(argv[1], NULL, 16);
    	rf_msg_test.cmd = RF_CMD_GENERIC_REQUEST;

    	err = hb_do_rf_send_callback(sid, &rf_msg_test);

    	if (err > 0)
    		return 0;

        return err;
}

static int do_setalarm(struct cmd_tbl_s *cmdtp, int flag, int argc,
                       char * const argv[])
{
    	RF_MSG rf_msg_test;

    	rf_msg_test.header	= 0x14;
    	rf_msg_test.sn_dest = 0xaaaaaaaa;
    	rf_msg_test.sn_mitt = strtoul(argv[1], NULL, 16);;
    	rf_msg_test.fw_major = 1;
    	rf_msg_test.fw_minor = 0;

    	rf_msg_test.cmd = RF_CMD_UNSOLICITED_STATUS;
    	rf_msg_test.sequence = local_seq++;
    	rf_send_then_receive(&rf_msg_test);

        return 0;
}

void showtestmenu(void)
{
		struct test_tbl_s *cmdtp;

        for (cmdtp = testlist; cmdtp != testlist + sizeof(testlist)/sizeof(testlist[0]); cmdtp++) {
                PRINTF("%d- %s\r\n", (cmdtp - testlist), cmdtp->name);
        }

        PRINTF("please select test num, type \"finish\" to end test\r\n");
        return;
}

void resettestresult(void)
{
		struct test_tbl_s *cmdtp;

        for (cmdtp = testlist; cmdtp != testlist + sizeof(testlist)/sizeof(testlist[0]); cmdtp++) {
                cmdtp->testresult = 0;
        }

        return;
}

void showtestresult(void)
{
		struct test_tbl_s *cmdtp;
		char result_str[][32] = {"NULL", "PASS", "FAIL"};

        for (cmdtp = testlist; cmdtp != testlist + sizeof(testlist)/sizeof(testlist[0]); cmdtp++) {
                PRINTF("%d:%s %s\r\n", (cmdtp - testlist), result_str[cmdtp->testresult], cmdtp->name);
        }

        return;
}

void gettestresult(struct test_tbl_s *testtp)
{

try_again:
		if (testtp->testresult == 0)
		{
			int n = 0;

			PRINTF("please input test result [pass/fail]\r\n");
	        n = readline_to_buffer(recv_buffer);

	       if (n > 0)
		   {
				strncpy(lastcommand, recv_buffer, n);
				lastcommand[n] = '\0';
				PRINTF("%s\r\n", lastcommand);
				if (!strncmp(lastcommand, "pass", 4))
					testtp->testresult = 1;
				else if(!strncmp(lastcommand, "fail", 2))
					testtp->testresult = 2;
				else
				{
					PRINTF("invalid result\r\n");
					goto try_again;
				}
		   }
	       else
				goto try_again;

		}

        return;
}

static int do_testmode(struct cmd_tbl_s *cmdtp, int flag, int argc,
							  char * const argv[])
{
	int n;
	PRINTF("%s\r\n", __func__);
	int testlen = sizeof(testlist)/sizeof(testlist[0]);
    char *str;
    int testnum = -1;
	struct test_tbl_s *testtp = testlist;

    resettestresult();

    for (;;)
    {
   	    showtestmenu();

        n = readline_to_buffer(recv_buffer);
        if (n > 0)
        {
            /* send back the received data */
        	 //strlcpy(lastcommand, recv_buffer, CONFIG_SYS_CBSIZE + 1);

             strncpy(lastcommand, recv_buffer, n);
             lastcommand[n] = '\0';
        	 PRINTF("%s\r\n", lastcommand);

        	 str = lastcommand;

        	 if (!strncmp("finish", lastcommand, 6))
        		 break;

        	 testnum = strtoul(str, NULL,16);
        	 if (testnum < 0 || testnum >= testlen)
        	 {
        		 PRINTF("please input a valid number\r\n");
        	   	 showtestmenu();
        	   	 continue;
        	 }

        	 if (testtp[testnum].setup)
        		 testtp[testnum].setup(&testtp[testnum], 0, 0, NULL);

        	 if (testtp[testnum].showstate)
        		 testtp[testnum].showstate(&testtp[testnum], 0, 0, NULL);

        	 if (testtp[testnum].runtest)
        		 testtp[testnum].runtest(&testtp[testnum], 0, 0, NULL);

        	 gettestresult(&testtp[testnum]);
        }
    }

    showtestresult();

	return 0;
}

#define isblank(c)      (c == ' ' || c == '\t')

#define isascii(c) (((unsigned char)(c))<=0x7f)
#define toascii(c) (((unsigned char)(c))&0x7f)

struct cmd_tbl_s *find_cmd_tbl(const char *cmd, struct cmd_tbl_s *table, int table_len)
{
		struct cmd_tbl_s *cmdtp;
		struct cmd_tbl_s *cmdtp_temp = table;  /* Init value */
        int len;
        int n_found = 0;

        if (!cmd)
                return NULL;
        len = strlen(cmd);

        for (cmdtp = table; cmdtp != table + table_len; cmdtp++) {
                if (strncmp(cmd, cmdtp->name, len) == 0) {
                        if (len == strlen(cmdtp->name))
                                return cmdtp;   /* full match */

                        cmdtp_temp = cmdtp;     /* abbreviated command ? */
                        n_found++;
                }
        }
        if (n_found == 1) {                     /* exactly one match */
                return cmdtp_temp;
        }

        return NULL;    /* not found or ambiguous command */
}

static int cmd_call(struct cmd_tbl_s *cmdtp, int flag, int argc, char * const argv[])
{
        int result;

        result = (cmdtp->cmd)(cmdtp, flag, argc, argv);
        if (result)
                PRINTF("Command failed, result=%d\n", result);
        return result;
}

int cmd_usage(const struct cmd_tbl_s *cmdtp)
{
        PRINTF("%s - %s\n\n", cmdtp->name, cmdtp->usage);

        return 1;
}

int  cmd_process(int flag, int argc, char * const argv[])
{
        int rc = 0;
        struct cmd_tbl_s *cmdtp;

        /* Look up command in command table */
        cmdtp = find_cmd_tbl(argv[0], cmdlist, sizeof(cmdlist)/sizeof(cmdlist[0]));
        if (cmdtp == NULL) {
                PRINTF("Unknown command '%s' - try 'help'\r\n", argv[0]);
                return 1;
        }

        /* found - check max args */
        if (argc > cmdtp->maxargs)
                rc = 2;

        /* If OK so far, then do the command */
        if (!rc) {
                rc = cmd_call(cmdtp, flag, argc, argv);
        }
        if (rc == 2)
                rc = cmd_usage(cmdtp);
        return rc;
}

int cli_simple_parse_line(char *line, char *argv[])
{
        int nargs = 0;
        int i;

        PRINTF("%s: \"%s\"\r\n", __func__, line);
        while (nargs < CONFIG_SYS_MAXARGS) {
                /* skip any white space */
                while (isblank(*line))
                        ++line;

                if (*line == '\0') {    /* end of line, no more args    */
                        argv[nargs] = NULL;
                        PRINTF("%s: nargs=%d\r\n", __func__, nargs);
                        return nargs;
                }

                argv[nargs++] = line;   /* begin of argument string     */

                /* find end of string */
                while (*line && !isblank(*line))
                        ++line;

                if (*line == '\0') {    /* end of line, no more args    */
                        argv[nargs] = NULL;
                        PRINTF("parse_line1: nargs=%d\r\n", nargs);
                        for (i = 0; i < nargs; i++)
                        	PRINTF("%s ", argv[i]);
                        return nargs;
                }

                *line++ = '\0';         /* terminate current arg         */
        }



        return nargs;
}

int simple_run_command(char *cmd, int flag)
{
    char *str = cmd;
    char *argv[CONFIG_SYS_MAXARGS + 1];     /* NULL terminated      */
    int argc;
    int rc = 0;

	PRINTF("[RUN_COMMAND] cmd[%s]\r\n", cmd);

    if (*str) {

            /* Extract arguments */
            argc = cli_simple_parse_line(cmd, argv);
            if (argc == 0) {
                    return -1;
            }

            if(flag == 2)
            {
            	PRINTF("run test\r\n");
            }
            else
            {
				if (cmd_process(flag, argc, argv))
						rc = -1;
            }

    }

    return rc;
}

static const char erase_seq[] = "\b \b";        /* erase sequence */
static const char   tab_seq[] = "        ";     /* used to expand TABs */

int getc(void)
{
	int error;
	unsigned char c;

	error = UART_RTOS_Receive(&console_handle, &c, 1, NULL);
	if (!error)
		return (int)c;
	else
		return error;
}

void putc(const char c)
{
	  UART_RTOS_Send(&console_handle, (uint8_t *)&c, 1);
}

void puts(const char *str)
{
        while (*str)
                putc(*str++);
}

static char *delete_char (char *buffer, char *p, int *colp, int *np, int plen)
{
        char *s;

        if (*np == 0)
                return p;

        if (*(--p) == '\t') {           /* will retype the whole line */
                while (*colp > plen) {
                        puts(erase_seq);
                        (*colp)--;
                }
                for (s = buffer; s < p; ++s) {
                        if (*s == '\t') {
                                puts(tab_seq + ((*colp) & 07));
                                *colp += 8 - ((*colp) & 07);
                        } else {
                                ++(*colp);
                                putc(*s);
                        }
                }
        } else {
                puts(erase_seq);
                (*colp)--;
        }
        (*np)--;

        return p;
}

int readline_to_buffer(char *buffer)
{
	char *p = buffer;
    char *p_buf = p;
    int     n = 0;                          /* buffer index         */
    int     plen = 0;                       /* prompt length        */
    int     col;                            /* output column cnt    */
    char    c;


    for (;;) {

        col = plen;
            c = getc();
            /*
             * Special character handling
             */
            switch (c) {
            case '\r':                      /* Enter                */
            case '\n':
                    *p = '\0';
                    puts("\r\n");
                    return p - p_buf;

            case '\0':                      /* nul                  */
                    continue;

            case 0x03:                      /* ^C - break           */
                    p_buf[0] = '\0';        /* discard input */
                    return -1;

            case 0x15:                      /* ^U - erase line      */
                    while (col > plen) {
                            puts(erase_seq);
                            --col;
                    }
                    p = p_buf;
                    n = 0;
                    continue;

            case 0x17:                      /* ^W - erase word      */
                    p = delete_char(p_buf, p, &col, &n, plen);
                    while ((n > 0) && (*p != ' '))
                            p = delete_char(p_buf, p, &col, &n, plen);
                    continue;

            case 0x08:                      /* ^H  - backspace      */
            case 0x7F:                      /* DEL - backspace      */
                    p = delete_char(p_buf, p, &col, &n, plen);
                    continue;

            default:
                /*
                 * Must be a normal character then
                 */
                if (n < 62) {
                		char buf[2];
                       *p++ = c;
                        ++n;
                        ++col;
                        buf[0] = c;
                        buf[1] = '\0';
                        puts(buf);
                } else {                        /* Buffer full */
                        putc('\a');
                }
            }
    }
}

void uart_task(void *pvParameters)
{
    int n;

    console_config.srcclk = CLOCK_GetFreq(CONSOLE_UART_CLKSRC);
    console_config.base = CONSOLE_UART;
    NVIC_SetPriority(CONSOLE_UART_RX_TX_IRQn, 8);

    // PRINTF("Test");

    if (0 > UART_RTOS_Init(&console_handle, &t_console_handle, &console_config))
    {
        PRINTF("Error during UART initialization.\r\n");
        vTaskSuspend(NULL);
    }

    /* Send data */
    /* Send data */
    for (;;)
    {
//        /* Send some data */
        puts(to_send);

        n = readline_to_buffer(recv_buffer);
        if (n > 0)
        {
            /* send back the received data */
        	 //strlcpy(lastcommand, recv_buffer, CONFIG_SYS_CBSIZE + 1);
             strncpy(lastcommand, recv_buffer, n);
             lastcommand[n] = '\0';
        	 PRINTF("%s\r\n", lastcommand);
        	 simple_run_command(lastcommand, 0);
        }

//        do
//        {
//            error = UART_RTOS_Receive(&console_handle, recv_buffer, 1/*sizeof(recv_buffer)*/, &n);
//            if (error == kStatus_UART_RxHardwareOverrun)
//            {
//                /* Notify about hardware buffer overrun */
//                if (kStatus_Success != UART_RTOS_Send(&console_handle, (uint8_t *)send_hardware_overrun, strlen(send_hardware_overrun)))
//                {
//                    vTaskSuspend(NULL);
//                }
//            }
//            if (error == kStatus_UART_RxRingBufferOverrun)
//            {
//                /* Notify about ring buffer overrun */
//                if (kStatus_Success != UART_RTOS_Send(&console_handle, (uint8_t *)send_ring_overrun, strlen(send_ring_overrun)))
//                {
//                    vTaskSuspend(NULL);
//                }
//            }
//            if (n > 0)
//            {
//                /* send back the received data */
//                UART_RTOS_Send(&console_handle, (uint8_t *)recv_buffer, n);
//            }
//        } while (kStatus_Success == error);
//        UART_RTOS_Deinit(&console_handle);


    }

    vTaskSuspend(NULL);
}
