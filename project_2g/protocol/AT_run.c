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
#include "fsl_adc16.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "aes256.h"
#include "sha1.h"
#include "spiflash_config.h"
#include "fota.h"
#include "rtc_fun.h"
#include "message.h"
#include "hb_protocol.h"
#include "AT_execute.h"
#include "fota.h"
#include "audio_play.h"

HWTEST_STATUS_t HwTestResult;

AT_RUN_MSG_t	AtRunMsg;
QueueHandle_t 	at_run_handle;


void get_nordata(){
	//PRINTF("GET NORDATA %d\r\n", nor_data_addr);
	;
}

void at_run_task(void *p)
{
	int 		local_conf, desire_conf;
	float		freq_pc, freq_local;
	char 		id_buf[9]={0};
	uint32_t	reget_freq_count=0, reget_nor_n=0;

	uint16_t	blk_num=0;

	at_run_handle  = xQueueCreate(1, sizeof(AT_RUN_MSG_t));

	memset(&HwTestResult, true, sizeof(HWTEST_STATUS_t));

//	while(1)
//	{
//		xQueueReceive(at_run_handle, &AtRunMsg, portMAX_DELAY);
//		if (AtRunMsg.cmd == HW_START) break;
//	}
	init_A7139();
	PPP_RED_LED_INIT();
	PPP_GREEN_LED_INIT();
	spi_flash_test();
	if (GET_RTC_SECONDS = 0) PRINTF("ERR %d\r\n", ERR_RTC);
	check_battery();

	PRINTF("GETPRODUCEINFO\r\n");

	while(1)
	{
		xQueueReceive(at_run_handle, &AtRunMsg, portMAX_DELAY);

		switch (AtRunMsg.cmd)
		{
		case HW_PRODUCEINFO:
			//PRINTF("%s\r\n", AtRunMsg.data);
			SETPRODUCEINFO(AtRunMsg.data);
			blk_num = 0;
			PRINTF("GETLONGID\r\n");
			break;
		case HW_LONGID:
			//PRINTF("%s\r\n", AtRunMsg.data);
			memcpy(id_buf, AtRunMsg.data, 8);
			sscanf(id_buf, "%x", &hbb_info.config.longidh);
			memcpy(id_buf, AtRunMsg.data+8, 8);
			sscanf(id_buf, "%x", &hbb_info.config.longidm);
			memcpy(id_buf, AtRunMsg.data+16, 8);
			sscanf(id_buf, "%x", &hbb_info.config.id);
			sync_hbb_info();
			get_hbb_info();

			SPI_A7139_TX_TEST(hbb_info.config.freq);
			PRINTF("GETFREQ\r\n");
			break;
		case HW_FREQ:
			//PRINTF("%s\r\n", AtRunMsg.data);
			freq_pc = strtol(AtRunMsg.data, NULL, 10);
			freq_local = 868651;
			if ((freq_pc >= (freq_local-15)) && (freq_pc <= (freq_local+15))){
				if ((freq_pc >= (freq_local-3)) && (freq_pc <= (freq_local+3))){
					blk_num = 0;
					PRINTF("PUTFREQ %X\r\n", hbb_info.config.freq);
					PRINTF("GETNORDATAAUDIO %d\r\n", blk_num++);
				}else{
					vTaskDelay(1000);
					if (++reget_freq_count%3) {
						PRINTF("GETFREQ\r\n");
					}else if (12==reget_freq_count){
						PRINTF("ERR %d\r\n", ERR_FREQ);
					}else{
						local_conf = hbb_info.config.freq;
						desire_conf = (freq_pc/1000-12.8*67)*65536/12.8 - local_conf;
						desire_conf = local_conf - desire_conf;

						hbb_info.config.freq = desire_conf;
						//freq_local - freq_pc
						printf("%x\r\n", desire_conf);
						sync_hbb_info();
						get_hbb_info();
						SPI_A7139_TX_TEST(hbb_info.config.freq);
						PRINTF("GETFREQ\r\n");
					}
				}
			}else{
				PRINTF("ERR %d\r\n", ERR_FREQ);
			}
			break;
		case HW_NORDATAAUDIO:
			//PRINTF("%s\r\n", AtRunMsg.data);
			//PRINTF("%d,%d\r\n", AtRunMsg.blk_num, strlen(AtRunMsg.data));
			if (AtRunMsg.blk_num == 0xffff) {
				blk_num = 0;
				start_play_plan(1);
				vTaskDelay(2000);
				PRINTF("GETNORDATAIMG %d\r\n", blk_num++);
			}else if (AtRunMsg.blk_num != blk_num-1) {
				PRINTF("ERR %d\r\n", ERR_N_AUDIO);
			}else{
				if (0>SETNORDATAAUDIO(AtRunMsg.data, AtRunMsg.blk_num*0x1000+AUDIO_DATA_ADDR_START, AtRunMsg.b64_len)){
					if (3 < ++reget_nor_n){
						reget_nor_n = 0;
						PRINTF("ERR %d\r\n", ERR_W_AUDIO);
					}else{
						PRINTF("GETNORDATAAUDIO %d\r\n", blk_num-1);
					}
				}else{
					reget_nor_n = 0;
					PRINTF("GETNORDATAAUDIO %d\r\n", blk_num++);
				}
			}
			break;
		case HW_NORDATAIMG:
			//PRINTF("%s\r\n", AtRunMsg.data);
			//PRINTF("%d\r\n", AtRunMsg.blk_num);
			if (AtRunMsg.blk_num == 0xffff) {
				PRINTF("ALLPASS\r\n");
			}else if (AtRunMsg.blk_num != blk_num-1) {
				PRINTF("ERR %d\r\n", ERR_N_IMG);
			}else{
				if (0>SETNORDATAIMG(AtRunMsg.data, AtRunMsg.blk_num*0x1000+FOTA_HBB_IMG0_ADDR_START, AtRunMsg.b64_len)){
					if (3 < ++reget_nor_n){
						reget_nor_n = 0;
						PRINTF("ERR %d\r\n", ERR_W_IMG);
					}else{
						PRINTF("GETNORDATAIMG %d\r\n", blk_num-1);
					}
				}else{
					reget_nor_n = 0;
					PRINTF("GETNORDATAIMG %d\r\n", blk_num++);
				}
			}
			break;
		case HW_RESET:
			PRINTF("HBB Restarting...\r\n");
			NVIC_SystemReset();
			break;
		}
	}
}
