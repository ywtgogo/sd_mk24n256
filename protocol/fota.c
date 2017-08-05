/* FreeRTOS kernel includes. */
#define PR_MODULE "fota\1"

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "timers.h"
#include "semphr.h"
#include "sys.h"

/* Freescale includes. */
#include "fsl_device_registers.h"
#include "fsl_debug_console.h"
#include "fsl_common.h"
#include "fsl_pflash_freertos.h"

/* User include */
#include "A7139.h"
#include "hb_protocol.h"
#include "SPI_A7139.h"
#include "spiflash_config.h"
#include "message.h"
#include "aes256.h"
#include "fota.h"
#include "uplink_protocol.h"
#include "sha1.h"
#include "uart_ppp.h"
#include "version.h"
#ifdef PLC
#include "uart_plc.h"
#include "plc_if.h"
#endif

flash_rtos_handle_t pflash_handle;

QueueHandle_t 	fota_rq_handle;
uint8_t     	fota_data_read[4096]={0};
TimerHandle_t 	TimingFotaHandle = NULL;
TimerHandle_t 	TimeoutFotaHandle = NULL;
TimerHandle_t 	RestartCheckHandle = NULL;

LONG_ID96 		fail_sensor_lisr[MAX_SENSOR_COUNT];
uint32_t 		SmartWaterCount;
uint32_t 		SmartMagCount;
uint32_t 		SmartPirCount;
FOTA_Status		FotaStatusFlag;

void update_fota_timing(uint32_t newval)
{
	if( xSemaphoreTake( hbb_info.xSemaphore, 1000) == pdTRUE)
	{
		hbb_info.config.fota_check_interval = newval;
		xSemaphoreGive(hbb_info.xSemaphore);
		sync_hbb_info();
		xTimerStop(TimingFotaHandle, 0);
		if( xTimerChangePeriod(TimingFotaHandle, hbb_info.config.fota_check_interval, 0) != pdPASS )
		{
			PRINTF("change fota timer failed\r\n");
		}
		if (xTimerStart(TimingFotaHandle, 0) != pdPASS)
		{
			PRINTF("start fota timer failed\r\n");
		}
	} else {
		PRINTF("%s failed\r\n", __func__);
		return ;
	}
}

static void TimeoutFotaUpdateCheckCallback(TimerHandle_t xTimer)
{
	FOTA_RQ_MSG msg;
	msg.cmd = FW_TIMEOUT_FOTA;
	xQueueSend(fota_rq_handle, &msg, portMAX_DELAY);
}

static void TimingFotaUpdateCheckCallback(TimerHandle_t xTimer)
{
	FOTA_RQ_MSG msg;
	msg.cmd = FW_TIMING_CHECK_FOTA;
	xQueueSend(fota_rq_handle, &msg, portMAX_DELAY);
}

extern uint32_t sos_incoming_call_status;
static void TimingRestartCheckCallback(TimerHandle_t xTimer)
{
	bool HoldReset = false;
	EventBits_t uxBits;
	uxBits = xEventGroupGetBits(hbbeventgroup);
	if ((uxBits&HTTP_QUEUE_EVENT) != 0){
		PRINTF("HoldReset:HTTP_QUEUE\r\n");
		HoldReset = true;
	}
	for(uint32_t i = 0; i<MAX_SENSOR_COUNT; i++)
	{
		if( (sensor_status[i].send_message.alarm_status == 1) &&
			(sensor_status[i].send_message.query_acked == 1) )
		{
			PRINTF("HoldReset:alarm&ack\r\n");
			HoldReset = true;
			break;
		}
	}
	if(sos_incoming_call_status)
	{
		PRINTF("HoldReset:sos_incoming_call\r\n");
		HoldReset = true;
	}
	/* check log safety */
	if (FotaStatusFlag != Done/* new fota now? */) {
		PRINTF("TimingFotaHandle release\r\n");
		xTimerStop(TimingFotaHandle, 0);
		return;
	} else if (HoldReset) {
		/* reset timer */
		xTimerStop(TimingFotaHandle, 0);
		if( xTimerChangePeriod(RestartCheckHandle, 30000U, 0) != pdPASS )
		{
			PRINTF("change RestartCheck timer failed\r\n");
		}
		if (xTimerStart(RestartCheckHandle, 0) != pdPASS)
		{
			PRINTF("start RestartCheck timer failed\r\n");
		}
	} else{
		PRINTF("HBB Restarting...\r\n");
		NVIC_SystemReset();
	}
}

static void SysRestart()
{
	/* need software timer */
	xTimerStart(RestartCheckHandle, 0);
	/* reset sys */
}

static void check_hbb_image_fota_status(void)
{
//	uint32_t 		fota_sw_temp1, fota_sw_temp2;
//	uint32_t		new_ver;
	UPLINK_Q_MSG	uplink_tq_msq;
	BOOT_RECORD		boot_record;

//	norflash_read_data_ll(&flash_master_rtos_handle, FOTA_HBB_IMG1_ADDR_START+FOTA_HBB_IMG_SWVER_OFFSET, 4U, (uint8_t *)&fota_sw_temp1);
//	norflash_read_data_ll(&flash_master_rtos_handle, FOTA_HBB_IMG2_ADDR_START+FOTA_HBB_IMG_SWVER_OFFSET, 4U, (uint8_t *)&fota_sw_temp2);
//	new_ver = ver_info.swver;

	uplink_tq_msq.cmd = UPLINK_FW_UPD_RESULT;
	uplink_tq_msq.fw_upd_result.rid = READ32((uint32_t*)(FOTA_PRAM_ADDR +(sizeof(hbb_config)|0x07)+1));
//	if (fota_sw_temp1 == new_ver || fota_sw_temp2 == new_ver /*|| bootinfo*/){
//		uplink_tq_msq.fw_upd_result.rc	= REQ_SUCC;
//	}else{
//		uplink_tq_msq.fw_upd_result.rc	= ALL_DEV_FOTA_FAIL;
//	}

	if ((uplink_tq_msq.fw_upd_result.rid != 0xffffffff) && (uplink_tq_msq.fw_upd_result.rid != 0x0))
	{
		/* bootloader record info check */
		boot_record = (BOOT_RECORD)(*(BOOT_RECORD *)(0x2002f000));	//(uint8_t)(*(uint8_t*)(address))
		PRINTF("\r\nbootloader record------" );
		for (uint32_t i=0; i<sizeof(BOOT_RECORD); i++){
			PRINTF("*%02x", (uint8_t)(*((uint8_t*)(&boot_record)+i)));
		}
		PRINTF("\r\n------bootloader record 0x%x\r\n", &boot_record);
		if (boot_record.fw_change == FW_UPDATE_FAILED_BLOCK_SHA1_ERR) {
			uplink_tq_msq.fw_upd_result.rc = BOOT_HBB_BLK_SHA1_ERR;
			PRINTF("FW_UPDATE_FAILED_BLOCK_SHA1_ERR\r\n");
		}else if (boot_record.fw_change == FW_UPDATE_FAILED_IMG_SHA1_ERR) {
			uplink_tq_msq.fw_upd_result.rc = BOOT_HBB_ALL_SHA1_ERR;
			PRINTF("FW_UPDATE_FAILED_IMG_SHA1_ERR\r\n");
		}else if (boot_record.fw_change == FW_ROLLBACK) {
			uplink_tq_msq.fw_upd_result.rc = BOOT_HBB_VER_ROOLBACK;
			PRINTF("FW_ROLLBACK\r\n");
		}else if (boot_record.fw_change == FW_UPDATE_SUCCESS) {
			uplink_tq_msq.fw_upd_result.rc = REQ_SUCC;
			PRINTF("FW_UPDATE_SUCCESS\r\n");
		}else if (boot_record.fw_change == FW_NOCHANGE) {
			PRINTF("FW_NOCHANGE\r\n");
			return;
		}else {
			PRINTF("BOOTLOADER GIVEN BAD RESULT CODE %d\r\n");
			return;
		}

		PRINTF("HBB_FOTA_RESULT_SEND_BACK %d\r\n", uplink_tq_msq.fw_upd_result.rc);
		xQueueSend(uplink_q_handle, &uplink_tq_msq, portMAX_DELAY);
		sync_hbb_info();
	}
}

#ifdef BRIDGE
PLC_FOTA_INFO fota_info;
FOTA_SENSOR_4KHEAD fw_header;

bool fw_need_update()
{
	uint32_t sw_old;
	uint32_t sw_new;
	PRINTF("FOTA: Check if fw need update\r\n ");
	if(BRIDGE_GROUP == (fota_info.device_type>>24))
	{
		//todo
	}
	else if(SENSOR_GROUP == (fota_info.device_type>>24))
	{
		for (uint32_t i=0; i<MAX_SENSOR_COUNT; i++)
		{
			if( (sensor_status[i].send_message.sn_mitt == hbb_info.config.id) && (fota_info.fw_sw != GET_SENSOR_SW_Ver(&sensor_status[i])))
				return true;
		}
		return false;
	}
	else
	{
		PRINTF("FOTA: Wrong device type :%x\r\n",fota_info.device_type);
		return false;
	}
}

bool is_fw_header_right()
{
	uint8_t header_sha1[20] ={0};
	sha1_csum(plc_recv_fw_block.data + 32, FOTA_IMG_CELL_DOWN_LEN - 32,header_sha1);
	for(uint32_t j = 0; j<20; j++)
	{
		if(header_sha1[j]!=plc_recv_fw_block.data[j])
		{
			PRINTF("FOTA: get header sha1 error\r\n");
			return false;
		}
	}
	PRINTF("FOTA: get header sha1 pass\r\n");
	return true;
}

bool is_fw_block_right()
{
	uint8_t block_sha1[20] ={0};
	sha1_csum(plc_recv_fw_block.data, FOTA_IMG_CELL_DOWN_LEN, block_sha1);
	uint8_t block_sha1_from_header[20];

	if( (BRIDGE_GROUP == DEVICE_TYPE_Group(fota_info.device_type))|| fota_info.fw_block_down<=4)
		memcpy(&block_sha1_from_header,&fw_header.sub_node[0].block_sha1[0]+(fota_info.fw_block_down-1)*CELL_SHA1_UNIT,CELL_SHA1_UNIT);
	else
		memcpy(&block_sha1_from_header,&fw_header.sub_node[1].block_sha1[0]+(fota_info.fw_block_down-5)*CELL_SHA1_UNIT,CELL_SHA1_UNIT);
	for(uint32_t j = 0; j<20; j++)
	{
		if(block_sha1[j]!=block_sha1_from_header[j])
		{
			PRINTF("FOTA: block %d sha1 error\r\n",fota_info.fw_block_down);
			return false;
		}
	}
	PRINTF("FOTA: get block NO. %d sha1 pass\r\n",fota_info.fw_block_down);
	return true;
}

bool is_fw_img_right()
{
//todo?
	return true;
}

void plc_fota_task(void *pvParameters)
{
	PLC_MSG plc_fota_msg;
	uint32_t res;
	uint64_t ts;
	uint8_t write_cnt = 0;
	while(1)
	{
		memset(&fota_info,0x00,sizeof(fota_info));
		memset(fota_data_read, 0xff, FOTA_IMG_CELL_DOWN_LEN);
		xQueueReceive(plc_fota_queue, &plc_fota_msg,portMAX_DELAY);
		fota_info.device_type = plc_fota_msg.device_type;
		fota_info.fw_mitt = plc_fota_msg.sn_mitt;
		fota_info.fw_update_seq = plc_fota_msg.sequence;
		//request header
		do{
			plc_request_img_block(fota_info.device_type,fota_info.fw_block_down,fota_info.fw_update_seq);
			fota_info.fw_block_try_cnt++;
			res = xSemaphoreTake(plc_fota_block_sem,PLC_WAIT_IMG_BLOCK_TIME_OUT*2);
		}while((pdPASS != res|| !is_fw_header_right()) &&(fota_info.fw_block_try_cnt < PLC_MAX_REQ_IMG_BLOCK_CNT));

		//request header 5 times timeout , give up.
		if(fota_info.fw_block_try_cnt==PLC_MAX_REQ_IMG_BLOCK_CNT)
		{
			plc_fw_update_result(fota_info.device_type,FOTA_FAILED,fota_info.fw_update_seq);
			continue;
		}

		memcpy(&fw_header,&plc_recv_fw_block.data,sizeof(fw_header));
		fota_info.fw_sw = *(uint32_t *) &(fw_header.sw_ver);
		//fw is not needed, give up
		if(fw_need_update())
		{
			if(SENSOR_GROUP == DEVICE_TYPE_Group(fota_info.device_type))
			{
				fota_info.fw_block_num = FOTA_SENSOR_IMG_BLOCK_NUM;
				fota_info.fw_sflash_addr = FOTA_SENSOR_IMG_ADDR_START;
			}
			else
			{
				fota_info.fw_block_num  = (fw_header.sub_node[0].length+ 4096 - 1) >> 12;
				fota_info.fw_sflash_addr = FOTA_BRIDGE_ADDR_START;
			}
			norflash_block_erase_ll(&flash_master_rtos_handle, fota_info.fw_sflash_addr);
			write_cnt = 0;
			while(!norflash_write_data_ll(&flash_master_rtos_handle, fota_info.fw_sflash_addr, FOTA_IMG_CELL_DOWN_LEN, plc_recv_fw_block.data) &&
				  write_cnt < 5)
			{
				write_cnt++;
				PRINTF("FOTA: write header into spi-flash error\r\n",fota_info.fw_block_down);
			}
		}
		else
		{
			plc_fw_update_result(fota_info.device_type,FOTA_FAILED,fota_info.fw_update_seq);
			continue;
		}

		//request all img block
		write_cnt = 0;
		fota_info.fw_block_try_cnt = 0;
		fota_info.fw_block_down++;
		do
		{
			plc_request_img_block(fota_info.device_type,fota_info.fw_block_down,++plc_local_seq);
			fota_info.fw_block_try_cnt++;
			res = xSemaphoreTake(plc_fota_block_sem,PLC_WAIT_IMG_BLOCK_TIME_OUT*2);
			if(pdPASS != res)
			{
				PRINTF("FOTA: wait block timeout \r\n");
			}
			else if(is_fw_block_right())
			{
				//if block sha1 pass , resotre into spi-flash ,request next img block

				write_cnt = 1;
				while(!norflash_write_data_ll(&flash_master_rtos_handle, fota_info.fw_sflash_addr+FOTA_IMG_CELL_DOWN_LEN*fota_info.fw_block_down, FOTA_IMG_CELL_DOWN_LEN, plc_recv_fw_block.data) &&
						write_cnt < 5)
				{
					write_cnt++;
					PRINTF("FOTA: write block NO.%d into spi-flash error\r\n",fota_info.fw_block_down);
				}
				if(write_cnt == 5){
					break;
				}
				else
				{
					PRINTF("FOTA: write block NO.%d into spi-flash success\r\n",fota_info.fw_block_down);
				}

				fota_info.fw_block_try_cnt = 0;
				fota_info.fw_block_down++;
				if(fota_info.fw_block_down>fota_info.fw_block_num)
				{
					break;
				}
			}
		}while(fota_info.fw_block_try_cnt<PLC_MAX_REQ_IMG_BLOCK_CNT);

		if((write_cnt == 5) || (fota_info.fw_block_try_cnt == PLC_MAX_REQ_IMG_BLOCK_CNT) || !is_fw_img_right())
		{
			plc_fw_update_result(fota_info.device_type,FOTA_FAILED,fota_info.fw_update_seq);
			continue;
		}
		if(SENSOR_GROUP == DEVICE_TYPE_Group(fota_info.device_type))
		{
			uint8_t rf_fota_cnt = 0;
			do
			{
				//sent rf img to fota
				rf_fota_cnt++;
				for (uint32_t i=0; i<MAX_SENSOR_COUNT; i++)
				{
					if((sensor_status[i].send_message.sn_mitt == hbb_info.config.id) &&
					   (DEVICE_TYPE_Function(sensor_status[i].device_type) == DEVICE_TYPE_Function(fota_info.device_type)) &&
					   (GET_SENSOR_SW_Ver(&sensor_status[i]) != fota_info.fw_sw))
					{
						rf_send_image_norflash(sensor_status[i].send_message.sn_dest);
					}
				}
				vTaskDelay(1000);
				fota_check_sensor_status(device_type);
			}while(fw_need_update() && (rf_fota_cnt<=5));

			if(rf_fota_cnt==5)
			{
				plc_fw_update_result(fota_info.device_type,FOTA_FAILED,fota_info.fw_update_seq);
				continue;
			}

			plc_fw_update_result(fota_info.device_type,FOTA_SUCCEED,fota_info.fw_update_seq);
		}
		else if(BRIDGE_GROUP == DEVICE_TYPE_Group(fota_info.device_type))
		{
			//restart
			PRINTF("BRIDGE Restarting...\r\n");
			NVIC_SystemReset();
		}

	}
}
#else
void fota_task(void *pvParameters)
{
    uint8_t		sha1_temp[CELL_SHA1_UNIT];
    uint8_t		sha1_calc_out[CELL_SHA1_UNIT];
    uint8_t		img_blk_num = 0;
    uint32_t	hbb_img_blk_num = 0;
    uint32_t 	bridge_img_blk_num = 0;
    uint32_t	device_type = 0;
    uint32_t 	rule_id;
    uint32_t	hw_ver;
    uint32_t 	fw_u_ver =0 ;
    uint32_t	if_more = 0;
    uint32_t	fw_blks = 0;
    uint32_t	re_num = 0;
	uint32_t	fota_hbb_addr_start = FOTA_HBB_IMG2_ADDR_START;
	uint32_t 	fota_sw_temp1, fota_sw_temp2;
	EventBits_t	uxBits;

    FOTA_RQ_MSG		fota_rq_msg;
    UPLINK_Q_MSG	uplink_tq_msq;

    vTaskDelay(5000);
    //norflash_chip_erase_ll(&flash_master_rtos_handle);
    PRINTF("HBB_FOTA_RESULT_CHECK\r\n");
    check_hbb_image_fota_status();

    /* Create the software timer. */
    TimingFotaHandle = xTimerCreate("timing_sensor_fota",     /* Text name. */
                               (hbb_info.config.fota_check_interval), 			/* Timer period.  */
							   	 pdTRUE,                   	/* Enable auto reload. */
                                 0,                         /* ID is not used. */
								 TimingFotaUpdateCheckCallback);    /* The callback function. */
    xTimerStart(TimingFotaHandle, 0);

    TimeoutFotaHandle = xTimerCreate("timeout_sensor_fota",     /* Text name. */
                               (15*60*1000U), 					/* Timer period.  */
							   	 pdTRUE,                   	/* Enable auto reload. */
                                 0,                         /* ID is not used. */
								 TimeoutFotaUpdateCheckCallback);    /* The callback function. */

    RestartCheckHandle = xTimerCreate("timing_restart_check",     /* Text name. */
                               (1000U), 					/* Timer period.  */
							   	 pdFALSE,                   /* Enable auto reload. */
                                 0,                         /* ID is not used. */
								 TimingRestartCheckCallback);    /* The callback function. */

	while(1)
	{
		xQueueReceive(fota_rq_handle, &fota_rq_msg, portMAX_DELAY);
		PRINTF("!------FOTA------!\r\n");

		switch (fota_rq_msg.cmd)
		{
		case FW_SMS_CHECK_FOTA:
			PRINTF("RECVED FW_SMS_CHECK_FOTA\r\n");
			update_fota_timing(hbb_info.config.fota_check_interval);
	    	memset(&uplink_tq_msq, 0, sizeof(UPLINK_Q_MSG));
	    	uplink_tq_msq.cmd = UPLINK_FW_UPD_REQ;
	    	uplink_tq_msq.fw_req.bid = 0;
	    	xQueueSend(uplink_q_handle, &uplink_tq_msq, portMAX_DELAY);
			break;
		case FW_TIMING_CHECK_FOTA:
			PRINTF("RECVED FW_TIMING_CHECK_FOTA\r\n");
			if ((hbb_img_blk_num != 0) || (img_blk_num != 0)){
				PRINTF("FOTA_TASK_BUSY\r\n");
				break;
			}
	    	memset(&uplink_tq_msq, 0, sizeof(UPLINK_Q_MSG));
	    	uplink_tq_msq.cmd = UPLINK_FW_UPD_REQ;
	    	uplink_tq_msq.fw_req.bid = 0;
	    	xQueueSend(uplink_q_handle, &uplink_tq_msq, portMAX_DELAY);
			break;
		case FW_SOS_INCOMING_CALL:
			PRINTF("RECVED FW_SOS_INCOMING_CALL\r\n");
			if (FotaStatusFlag!=New) break;
			FotaStatusFlag = Abort;
			uplink_tq_msq.cmd				= UPLINK_FW_UPD_RESULT;
			uplink_tq_msq.fw_upd_result.rid = rule_id;
			uplink_tq_msq.fw_upd_result.rc 	= HBB_CALL_INCOMING;
			PRINTF("HBB_FOTA_RESULT_SEND_BACK %d\r\n", uplink_tq_msq.fw_upd_result.rc);
			xQueueSend(uplink_q_handle, &uplink_tq_msq, portMAX_DELAY);
			vTaskDelay(10000);
			hbb_img_blk_num = 0;
			img_blk_num = 0;
			break;
		case FW_TIMEOUT_FOTA:
			xTimerStop(TimeoutFotaHandle, 0);
			FotaStatusFlag = Abort;
			hbb_img_blk_num = 0;
			img_blk_num = 0;
			break;
		case FW_IMG_DOWNLOAD_FAIL:
			PRINTF("RECVED FW_IMG_DOWNLOAD_FAIL\r\n");
			FotaStatusFlag = Abort;
			uplink_tq_msq.cmd				= UPLINK_FW_UPD_RESULT;
			uplink_tq_msq.fw_upd_result.rid = rule_id;
			uplink_tq_msq.fw_upd_result.rc 	= DOWN_IMAGE_ABORT;
			PRINTF("HBB_FOTA_RESULT_SEND_BACK %d\r\n", uplink_tq_msq.fw_upd_result.rc);
			xQueueSend(uplink_q_handle, &uplink_tq_msq, portMAX_DELAY);
			vTaskDelay(10000);
			hbb_img_blk_num = 0;
			img_blk_num = 0;
			/* if has some more fota rule*/
			if (if_more) {
				if_more = 0;
				fota_rq_msg.cmd = FW_TIMING_CHECK_FOTA;
				xQueueSend(fota_rq_handle, &fota_rq_msg, portMAX_DELAY);
			}
			break;
		case FW_UPD_RESP:
			PRINTF("RECVED FW_UPD_RESP\r\n");
			device_type = fota_rq_msg.fw_upd_req.dt;
			rule_id		= fota_rq_msg.fw_upd_req.rid;
			hw_ver		= fota_rq_msg.fw_upd_req.hdv;
			fw_u_ver	= fota_rq_msg.fw_upd_req.fwuv;
			fw_blks		= fota_rq_msg.fw_upd_req.fwblks;
			if_more		= fota_rq_msg.fw_upd_req.ifm;

			/* tq_msq init */
			uplink_tq_msq.cmd 					= UPLINK_DOWNLOAD_IMG;
			uplink_tq_msq.fw_down.rid 			= fota_rq_msg.fw_upd_req.rid;
			uplink_tq_msq.fw_down.key 			= fota_rq_msg.fw_upd_req.key;
			uplink_tq_msq.fw_down.url	 		= fota_rq_msg.fw_upd_req.url;

			/* Suspend breakpoint continuingly */
			/* check extern charging */
			if (bat_volt_low_flag){
				uplink_tq_msq.fw_upd_result.rc = HBB_LOW_POWER;
				//uplink_tq_msq.fw_upd_result.rc 	= ALL_DEV_FOTA_FAIL;
				uplink_tq_msq.cmd = UPLINK_FW_UPD_RESULT;
				PRINTF("hbb low power, terminated update!\r\n");
				PRINTF("SENSOR_FOTA_RESULT_SEND_BACK %d\r\n", uplink_tq_msq.fw_upd_result.rc);
				xQueueSend(uplink_q_handle, &uplink_tq_msq, portMAX_DELAY);
				img_blk_num = 0;
				hbb_img_blk_num = 0;
				break;
			}

			if ((device_type>>24) == SENSOR_GROUP){
				FotaStatusFlag = New;
				xTimerStart(TimeoutFotaHandle, 0);
				uplink_tq_msq.fw_down.img_blk_num 	= img_blk_num;
				uplink_tq_msq.fw_down.back_cmd		= FW_IMG_BLK;
				/* check if need to update storage image */
				PRINTF("\r\n+++++++++++++++++++++++++++++++++++++++++++\r\n");
				PRINTF("erase start\r\n");
				norflash_block_erase_ll(&flash_master_rtos_handle, FOTA_SENSOR_IMG_ADDR_START);
				PRINTF("erase done!\r\n");
				xQueueSend(uplink_q_handle, &uplink_tq_msq, portMAX_DELAY);
			}else if ((device_type>>24) == BRIDGE_GROUP){
				uplink_tq_msq.fw_down.img_blk_num 	= bridge_img_blk_num;
				uplink_tq_msq.fw_down.back_cmd		= FW_BRIDGE_IMG_BLK;
				/* check if need to update storage image */
				PRINTF("\r\n+++++++++++++++++++++++++++++++++++++++++++\r\n");
				PRINTF("erase start\r\n");
				norflash_block_erase_ll(&flash_master_rtos_handle, FOTA_BRIDGE_ADDR_START);
				norflash_block_erase_ll(&flash_master_rtos_handle, FOTA_BRIDGE_ADDR_START+0x10000);
				norflash_block_erase_ll(&flash_master_rtos_handle, FOTA_BRIDGE_ADDR_START+0x20000);
				norflash_block_erase_ll(&flash_master_rtos_handle, FOTA_BRIDGE_ADDR_START+0x30000);
				PRINTF("erase done!\r\n");
				xQueueSend(uplink_q_handle, &uplink_tq_msq, portMAX_DELAY);
			}else if ((device_type>>24) == HOMEBOX_GROUP){
				FotaStatusFlag = New;
				xTimerStart(TimeoutFotaHandle, 0);
				uplink_tq_msq.fw_down.img_blk_num 	= hbb_img_blk_num;
				uplink_tq_msq.fw_down.back_cmd		= FW_HBB_IMG_BLK;
				/* need check more version info */

				/* check if need to update storage image */
				norflash_read_data_ll(&flash_master_rtos_handle, FOTA_HBB_IMG1_ADDR_START+FOTA_HBB_IMG_SWVER_OFFSET, 4U, (uint8_t *)&fota_sw_temp1);
				norflash_read_data_ll(&flash_master_rtos_handle, FOTA_HBB_IMG2_ADDR_START+FOTA_HBB_IMG_SWVER_OFFSET, 4U, (uint8_t *)&fota_sw_temp2);
				if (fota_sw_temp1 < fota_sw_temp2 && fota_sw_temp2 < fw_u_ver){
					fota_hbb_addr_start	= FOTA_HBB_IMG2_ADDR_START;
				}else if (fota_sw_temp2 < fota_sw_temp1 && fota_sw_temp1 < fw_u_ver){
					fota_hbb_addr_start	= FOTA_HBB_IMG1_ADDR_START;
				}else { //no image
					if (fota_sw_temp2 == 0xffffffff) fota_hbb_addr_start	= FOTA_HBB_IMG2_ADDR_START;
					if (fota_sw_temp1 == 0xffffffff) fota_hbb_addr_start	= FOTA_HBB_IMG1_ADDR_START;
				}
				PRINTF("\r\n+++++++++++++++++++++++++++++++++++++++++++\r\n");
				PRINTF("erase start\r\n");
				norflash_block_erase_ll(&flash_master_rtos_handle, fota_hbb_addr_start);
				norflash_block_erase_ll(&flash_master_rtos_handle, fota_hbb_addr_start+0x10000);
				norflash_block_erase_ll(&flash_master_rtos_handle, fota_hbb_addr_start+0x20000);
				norflash_block_erase_ll(&flash_master_rtos_handle, fota_hbb_addr_start+0x30000);
				PRINTF("erase done!\r\n");
				xQueueSend(uplink_q_handle, &uplink_tq_msq, portMAX_DELAY);
			}else{
				PRINTF("RECVED FW_UPD_RESP, NO DEVICE NEED UPGRADE\r\n");
			}
			break;
		case FW_IMG_BLK:
			PRINTF("RECVED FW_SENSOR_IMG_BLK\r\n");
			/* check extern charging */
			if (bat_volt_low_flag){
				FotaStatusFlag=Abort;
				uplink_tq_msq.fw_upd_result.rc = HBB_LOW_POWER;
				//uplink_tq_msq.fw_upd_result.rc 	= ALL_DEV_FOTA_FAIL;
				uplink_tq_msq.cmd = UPLINK_FW_UPD_RESULT;
				PRINTF("hbb low power, terminated update!\r\n");
				PRINTF("SENSOR_FOTA_RESULT_SEND_BACK %d\r\n", uplink_tq_msq.fw_upd_result.rc);
				xQueueSend(uplink_q_handle, &uplink_tq_msq, portMAX_DELAY);
				img_blk_num = 0;
				hbb_img_blk_num = 0;
				break;
			}
			if (FotaStatusFlag==Abort) {
				break;
			}
			if (img_blk_num==0){
				memcpy(sha1_temp, fota_data_read, CELL_SHA1_UNIT);
				//sha1_csum(fota_data_read, FOTA_IMG_CELL_DOWN_LEN, sha1_calc_out );
				//sha1_csum(fota_data_read, FOTA_IMG_CELL_DOWN_LEN-32U, sha1_calc_out_temp );
				sha1_csum(fota_data_read + 32U, FOTA_IMG_CELL_DOWN_LEN - 32U, sha1_calc_out);
				if (0 != memcmp(sha1_calc_out, sha1_temp ,(size_t)CELL_SHA1_UNIT)) {
					FotaStatusFlag = Bad;
					uplink_tq_msq.fw_upd_result.rc = DOWN_SHA1_ERR;
					//uplink_tq_msq.fw_upd_result.rc 	= ALL_DEV_FOTA_FAIL;
					uplink_tq_msq.cmd = UPLINK_FW_UPD_RESULT;
					PRINTF("sha1 check error, img_blk_num=%d \r\n", img_blk_num);
					PRINTF("SENSOR_FOTA_RESULT_SEND_BACK %d\r\n", uplink_tq_msq.fw_upd_result.rc);
					xQueueSend(uplink_q_handle, &uplink_tq_msq, portMAX_DELAY);
					img_blk_num = 0;
					/* if has some more fota rule*/
					if (if_more) {
						if_more = 0;
						fota_rq_msg.cmd = FW_TIMING_CHECK_FOTA;
						xQueueSend(fota_rq_handle, &fota_rq_msg, portMAX_DELAY);
					}
					break;
				}
				else {
					PRINTF("sha1 check pass , img_blk_num=%d \r\n", img_blk_num);
				}
				/* block0 check verion */

			}else if (img_blk_num > 4){
				norflash_read_data_ll (&flash_master_rtos_handle, FOTA_SENSOR_IMG_ADDR_START+FOTA_SENSOR_SHA1_OFFSET_SECOND+CELL_SHA1_UNIT*(img_blk_num-5), 20U, sha1_temp);
				sha1_csum(fota_data_read, FOTA_IMG_CELL_DOWN_LEN, sha1_calc_out );
				if (0 != memcmp(sha1_calc_out, sha1_temp ,(size_t)CELL_SHA1_UNIT)) {
					FotaStatusFlag = Bad;
					uplink_tq_msq.fw_upd_result.rc = DOWN_SHA1_ERR;
					//uplink_tq_msq.fw_upd_result.rc 	= ALL_DEV_FOTA_FAIL;
					uplink_tq_msq.cmd = UPLINK_FW_UPD_RESULT;
					PRINTF("sha1 check error, img_blk_num=%d \r\n", img_blk_num);
					PRINTF("SENSOR_FOTA_RESULT_SEND_BACK %d\r\n", uplink_tq_msq.fw_upd_result.rc);
					xQueueSend(uplink_q_handle, &uplink_tq_msq, portMAX_DELAY);
					img_blk_num = 0;
					/* if has some more fota rule*/
					if (if_more) {
						if_more = 0;
						fota_rq_msg.cmd = FW_TIMING_CHECK_FOTA;
						xQueueSend(fota_rq_handle, &fota_rq_msg, portMAX_DELAY);
					}
					break;
				}
				else {
					PRINTF("sha1 check pass , img_blk_num=%d \r\n", img_blk_num);
				}
			} else {
				norflash_read_data_ll (&flash_master_rtos_handle, FOTA_SENSOR_IMG_ADDR_START+FOTA_SENSOR_SHA1_OFFSET_START+CELL_SHA1_UNIT*(img_blk_num-1), 20U, sha1_temp);
				sha1_csum(fota_data_read, FOTA_IMG_CELL_DOWN_LEN, sha1_calc_out );
				if (0 != memcmp(sha1_calc_out, sha1_temp ,(size_t)CELL_SHA1_UNIT)) {
					FotaStatusFlag = Bad;
					uplink_tq_msq.fw_upd_result.rc = DOWN_SHA1_ERR;
					//uplink_tq_msq.fw_upd_result.rc 	= ALL_DEV_FOTA_FAIL;
					uplink_tq_msq.cmd = UPLINK_FW_UPD_RESULT;
					PRINTF("sha1 check error, img_blk_num=%d \r\n", img_blk_num);
					PRINTF("SENSOR_FOTA_RESULT_SEND_BACK %d\r\n", uplink_tq_msq.fw_upd_result.rc);
					xQueueSend(uplink_q_handle, &uplink_tq_msq, portMAX_DELAY);
					img_blk_num = 0;
					/* if has some more fota rule*/
					if (if_more) {
						if_more = 0;
						fota_rq_msg.cmd = FW_TIMING_CHECK_FOTA;
						xQueueSend(fota_rq_handle, &fota_rq_msg, portMAX_DELAY);
					}
					break;
				}
				else {
					PRINTF("sha1 check pass , img_blk_num=%d \r\n", img_blk_num);
				}
			}

			norflash_write_data_ll(&flash_master_rtos_handle, FOTA_SENSOR_IMG_ADDR_START+FOTA_IMG_CELL_DOWN_LEN*img_blk_num, FOTA_IMG_CELL_DOWN_LEN, fota_data_read);

			/*ok +1*/
			img_blk_num++;
			uplink_tq_msq.fw_down.img_blk_num = img_blk_num;
			/* continue req img_blk */
			if (img_blk_num < 9) {
				xQueueSend(uplink_q_handle, &uplink_tq_msq, portMAX_DELAY);
			}else{
				PRINTF("SENSOR IMG RECVED TOTAL BLK %d\r\n", img_blk_num);
				img_blk_num = 0;

retry_send_image:
				for(uint32_t i = 0; i<MAX_SENSOR_COUNT; i++)
				{
					if( (device_type == sensor_status[i].device_type) &&
					    (sensor_status[i].send_message.sn_mitt == hbb_info.config.id) &&
						(GET_SENSOR_SW_Ver(&sensor_status[i]) != fw_u_ver) &&
						(sensor_status[i].send_message.query_acked == 1) &&
						(sensor_status[i].send_message.alarm_status == 0) &&
						(sensor_status[i].send_message.vbatt == 3) )
					{
						if (DeviceTypeErr == rf_send_image_norflash(sensor_status[i].device_type, sensor_status[i].send_message.sn_dest))
						{
							FotaStatusFlag = Bad;
							uplink_tq_msq.cmd				= UPLINK_FW_UPD_RESULT;
							uplink_tq_msq.fw_upd_result.rid = rule_id;
							uplink_tq_msq.fw_upd_result.rc 	= DOWN_SENSOR_DEVTYPE_ERR;
							PRINTF("device type check error\r\n");
							PRINTF("SENSOR_FOTA_RESULT_SEND_BACK %d\r\n", uplink_tq_msq.fw_upd_result.rc);
							xQueueSend(uplink_q_handle, &uplink_tq_msq, portMAX_DELAY);
							img_blk_num = 0;
							/* if has some more fota rule*/
							if (if_more) {
								if_more = 0;
								fota_rq_msg.cmd = FW_TIMING_CHECK_FOTA;
								xQueueSend(fota_rq_handle, &fota_rq_msg, portMAX_DELAY);
							}
							break;
						}
					}
				}

				vTaskDelay(2000);
				fota_check_sensor_status(device_type);
#ifdef PLC
				plc_local_seq++;
				//todo send fw_request to all bridge
				for(uint32_t i = 0; i< 1;i++)
				{
					for(uint32_t retry=0; retry<5; retry++)
					{
						plc_fw_need_update(bridge_list[i].id,device_type,plc_local_seq);
						uxBits = xEventGroupWaitBits(BridgeFwReqAckEventGroup,  	/* The event group handle. */
											B_BRIDGE_FW_REQ_ACK,		/* The bit pattern the event group is waiting for. */
											pdTRUE,         			/* BIT will be cleared automatically. */
											pdTRUE,         			/* Don't wait for both bits, either bit unblock task. */
											(TickType_t) 200U); 			/* Block indefinitely to wait for the condition to be met. */
						if((uxBits & B_BRIDGE_FW_REQ_ACK ) != 0)
						{
							PRINTF("BRIDGE_SENSOR_FW_ACKED\r\n");
							break;
						}
						else
						{
							//  xTicksToWait ticks passed
							PRINTF("BRIDGE_FW_ACKED timeout retry=%d\r\n", retry);
							continue;
						}

					}

				}
				//todo wait plc result , check bridge-sensor status
				uxBits = xEventGroupWaitBits(BridgeFwReqAckEventGroup,  	/* The event group handle. */
											 B_SENSOR_FW_RET_ACK,		/* The bit pattern the event group is waiting for. */
											 pdTRUE,         			/* BIT will be cleared automatically. */
											 pdTRUE,         			/* Don't wait for both bits, either bit unblock task. */
											 (TickType_t) 600000U); 			/* Block indefinitely to wait for the condition to be met. */
				if((uxBits & B_SENSOR_FW_RET_ACK ) != 0)
				{
					PRINTF("GET_BRIDGE_SENSOR_RESULT\r\n");
				}
				else
				{
					PRINTF("BRIDGE_SENSOR_RESULT time out!!!\r\n");
				}

#endif
				//wait sensor fota update result check
				{
					UPLINK_Q_MSG 	fota_rc;
					uint32_t		need_fota = 0;
					uint32_t		fota_fail_num = 0;
					fota_rc.cmd	= UPLINK_FW_UPD_RESULT;
					fota_rc.fw_upd_result.rid = rule_id;
					fota_rc.fw_upd_result.rc = REQ_SUCC;
					for(uint32_t i = 0; i<MAX_SENSOR_COUNT; i++)
					{
						if(device_type == sensor_status[i].device_type)
						{
							need_fota++;
							if(GET_SENSOR_SW_Ver(&sensor_status[i]) != fw_u_ver)
							{
								fail_sensor_lisr[fota_fail_num].id = sensor_status[i].send_message.sn_dest;
								fail_sensor_lisr[fota_fail_num].idh = sensor_status[i].longidh;
								fail_sensor_lisr[fota_fail_num].idm = sensor_status[i].longidm;
								fota_fail_num++;
								fota_rc.fw_upd_result.rc = PART_OF_DEV_FAIL;
							}else{
								PRINTF("SENSOR_FOTA SUCCEED ID: 0x%x\r\n", sensor_status[i].send_message.sn_dest);
							}
						}
					}
					fota_rc.fw_upd_result.fl = fail_sensor_lisr;

					fota_rc.fw_upd_result.flnum	= fota_fail_num;
					if (need_fota == fota_fail_num) fota_rc.fw_upd_result.rc = ALL_DEV_FOTA_FAIL;
					if (fota_rc.fw_upd_result.rc != REQ_SUCC){
						re_num++;
						if (re_num >= 5){
							xQueueSend(uplink_q_handle, &fota_rc, portMAX_DELAY);
							img_blk_num = 0;
							re_num = 0;
						}else{
							goto  retry_send_image;
						}
					}else{
						xQueueSend(uplink_q_handle, &fota_rc, portMAX_DELAY);
						img_blk_num = 0;
						re_num = 0;
					}
					PRINTF("SENSOR_FOTA_RESULT_SEND_BACK %d\r\n", fota_rc.fw_upd_result.rc);
					if (fota_rc.fw_upd_result.flnum) {
						PRINTF("SENSOR_FOTA FIALED LIST \r\n");
						for (uint32_t i=1; i<=fota_rc.fw_upd_result.flnum; i++) {
							PRINTF("%d:0x%x\r\n", i, fota_rc.fw_upd_result.fl[i-1].id);
						}
					}
				}
				/* if has some more fota rule*/
				if (if_more) {
					if_more = 0;
					fota_rq_msg.cmd = FW_TIMING_CHECK_FOTA;
					PRINTF("FOTA GET MORE ");
					xQueueSend(fota_rq_handle, &fota_rq_msg, portMAX_DELAY);
					PRINTF("DONE\r\n");
				}
			}
			break;
		case FW_BRIDGE_IMG_BLK:
			PRINTF("RECVED FW_BRIDGE_IMG_BLK\r\n");

			if (bridge_img_blk_num==0){
				memcpy(sha1_temp, fota_data_read, CELL_SHA1_UNIT);
//				memset(fota_data_read, 0xFF, CELL_SHA1_UNIT);
//				//sha1_csum(fota_data_read+32U, FOTA_IMG_CELL_DOWN_LEN-32U, sha1_calc_out );
//				sha1_csum(fota_data_read, FOTA_IMG_CELL_DOWN_LEN, sha1_calc_out );
				sha1_csum(fota_data_read + 32U, FOTA_IMG_CELL_DOWN_LEN - 32U, sha1_calc_out);
				if (0 != memcmp(sha1_calc_out, sha1_temp ,(size_t)CELL_SHA1_UNIT)){
					//uplink_tq_msq.fw_upd_result.rc = DOWN_SHA1_ERR;
					uplink_tq_msq.fw_upd_result.rc 	= ALL_DEV_FOTA_FAIL;
					uplink_tq_msq.cmd = UPLINK_FW_UPD_RESULT;
					PRINTF("sha1 check error, bridge_img_blk_num=%d \r\n", bridge_img_blk_num);
					PRINTF("BRIDGE_FOTA_RESULT_SEND_BACK %d\r\n", uplink_tq_msq.fw_upd_result.rc);
					xQueueSend(uplink_q_handle, &uplink_tq_msq, portMAX_DELAY);
					break;
				}else{
					PRINTF("sha1 check pass , bridge_img_blk_num=%d \r\n", bridge_img_blk_num);
				}
			}else{
				norflash_read_data_ll (&flash_master_rtos_handle, FOTA_BRIDGE_ADDR_START+FOTA_HBB_SHA1_OFFSET_START+CELL_SHA1_UNIT*(bridge_img_blk_num-1), 20U, sha1_temp);
				sha1_csum(fota_data_read, FOTA_IMG_CELL_DOWN_LEN, sha1_calc_out );
				if (0 != memcmp(sha1_calc_out, sha1_temp ,(size_t)CELL_SHA1_UNIT)) {
					//uplink_tq_msq.fw_upd_result.rc = DOWN_SHA1_ERR;
					uplink_tq_msq.fw_upd_result.rc 	= ALL_DEV_FOTA_FAIL;
					uplink_tq_msq.cmd = UPLINK_FW_UPD_RESULT;
					PRINTF("sha1 check error, bridge_img_blk_num=%d \r\n", bridge_img_blk_num);
					PRINTF("BRIDGE_FOTA_RESULT_SEND_BACK %d\r\n", uplink_tq_msq.fw_upd_result.rc);
					xQueueSend(uplink_q_handle, &uplink_tq_msq, portMAX_DELAY);
					break;
				}
				else {
					PRINTF("sha1 check pass , bridge_img_blk_num=%d \r\n", bridge_img_blk_num);
				}
			}

			norflash_write_data_ll(&flash_master_rtos_handle, FOTA_BRIDGE_ADDR_START+FOTA_IMG_CELL_DOWN_LEN*bridge_img_blk_num, FOTA_IMG_CELL_DOWN_LEN, fota_data_read);
			bridge_img_blk_num++;
			if (bridge_img_blk_num >= fw_blks) {
				PRINTF("BRIDGE IMG RECVED TOTAL BLK %d\r\n", bridge_img_blk_num);
				bridge_img_blk_num = 0;
				PRINTF("SEND MSG TO BRIDGE...\r\n");
#ifdef PLC
				for(uint32_t i = 0; i< 1;i++)
				{
					for(uint32_t retry=0; retry<5; retry++)
					{
						plc_fw_need_update(bridge_list[i].id,device_type,plc_local_seq);
						uxBits = xEventGroupWaitBits(BridgeFwReqAckEventGroup,  	/* The event group handle. */
											B_BRIDGE_FW_REQ_ACK,		/* The bit pattern the event group is waiting for. */
											pdTRUE,         			/* BIT will be cleared automatically. */
											pdTRUE,         			/* Don't wait for both bits, either bit unblock task. */
											(TickType_t) 200U); 			/* Block indefinitely to wait for the condition to be met. */
						if((uxBits & B_BRIDGE_FW_REQ_ACK ) != 0)
						{
							PRINTF("BRIDGE_FW_ACKED\r\n");
							break;
						}
						else
						{
							//  xTicksToWait ticks passed
							PRINTF("BRIDGE_FW_ACKED timeout retry=%d\r\n", retry);
							continue;
						}

					}

				}
#endif

				break;
			}
			uplink_tq_msq.fw_down.img_blk_num = bridge_img_blk_num;
			xQueueSend(uplink_q_handle, &uplink_tq_msq, portMAX_DELAY);
			break;
//		case FW_BRIDGE_IMG_BLK_REQ:
//
//			break;
		case FW_HBB_IMG_BLK:
			PRINTF("RECVED FW_HBB_IMG_BLK\r\n");
			/* check extern charging */
			if (bat_volt_low_flag){
				FotaStatusFlag = Abort;
				uplink_tq_msq.fw_upd_result.rc = HBB_LOW_POWER;
				//uplink_tq_msq.fw_upd_result.rc 	= ALL_DEV_FOTA_FAIL;
				uplink_tq_msq.cmd = UPLINK_FW_UPD_RESULT;
				PRINTF("hbb low power, terminated update!\r\n");
				PRINTF("SENSOR_FOTA_RESULT_SEND_BACK %d\r\n", uplink_tq_msq.fw_upd_result.rc);
				xQueueSend(uplink_q_handle, &uplink_tq_msq, portMAX_DELAY);
				img_blk_num = 0;
				hbb_img_blk_num = 0;
				break;
			}
			if (FotaStatusFlag==Abort) {
				break;
			}
			if (hbb_img_blk_num==0){
				FOTA_HBB_4KHEAD		fota_hbb_4khead;
				memcpy(sha1_temp, fota_data_read, CELL_SHA1_UNIT);
				memcpy(&fota_hbb_4khead, fota_data_read, 4096);
//				memset(fota_data_read, 0xFF, CELL_SHA1_UNIT);
//				//sha1_csum(fota_data_read+32U, FOTA_IMG_CELL_DOWN_LEN-32U, sha1_calc_out );
//				sha1_csum(fota_data_read, FOTA_IMG_CELL_DOWN_LEN, sha1_calc_out );
				sha1_csum(fota_data_read + 32U, FOTA_IMG_CELL_DOWN_LEN - 32U, sha1_calc_out);
				if (0 != memcmp(sha1_calc_out, sha1_temp ,(size_t)CELL_SHA1_UNIT)){
					FotaStatusFlag = Bad;
					uplink_tq_msq.fw_upd_result.rc = DOWN_SHA1_ERR;
					//uplink_tq_msq.fw_upd_result.rc 	= ALL_DEV_FOTA_FAIL;
					uplink_tq_msq.cmd = UPLINK_FW_UPD_RESULT;
					PRINTF("sha1 check error, hbb_img_blk_num=%d \r\n", hbb_img_blk_num);
					PRINTF("HBB_FOTA_RESULT_SEND_BACK %d\r\n", uplink_tq_msq.fw_upd_result.rc);
					xQueueSend(uplink_q_handle, &uplink_tq_msq, portMAX_DELAY);
					hbb_img_blk_num = 0;
					/* if has some more fota rule*/
					if (if_more) {
						if_more = 0;
						fota_rq_msg.cmd = FW_TIMING_CHECK_FOTA;
						xQueueSend(fota_rq_handle, &fota_rq_msg, portMAX_DELAY);
					}
					break;
				}else{
					PRINTF("sha1 check pass , hbb_img_blk_num=%d \r\n", hbb_img_blk_num);
				}
				/* block0 check version */
				if (fota_hbb_4khead.sw_ver <= ver_info.swver) {
					FotaStatusFlag = Bad;
					uplink_tq_msq.cmd				= UPLINK_FW_UPD_RESULT;
					uplink_tq_msq.fw_upd_result.rid = rule_id;
					uplink_tq_msq.fw_upd_result.rc 	= DOWN_HBB_SWVER_ERR;
					PRINTF("swver check error\r\n");
					PRINTF("HBB_FOTA_RESULT_SEND_BACK %d\r\n", uplink_tq_msq.fw_upd_result.rc);
					xQueueSend(uplink_q_handle, &uplink_tq_msq, portMAX_DELAY);
					hbb_img_blk_num = 0;
					/* if has some more fota rule*/
					if (if_more) {
						if_more = 0;
						fota_rq_msg.cmd = FW_TIMING_CHECK_FOTA;
						xQueueSend(fota_rq_handle, &fota_rq_msg, portMAX_DELAY);
					}
					break;
				}
				if (fota_hbb_4khead.hw_ver != ver_info.hwver) {
					FotaStatusFlag = Bad;
					uplink_tq_msq.cmd				= UPLINK_FW_UPD_RESULT;
					uplink_tq_msq.fw_upd_result.rid = rule_id;
					uplink_tq_msq.fw_upd_result.rc 	= DOWN_HBB_HWVER_ERR;
					PRINTF("hwver check error\r\n");
					PRINTF("HBB_FOTA_RESULT_SEND_BACK %d\r\n", uplink_tq_msq.fw_upd_result.rc);
					xQueueSend(uplink_q_handle, &uplink_tq_msq, portMAX_DELAY);
					hbb_img_blk_num = 0;
					/* if has some more fota rule*/
					if (if_more) {
						if_more = 0;
						fota_rq_msg.cmd = FW_TIMING_CHECK_FOTA;
						xQueueSend(fota_rq_handle, &fota_rq_msg, portMAX_DELAY);
					}
					break;
				}
				if (fota_hbb_4khead.dev_type != ver_info.dt) {
					FotaStatusFlag = Bad;
					uplink_tq_msq.cmd				= UPLINK_FW_UPD_RESULT;
					uplink_tq_msq.fw_upd_result.rid = rule_id;
					uplink_tq_msq.fw_upd_result.rc 	= DOWN_HBB_DEVTYPE_ERR;
					PRINTF("device type check error\r\n");
					PRINTF("HBB_FOTA_RESULT_SEND_BACK %d\r\n", uplink_tq_msq.fw_upd_result.rc);
					xQueueSend(uplink_q_handle, &uplink_tq_msq, portMAX_DELAY);
					hbb_img_blk_num = 0;
					/* if has some more fota rule*/
					if (if_more) {
						if_more = 0;
						fota_rq_msg.cmd = FW_TIMING_CHECK_FOTA;
						xQueueSend(fota_rq_handle, &fota_rq_msg, portMAX_DELAY);
					}
					break;
				}
			}else{
				norflash_read_data_ll (&flash_master_rtos_handle, fota_hbb_addr_start+FOTA_HBB_SHA1_OFFSET_START+CELL_SHA1_UNIT*(hbb_img_blk_num-1), 20U, sha1_temp);
				sha1_csum(fota_data_read, FOTA_IMG_CELL_DOWN_LEN, sha1_calc_out );
				if (0 != memcmp(sha1_calc_out, sha1_temp ,(size_t)CELL_SHA1_UNIT)){
					FotaStatusFlag = Bad;
					uplink_tq_msq.fw_upd_result.rc = DOWN_SHA1_ERR;
					//uplink_tq_msq.fw_upd_result.rc 	= ALL_DEV_FOTA_FAIL;
					uplink_tq_msq.cmd = UPLINK_FW_UPD_RESULT;
					PRINTF("sha1 check error, hbb_img_blk_num=%d \r\n",hbb_img_blk_num);
					PRINTF("HBB_FOTA_RESULT_SEND_BACK %d\r\n", uplink_tq_msq.fw_upd_result.rc);
					xQueueSend(uplink_q_handle, &uplink_tq_msq, portMAX_DELAY);
					hbb_img_blk_num = 0;
					/* if has some more fota rule*/
					if (if_more) {
						if_more = 0;
						fota_rq_msg.cmd = FW_TIMING_CHECK_FOTA;
						xQueueSend(fota_rq_handle, &fota_rq_msg, portMAX_DELAY);
					}
					break;
				}
			}

			norflash_write_data_ll(&flash_master_rtos_handle, fota_hbb_addr_start+FOTA_IMG_CELL_DOWN_LEN*hbb_img_blk_num, FOTA_IMG_CELL_DOWN_LEN, fota_data_read);

			hbb_img_blk_num++;
			if (hbb_img_blk_num >= fw_blks) {
				PRINTF("HBB IMG RECVED TOTAL BLK %d\r\n", hbb_img_blk_num);
				//	uplink_tq_msq.cmd = UPLINK_FW_UPD_REQ_EMER;
				//	uplink_tq_msq.fw_upd_result.rid = ;
				//	uplink_tq_msq.fw_upd_result.rc 	= ;
				//	xQueueSend(uplink_q_handle, &uplink_tq_msq, portMAX_DELAY);
				hbb_img_blk_num = 0;
				taskENTER_CRITICAL();
				FLASH_RTOS_Erase_Sector(&pflash_handle, FOTA_PRAM_ADDR, 1);
				FLASH_RTOS_Program(&pflash_handle, FOTA_PRAM_ADDR, (uint32_t *)&hbb_info.config, (sizeof(hbb_config)|0x07)+1);
				FLASH_RTOS_Program(&pflash_handle, FOTA_PRAM_ADDR +(sizeof(hbb_config)|0x07)+1, &rule_id, FSL_FEATURE_FLASH_PFLASH_BLOCK_WRITE_UNIT_SIZE);
				FLASH_RTOS_Program(&pflash_handle, HBB_INFO_CONFIG + 2048, (uint32_t *)&ppp_gprs_apn_pap_info, ((sizeof(ppp_gprs_apn_pap_info) + 7) & ~7));
				taskEXIT_CRITICAL();
				vTaskDelay(50);
				FotaStatusFlag = Done;
				SysRestart();

				break;
			}
			uplink_tq_msq.fw_down.img_blk_num = hbb_img_blk_num;
			xQueueSend(uplink_q_handle, &uplink_tq_msq, portMAX_DELAY);

			break;

		default :

			break;
		}
	}
}
#endif

#include "A7139.h"
uint8_t				fota_blk_temp[4096]={0};
extern uint8_t rf_send_data_block(uint8_t* buf);
uint32_t rf_send_image_norflash(uint32_t device_type, uint32_t sensor_id)
{
	uint32_t 			length;
    uint8_t				sha1_temp[20];
    uint8_t				sha1_calc_out[20];
	uint16_t 			crc;
	SENSOR_MSG_fota_update_header	sendor_msg_header;
	FOTA_SENSOR_4KHEAD	sensor_4khead;
	RF_MSG 				rf_msg_test;

	PRINTF("fota sensor id 0x%x \r\n", sensor_id);

	norflash_read_data_ll(&flash_master_rtos_handle, FOTA_SENSOR_IMG_ADDR_START, sizeof(FOTA_SENSOR_4KHEAD), (uint8_t *)&sensor_4khead);
	if (sensor_4khead.dev_type != device_type) return DeviceTypeErr;

	rf_msg_test.header	= 0x16;
	rf_msg_test.sn_dest = sensor_id;//0x300203a1;//0x0aaaaaaa;//0x50020023, 0x31000004
	rf_msg_test.sn_mitt = hbb_info.config.id;
	rf_msg_test.fw_major = 0;
	rf_msg_test.fw_minor = 0;

	rf_msg_test.cmd = RF_CMD_UPDATE_IMAGE;
	rf_msg_test.sequence = local_seq++;
	rf_send_then_receive(&rf_msg_test);

	vTaskDelay(100);

	/* image 0 */
	EInt_GIO2_Disable();

	sendor_msg_header.dt 	= sensor_4khead.dev_type ;
	sendor_msg_header.hwver = sensor_4khead.hw_ver;
	sendor_msg_header.swver = sensor_4khead.sw_ver;
	sendor_msg_header.addr	= sensor_4khead.sub_node[0].addr;
	sendor_msg_header.length= sensor_4khead.sub_node[0].length;

	crc = Crc16_Calc((uint8_t*)&sendor_msg_header.dt, 20);

	A7139_SetPackLen(22);  // work around for not long enough WTR
	A7139_StrobeCmd(CMD_TFR);
	A7139_WriteFIFO((uint8_t*)&sendor_msg_header.dt, 20);
	A7139_WriteFIFO((uint8_t*)&crc, 2);
	A7139_SetGIO2_Func(e_GIOS_WTR);
	A7139_StrobeCmd(CMD_TX);
	lp_sleep_for_ms(100);
	PRINTF("send header done\r\n");

	length = (sensor_4khead.sub_node[0].length+0x3FF)>>10;

	for(int i=0;i<length;i++)
	{
		if (i%4 == 0){
			norflash_read_data_ll(&flash_master_rtos_handle, FOTA_SENSOR_IMG_ADDR_START+FOTA_IMG_CELL_DOWN_LEN*(i/4+1), FOTA_IMG_CELL_DOWN_LEN, fota_data_read);

    		sha1_csum(fota_data_read, CELL_SHA1_UNIT, sha1_calc_out );
    		if (!memcmp(sha1_calc_out, sha1_temp ,(size_t)CELL_SHA1_UNIT)){
    			PRINTF("sha1 check error\r\n");
    			vTaskDelay(3000);
    		}

			AES_Image_decrypt(hbb_info.config.aeskey, hbb_info.config.aesivv, fota_data_read, fota_blk_temp, 4096);
//			for(uint32_t j=0;j<4096;j++){
//				printf("%02x", fota_blk_temp[j]);
//			}
		}
		rf_send_data_block(fota_blk_temp+1024*(i%4));
		lp_sleep_for_ms(100);
	}
	/* image 0 rf send done */
	A7139_SetPackLen(PACKET_CONTENT_LENGTH);
	rf_receive();
    EInt_GIO2_Enable();
    vTaskDelay(1000);


	rf_msg_test.header	= 0x16;
	rf_msg_test.sn_dest = sensor_id;//0x300203a1;//0x0aaaaaaa;//0x50020023, 0x31000004
	rf_msg_test.sn_mitt = hbb_info.config.id;
	rf_msg_test.fw_major = 0;
	rf_msg_test.fw_minor = 0;


	rf_msg_test.cmd = RF_CMD_UPDATE_IMAGE;
	rf_msg_test.sequence = local_seq++;
	rf_send_then_receive(&rf_msg_test);
	vTaskDelay(100);

	/* image 1 */
	EInt_GIO2_Disable();
	sendor_msg_header.addr	= sensor_4khead.sub_node[1].addr;
	sendor_msg_header.length= sensor_4khead.sub_node[1].length;

	crc = Crc16_Calc((uint8_t*)&sendor_msg_header.dt, 20);

	A7139_SetPackLen(22);  // work around for not long enough WTR
	A7139_StrobeCmd(CMD_TFR);
	A7139_WriteFIFO((uint8_t *)&sendor_msg_header.dt, 20);
	A7139_WriteFIFO((uint8_t *)&crc, 2);
	A7139_SetGIO2_Func(e_GIOS_WTR);
	A7139_StrobeCmd(CMD_TX);
	lp_sleep_for_ms(100);
	PRINTF("send header done\r\n");

	length = (sensor_4khead.sub_node[1].length+0x3FF)>>10;

	for(int i=0;i<length;i++)
	{
		if (i%4 == 0){
			norflash_read_data_ll(&flash_master_rtos_handle, FOTA_SENSOR_IMG_ADDR_START+FOTA_SENSOR_IMG_CELL_LEN+FOTA_IMG_CELL_DOWN_LEN*(i/4+1), FOTA_IMG_CELL_DOWN_LEN, fota_data_read);
    		sha1_csum(fota_data_read, CELL_SHA1_UNIT, sha1_calc_out );
    		if (!memcmp(sha1_calc_out, sha1_temp ,(size_t)CELL_SHA1_UNIT)){
    			PRINTF("sha1 check error\r\n");
    		}
			AES_Image_decrypt(hbb_info.config.aeskey, hbb_info.config.aesivv, fota_data_read, fota_blk_temp, 4096);
//			for(uint32_t j=0;j<4096;j++){
//				printf("%02x", fota_blk_temp[j]);
//			}
		}
		rf_send_data_block(fota_blk_temp+1024*(i%4));
		lp_sleep_for_ms(100);
	}
	A7139_SetPackLen(PACKET_CONTENT_LENGTH);
	rf_receive();
    EInt_GIO2_Enable();
	lp_sleep_for_ms(1000);

	return 0;
}
