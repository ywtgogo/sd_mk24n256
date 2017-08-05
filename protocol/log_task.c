/* FreeRTOS kernel includes. */
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"

/* Freescale includes. */
#include "fsl_debug_console.h"

/* User include */
#include "log_task.h"
#include "spiflash_config.h"
#include "aes256.h"
#include "hb_protocol.h"
#include "message.h"
#include "rtc_fun.h"
#include "fsl_rtc.h"
#include "fsl_pflash_freertos.h"
#include <stdarg.h>
#include <stdio.h>
#include "mqtt_task.h"

SECTOR_INFO_STRUCT	sector_info;
LOG_INFO_STRUCT sys_log_info;
uint8_t dirty_mark = 0xA5;

flash_rtos_handle_t fd;
HBB_STATUS_STORAGE_BLOCK hbb_info_temp;
LOG_UPLOADING_STRUCT uploading_log;
char log_uplink_buf[4096];

int get_sector_info(int sector, uint8_t *info)
{
	uint32_t	addr;

	if ((sector > DEFER_UPLINK_SECTOR_COUNT) || (sector < 0))
		return -1;

	addr = (DEFER_UPLINK_ADDR_START+sector*FLASH_SECTORSIZE) + INFO_OFFSET;
	norflash_read_data_ll (&flash_master_rtos_handle, addr, DEFER_UPLINK_MSG_UNIT, info);

	return 0;
}

void erase_defer_sector(int sector)
{
	norflash_sector_erase_ll(&flash_master_rtos_handle, DEFER_UPLINK_ADDR_START+sector*FLASH_SECTORSIZE);
}

void erase_log_sector(int sector)
{
	PRINTF("%s %d\r\n", __func__, sector);
	norflash_sector_erase_ll(&flash_master_rtos_handle, SYS_LOG_ADDR_START+sector*FLASH_SECTORSIZE);
}

int get_sector_w_count(const uint8_t *line)
{
	int count = 0;
	int j;

	for (j = 0; j < SECTOR_MSG_MAX_COUNT; j++) {
		if (line[j] == dirty_mark)
			count++;
		else if(line[j] == 0xFF)
			continue;
		else
		{
			PRINTF("%s sector broken\r\n", __func__);
			return -1;
		}
	}

	return count;
}

int get_sector_r_count(const uint8_t *line)
{
	int count = 0;
	int j;

	for (j = 0; j < SECTOR_MSG_MAX_COUNT; j++) {
		if (line[64 + j] == dirty_mark)
			count++;
		else if(line[64 + j] == 0xFF)
			continue;
		else
		{
			PRINTF("%s sector broken\r\n", __func__);
			return -1;
		}
	}

	return count;
}

void print_infoline(const uint8_t *infoline)
{
	uint32_t *tmp = (uint32_t *)infoline;

	PRINTF("info line\r\n");
	for (int i = 0; i < 64/4; i++)
		PRINTF("%x ", tmp[i]);
	PRINTF("\r\n++++++++++++++\r\n");

	for (int i = 64/4; i < 128/4; i++)
		PRINTF("%x ", tmp[i]);
	PRINTF("\r\n++++++++++++++\r\n");
}

void scan_defer_area(SECTOR_INFO_STRUCT	*info)
{
	int i;
	int found;
	int wcount, rcount;
	uint8_t log_line[DEFER_UPLINK_MSG_UNIT];

	info->offset_sector_num_r = 0;
	info->offset_sector_num_w = 0;
	info->info_num_r = 0;
	info->info_num_w = 0;
	found = 0;
	/* check log offset */
	for (i = 0; i < DEFER_UPLINK_SECTOR_COUNT; i++) {
		get_sector_info(i, log_line);
		print_infoline(log_line);
		wcount = get_sector_w_count(log_line);
		rcount = get_sector_r_count(log_line);

		PRINTF("line w %d r %d\r\n", wcount, rcount);
		if ((wcount == 0) && (rcount == 0))
			break;

		if (wcount < rcount)
			PRINTF("sector corrupt %d\r\n", i);

		if (wcount < 31) {
			info->info_num_w = wcount;
			info->offset_sector_num_w = i;
		}

		if (rcount < 31) {
			if (found == 0) {
				found = 1;
				info->info_num_r = rcount;
				info->offset_sector_num_r = i;
			} else if (found == 1) {
				if (wcount == 31) {
					found = 2;
					info->info_num_r = rcount;
					info->offset_sector_num_r = i;
				}
			}
		}
	}
}

uint64_t get_log_timestamp(int offset)
{
	uint32_t	addr;
	uint64_t ts;

	if (offset == SYS_LOG_ENTRY_COUNT)
		offset = 0;

	if ((offset > SYS_LOG_ENTRY_COUNT) || (offset < 0))
		return 0;

	addr = SYS_LOG_ADDR_START + SYS_LOG_UNIT * offset;
	norflash_read_data_ll (&flash_master_rtos_handle, addr, 8, (uint8_t *)&ts);

	return ts;
}

void scan_log_area(LOG_INFO_STRUCT *info)
{
	int head_log_offset = 0, latest_log_offset = 0;
	int offset = 0;
	uint64_t current_ts;
	uint64_t logts;
	int64_t oldestts, latestts, tmpts;
	rtc_datetime_t rtcdate;

	//norflash_chip_erase_ll(&flash_master_rtos_handle);
	get_current_systime(&current_ts);

	oldestts = get_log_timestamp(0);
	latestts = 0;

	if ((oldestts & 0xFFFFFFFF) == 0xFFFFFFFF)
	{
		info->head_log_offset = 0;
		info->tail_log_offset = 0;
		PRINTF("head %d, tail %d\r\n", info->head_log_offset, info->tail_log_offset );
		//return;
	}

	/* check log offset */
	for (offset = 0; offset < SYS_LOG_ENTRY_COUNT; offset++)
	{
		logts = get_log_timestamp(offset);
		if ((logts & 0xFFFFFFFF) == 0xFFFFFFFF)
		{
			if (latest_log_offset == offset)
			{
				PRINTF("found tail %d\r\n", offset);
			}
			continue;
		}

//		RTC_ConvertSecondsToDatetime(logts / 1000, &rtcdate);
//		PRINTF("offset %d %d/%d/%d %d:%d:%d %d\r\n", offset, rtcdate.year, rtcdate.month, rtcdate.day,  rtcdate.hour, rtcdate.minute, rtcdate.second, (logts % 1000));

//		tmpts = current_ts - logts;
//
//		if (tmpts < 0) {
//			erase_log_sector(offset/SYS_LOG_OFFSET_COUNT);
//			PRINTF("sys log corrupt\r\n");
//		}

		if (logts < oldestts)
		{
			oldestts = logts;
			head_log_offset = offset;
		}

		if (logts > latestts)
		{
			latestts = logts;
			latest_log_offset = offset + 1;
		}
	}

	if (oldestts > latestts)
	{
		PRINTF("sys log corrupt oldestts > latestts\r\n");
	}


	info->head_log_offset = head_log_offset;
	info->tail_log_offset = latest_log_offset;
	if (sys_log_info.tail_log_offset >= SYS_LOG_ENTRY_COUNT)
	{
		sys_log_info.tail_log_offset = 0;
	}

	PRINTF("head %d, tail %d\r\n", info->head_log_offset, info->tail_log_offset);
}

void log_task(void *pvParameters)
{

	LOG_RQ_MSG 	rq_msg;
	uint32_t 	encrypt_len, decrypt_len;
	//uint8_t		data_temp[LOG_MSG_MAX_LEN];
	uint8_t		data_temp[128];
	uint8_t		info_line[128];
	uint8_t		data_encrypt[128];
	uint32_t	msg_addr;
	uint32_t	info_addr;
	uint32_t    *tmp;
	uint8_t     *info_byte_r;
	uint8_t     *info_byte_w;
	UPLINK_Q_MSG *uplink_msg;
	PLC_MSG		*plc_msg;
	scan_log_area(&sys_log_info);
	scan_defer_area(&sector_info);

    while(1)
    {

    	if (log_rq_handle != NULL)
    	xQueueReceive(log_rq_handle, &rq_msg, portMAX_DELAY);

    	switch(rq_msg.type)
    	{
			case	SYS_LOG:
			{
				uint64_t *tempts;
				uint64_t tscheck;
				SYS_LOG_STRUCT *templog;
				int i;

				memset(data_temp, 0, SYS_LOG_UNIT);
				memset(data_encrypt, 0, SYS_LOG_UNIT);
				encrypt_len = AES_Image_encrypt(hbb_info.config.aeskey, hbb_info.config.aesivv, (unsigned char *)&rq_msg.sys_log.sys_log_body, &data_temp[16], LOG_MSG_MAX_LEN - 16);
				tempts = (uint64_t *)data_temp;
				*tempts = rq_msg.sys_log.timestamp;

//				tmp = data_temp;
//				PRINTF("\r\n++++++++++++++ encrypt_len %d\r\n", encrypt_len);
//				for(i = 0; i < 128/4; i++)
//					PRINTF("%X ", tmp[i]);

				tscheck = get_log_timestamp(sys_log_info.tail_log_offset);

				if ((tscheck & 0xFFFFFFFF) != 0xFFFFFFFF)
				{
					PRINTF("data corrupt\r\n");
					erase_log_sector(sys_log_info.tail_log_offset/SYS_LOG_OFFSET_COUNT );
					scan_log_area(&sys_log_info);
				}

				i = (sys_log_info.tail_log_offset - 1 + SYS_LOG_ENTRY_COUNT) % SYS_LOG_ENTRY_COUNT;
				tscheck = get_log_timestamp(i);

				if ((tscheck & 0xFFFFFFFF) != 0xFFFFFFFF)
				{
					if (tscheck > rq_msg.sys_log.timestamp)
					{
						PRINTF("data corrupt, last timestamp > latest timestamp\r\n");
	//					erase_log_sector(sys_log_info.tail_log_offset/SYS_LOG_OFFSET_COUNT );
						scan_log_area(&sys_log_info);
						break;
					}
				}

				msg_addr = SYS_LOG_ADDR_START + sys_log_info.tail_log_offset*SYS_LOG_UNIT;
				while(true != norflash_write_data_ll(&flash_master_rtos_handle, msg_addr, SYS_LOG_UNIT, data_temp)) {
					vTaskDelay(50);
				}

				sys_log_info.tail_log_offset++;
				if (sys_log_info.tail_log_offset >= SYS_LOG_ENTRY_COUNT)
				{
					sys_log_info.tail_log_offset = 0;
				}

				break;
			}
			case	DUMP_LOG:
			{
				uint64_t start, end;
				uint32_t second;
				SYS_LOG_STRUCT *templog;
				rtc_datetime_t rtcdate;
				int count  =  sys_log_info.tail_log_offset - sys_log_info.head_log_offset;

				if (count < 0)
					count += SYS_LOG_ENTRY_COUNT;

				start = rq_msg.sys_log.timestamp;
				end = rq_msg.sys_log.timestamp_end;
				if (end == 0)
					get_current_systime(&end);
				memset(data_temp, 0, SYS_LOG_UNIT);
				memset(data_encrypt, 0, SYS_LOG_UNIT);
				msg_addr = (SYS_LOG_ADDR_START + sys_log_info.head_log_offset * SYS_LOG_UNIT);
				PRINTF("log count %d \r\n", count);

				while(count--)
				{
					norflash_read_data_ll (&flash_master_rtos_handle, msg_addr, SYS_LOG_UNIT, (uint8_t *)data_encrypt);
					decrypt_len = AES_Image_decrypt(hbb_info.config.aeskey, hbb_info.config.aesivv, data_encrypt + SYS_LOG_BODY_OFF, data_temp + SYS_LOG_BODY_OFF,  SYS_LOG_UNIT - SYS_LOG_BODY_OFF);

					templog = (SYS_LOG_STRUCT *)data_encrypt;
					if ((templog->timestamp >= start) && ((templog->timestamp <= end)))
					{
						second = templog->timestamp / 1000;
						RTC_ConvertSecondsToDatetime(second, &rtcdate);
						PRINTF("msg_addr %x ", msg_addr);
						PRINTF("%d/%d/%d %d:%d:%d %s\r\n", rtcdate.year, rtcdate.month, rtcdate.day,  rtcdate.hour, rtcdate.minute, rtcdate.second, data_temp + 16);
					}
					msg_addr += SYS_LOG_UNIT;
					if (msg_addr >= SYS_LOG_ADDR_END)
						msg_addr = SYS_LOG_ADDR_START;
				}
				break;
			}
			case	UPLINK_LOG:
			{
				uint64_t start, end;
				uint32_t second;
				uint32_t msecond;
				SYS_LOG_STRUCT *templog;
				rtc_datetime_t rtcdate;
				int count  =  sys_log_info.tail_log_offset - sys_log_info.head_log_offset;

				if (count < 0)
					count += SYS_LOG_ENTRY_COUNT;

				start = uploading_log.st;
				end = uploading_log.et;
				if (end == 0)
					get_current_systime(&end);
				memset(log_uplink_buf, 0, sizeof(log_uplink_buf));
				memset(data_temp, 0, SYS_LOG_UNIT);
				memset(data_encrypt, 0, SYS_LOG_UNIT);
				msg_addr = (SYS_LOG_ADDR_START + sys_log_info.head_log_offset * SYS_LOG_UNIT);

				RTC_ConvertSecondsToDatetime(start/1000, &rtcdate);
				PRINTF("start: %d/%d/%d %d:%d:%d\r\n", rtcdate.year, rtcdate.month, rtcdate.day,  rtcdate.hour, rtcdate.minute, rtcdate.second);
				RTC_ConvertSecondsToDatetime(end/1000, &rtcdate);
				PRINTF("end: %d/%d/%d %d:%d:%d\r\n", rtcdate.year, rtcdate.month, rtcdate.day,  rtcdate.hour, rtcdate.minute, rtcdate.second);

				while(count--)
				{
					if (strlen(log_uplink_buf) > 2048 )
						break;
					norflash_read_data_ll (&flash_master_rtos_handle, msg_addr, SYS_LOG_UNIT, (uint8_t *)data_encrypt);
					decrypt_len = AES_Image_decrypt(hbb_info.config.aeskey, hbb_info.config.aesivv, data_encrypt + SYS_LOG_BODY_OFF, data_temp + SYS_LOG_BODY_OFF,  SYS_LOG_UNIT - SYS_LOG_BODY_OFF);

					templog = (SYS_LOG_STRUCT *)data_encrypt;
					if ((templog->timestamp >= start) && ((templog->timestamp <= end)))
					{
						second = templog->timestamp / 1000;
						msecond = templog->timestamp % 1000;
						sprintf(log_uplink_buf + strlen(log_uplink_buf), "%u%03u\1%s\r\n", second, msecond, data_temp + SYS_LOG_BODY_OFF);
						RTC_ConvertSecondsToDatetime(second, &rtcdate);
						PRINTF("%d/%d/%d %d:%d:%d %s\r\n", rtcdate.year, rtcdate.month, rtcdate.day,  rtcdate.hour, rtcdate.minute, rtcdate.second, data_temp + 16);
					}

					msg_addr += SYS_LOG_UNIT;
					if (msg_addr >= SYS_LOG_ADDR_END)
						msg_addr = SYS_LOG_ADDR_START;
				}
				uploading_log.log_ptr = log_uplink_buf;
				uploading_log.log_len = strlen(log_uplink_buf);
				uploading_log.batch_count = 1;
				uploading_log.batch_id = 1;
#ifdef MQTT
				submit_log();
#endif
				break;
			}
			case	DEFER_UPLINK:
			{
				/* encryped msg */
				encrypt_len = AES_Image_encrypt(hbb_info.config.aeskey, hbb_info.config.aesivv, (unsigned char *)&rq_msg.defer_msg, data_temp, LOG_MSG_MAX_LEN);

				msg_addr = (DEFER_UPLINK_ADDR_START+sector_info.offset_sector_num_w*FLASH_SECTORSIZE) + sector_info.info_num_w*DEFER_UPLINK_MSG_UNIT;
				while(true != norflash_write_data_ll(&flash_master_rtos_handle, msg_addr, DEFER_UPLINK_MSG_UNIT, data_temp)) {
					vTaskDelay(50);
				}
				info_addr = (DEFER_UPLINK_ADDR_START+sector_info.offset_sector_num_w*FLASH_SECTORSIZE) + INFO_OFFSET + sector_info.info_num_w;
				while(true != norflash_write_data_ll(&flash_master_rtos_handle, info_addr, 1U, &dirty_mark)) {
					vTaskDelay(50);
				}

				info_addr = (DEFER_UPLINK_ADDR_START+sector_info.offset_sector_num_w*FLASH_SECTORSIZE) + INFO_OFFSET;
				norflash_read_data_ll (&flash_master_rtos_handle, info_addr, DEFER_UPLINK_MSG_UNIT, (uint8_t *)info_line);

				print_infoline(info_line);

				sector_info.info_num_w++;
				if (sector_info.info_num_w >= 31) {
					int w_count;
					sector_info.info_num_w = 0;
					sector_info.offset_sector_num_w++;
					get_sector_info(sector_info.offset_sector_num_w, info_line);
					w_count = get_sector_w_count(info_line);
					if (w_count >= 31)
						erase_defer_sector(sector_info.offset_sector_num_w);
					if (w_count != 0)
						PRINTF("warning sector maybe broken\r\n");
				}

				if (sector_info.offset_sector_num_w >= DEFER_UPLINK_SECTOR_COUNT){
					sector_info.offset_sector_num_w = 0;
				}

				PRINTF("sector info %d %d\r\n", sector_info.offset_sector_num_w, sector_info.info_num_w);

				break;
			}
			case	NEW_LINK_ON:
			{
				/* goto log send func */
				PRINTF("sector info w %d %d\r\n", sector_info.offset_sector_num_w, sector_info.info_num_w);
				PRINTF("sector info r %d %d\r\n", sector_info.offset_sector_num_r, sector_info.info_num_r);
				info_addr = (DEFER_UPLINK_ADDR_START+sector_info.offset_sector_num_r*FLASH_SECTORSIZE) + INFO_OFFSET;

				norflash_read_data_ll (&flash_master_rtos_handle, info_addr, DEFER_UPLINK_MSG_UNIT, (uint8_t *)info_line);
				print_infoline(info_line);

				info_byte_r = info_line + 64 + sector_info.info_num_r;
				info_byte_w = info_line + sector_info.info_num_r;

				while((*info_byte_r == 0xFF) && (*info_byte_w == 0xA5))
				{
					msg_addr = (DEFER_UPLINK_ADDR_START+sector_info.offset_sector_num_r*FLASH_SECTORSIZE) + sector_info.info_num_r*DEFER_UPLINK_MSG_UNIT;
					norflash_read_data_ll (&flash_master_rtos_handle, msg_addr, DEFER_UPLINK_MSG_UNIT, (uint8_t *)data_encrypt);
					decrypt_len = AES_Image_decrypt(hbb_info.config.aeskey, hbb_info.config.aesivv, data_encrypt, data_temp, 128);

					uplink_msg = (UPLINK_Q_MSG *)data_temp;
					if (xQueueSend(uplink_q_handle, uplink_msg, 3000) != pdPASS)
					{
						PRINTF("uplink timeout, abort\r\n");
						break;
					}
					while(true != norflash_write_data_ll(&flash_master_rtos_handle, info_addr + 64 + sector_info.info_num_r, 1U, &dirty_mark)) {
						vTaskDelay(50);
					}
					info_byte_r++;
					info_byte_w++;
					sector_info.info_num_r++;
					vTaskDelay(1000);
					if (sector_info.info_num_r >= 31) {
						sector_info.info_num_r = 0;
						sector_info.offset_sector_num_r++;
						info_addr = (DEFER_UPLINK_ADDR_START+sector_info.offset_sector_num_r*FLASH_SECTORSIZE) + INFO_OFFSET;
						norflash_read_data_ll (&flash_master_rtos_handle, info_addr, DEFER_UPLINK_MSG_UNIT, (uint8_t *)info_line);
						info_byte_r = info_line + 64 + sector_info.info_num_r;
						info_byte_w = info_line + sector_info.info_num_r;

						print_infoline(info_line);
					}
				}
				PRINTF("uplink done\r\n");
				break;
			}
			case	PLC_DEFER_MSG:
			{
				/* encryped msg */
				encrypt_len = AES_Image_encrypt(hbb_info.config.aeskey, hbb_info.config.aesivv, (unsigned char *)&rq_msg.plc_defer_msg, data_temp, LOG_MSG_MAX_LEN);

				msg_addr = (DEFER_UPLINK_ADDR_START+sector_info.offset_sector_num_w*FLASH_SECTORSIZE) + sector_info.info_num_w*DEFER_UPLINK_MSG_UNIT;
				while(true != norflash_write_data_ll(&flash_master_rtos_handle, msg_addr, DEFER_UPLINK_MSG_UNIT, data_temp)) {
					vTaskDelay(50);
				}
				info_addr = (DEFER_UPLINK_ADDR_START+sector_info.offset_sector_num_w*FLASH_SECTORSIZE) + INFO_OFFSET + sector_info.info_num_w;
				while(true != norflash_write_data_ll(&flash_master_rtos_handle, info_addr, 1U, &dirty_mark)) {
					vTaskDelay(50);
				}

				info_addr = (DEFER_UPLINK_ADDR_START+sector_info.offset_sector_num_w*FLASH_SECTORSIZE) + INFO_OFFSET;
				norflash_read_data_ll (&flash_master_rtos_handle, info_addr, DEFER_UPLINK_MSG_UNIT, (uint8_t *)info_line);

				print_infoline(info_line);

				sector_info.info_num_w++;
				if (sector_info.info_num_w >= 31) {
					int w_count;
					sector_info.info_num_w = 0;
					sector_info.offset_sector_num_w++;
					get_sector_info(sector_info.offset_sector_num_w, info_line);
					w_count = get_sector_w_count(info_line);
					if (w_count >= 31)
						erase_defer_sector(sector_info.offset_sector_num_w);
					if (w_count != 0)
						PRINTF("warning sector maybe broken\r\n");
				}

				if (sector_info.offset_sector_num_w >= DEFER_UPLINK_SECTOR_COUNT){
					sector_info.offset_sector_num_w = 0;
				}

				PRINTF("sector info %d %d\r\n", sector_info.offset_sector_num_w, sector_info.info_num_w);

				break;
			}
			case	PLC_MSG_ON:
			{
				/* goto log send func */
				PRINTF("sector info w %d %d\r\n", sector_info.offset_sector_num_w, sector_info.info_num_w);
				PRINTF("sector info r %d %d\r\n", sector_info.offset_sector_num_r, sector_info.info_num_r);
				info_addr = (DEFER_UPLINK_ADDR_START+sector_info.offset_sector_num_r*FLASH_SECTORSIZE) + INFO_OFFSET;

				norflash_read_data_ll (&flash_master_rtos_handle, info_addr, DEFER_UPLINK_MSG_UNIT, (uint8_t *)info_line);
				print_infoline(info_line);

				info_byte_r = info_line + 64 + sector_info.info_num_r;
				info_byte_w = info_line + sector_info.info_num_r;

				while((*info_byte_r == 0xFF) && (*info_byte_w == 0xA5))
				{
					msg_addr = (DEFER_UPLINK_ADDR_START+sector_info.offset_sector_num_r*FLASH_SECTORSIZE) + sector_info.info_num_r*DEFER_UPLINK_MSG_UNIT;
					norflash_read_data_ll (&flash_master_rtos_handle, msg_addr, DEFER_UPLINK_MSG_UNIT, (uint8_t *)data_encrypt);
					decrypt_len = AES_Image_decrypt(hbb_info.config.aeskey, hbb_info.config.aesivv, data_encrypt, data_temp, 128);

					plc_msg = (PLC_MSG *)data_temp;
					xQueueSend(uplink_q_handle, plc_msg, portMAX_DELAY);
					while(true != norflash_write_data_ll(&flash_master_rtos_handle, info_addr + 64 + sector_info.info_num_r, 1U, &dirty_mark)) {
						vTaskDelay(50);
					}
					info_byte_r++;
					info_byte_w++;
					sector_info.info_num_r++;
					vTaskDelay(1000);
					if (sector_info.info_num_r >= 31) {
						sector_info.info_num_r = 0;
						sector_info.offset_sector_num_r++;
						info_addr = (DEFER_UPLINK_ADDR_START+sector_info.offset_sector_num_r*FLASH_SECTORSIZE) + INFO_OFFSET;
						norflash_read_data_ll (&flash_master_rtos_handle, info_addr, DEFER_UPLINK_MSG_UNIT, (uint8_t *)info_line);
						info_byte_r = info_line + 64 + sector_info.info_num_r;
						info_byte_w = info_line + sector_info.info_num_r;

						print_infoline(info_line);
					}
				}
				PRINTF("plc on done\r\n");
				break;
			}
			default:
			{
				break;
			}
    	}
    }
}

int save_defer_log(	UPLINK_Q_MSG *uplink_msg)
{
	LOG_RQ_MSG 	rq_msg;

	memcpy(&rq_msg.defer_msg, uplink_msg, sizeof(UPLINK_Q_MSG));
	rq_msg.type = DEFER_UPLINK;

	xQueueSend(log_rq_handle, &rq_msg, 0);

	return 0;
}

int plc_save_defer_log(PLC_MSG *plc_msg)
{
	LOG_RQ_MSG 	rq_msg;

	memcpy(&rq_msg.defer_msg, plc_msg, sizeof(PLC_MSG));
	rq_msg.type = PLC_DEFER_MSG;

	xQueueSend(log_rq_handle, &rq_msg, portMAX_DELAY);

	return 0;
}

int defer_log_uplink()
{
	LOG_RQ_MSG 	rq_msg;

	rq_msg.type = NEW_LINK_ON;

	xQueueSend(log_rq_handle, &rq_msg, 0);

	return 0;
}

int plc_defer_log_on()
{
	LOG_RQ_MSG 	rq_msg;

	rq_msg.type = PLC_MSG_ON;

	xQueueSend(log_rq_handle, &rq_msg, portMAX_DELAY);

	return 0;
}

int print_log(char *fmt_ptr, ...)
{
	LOG_RQ_MSG 	log_msg;
    va_list ap;
    int result;
    char *end;
	rtc_datetime_t rtcdate;
	uint32_t sec;
	uint32_t msec;
	uint64_t logts;

    memset(&log_msg, 0, sizeof(log_msg));
	log_msg.type = SYS_LOG;

    va_start(ap, fmt_ptr);
    result = vsprintf(log_msg.sys_log.sys_log_body, fmt_ptr, ap);
    va_end(ap);
    end = strrchr(log_msg.sys_log.sys_log_body, '\r');

    if (end)
    {
    	end[0] = '\0';
       	end[1] = '\0';
    }

    get_current_systime(&log_msg.sys_log.timestamp);

    logts = log_msg.sys_log.timestamp;
    sec = logts/1000;
    msec = logts % 1000;

//	RTC_ConvertSecondsToDatetime(logts / 1000, &rtcdate);
//	PRINTF("\r\n%d/%d/%d %d:%d:%d - %d %d", rtcdate.year, rtcdate.month, rtcdate.day,  rtcdate.hour, rtcdate.minute, rtcdate.second, sec, msec);

    PRINTF("sys log: %s\r\n", log_msg.sys_log.sys_log_body);
    /* filter log timestamp old than 20161206 */
	if (log_msg.sys_log.timestamp < 1481012965000ULL)
	{
		PRINTF("invalid timestamp\r\n");
		return 0;
	}

	xQueueSend(log_rq_handle, &log_msg, portMAX_DELAY);

	return result;
}

int test_print_log(int i, char *fmt_ptr, ...)
{
	LOG_RQ_MSG 	log_msg;
    va_list ap;
    int result;
    char *end;

    memset(&log_msg, 0, sizeof(log_msg));
	log_msg.type = SYS_LOG;

    va_start(ap, fmt_ptr);
    result = vsprintf(log_msg.sys_log.sys_log_body, fmt_ptr, ap);
    va_end(ap);

    end = strrchr(log_msg.sys_log.sys_log_body, '\r');

    if (end)
    {
    	end[0] = '\0';
    	if (end[1] == '\n')
        	end[1] = '\0';
    }

    get_current_systime(&log_msg.sys_log.timestamp);
    log_msg.sys_log.timestamp -= i*1000;
    PRINTF("sys log: %s", log_msg.sys_log.sys_log_body);
    /* filter log timestamp old than 20161206 */
	if (log_msg.sys_log.timestamp < 1481012965000ULL)
	{
		PRINTF("invalid timestamp\r\n");
		return result;
	}

	xQueueSend(log_rq_handle, &log_msg, portMAX_DELAY);

	return result;
}

int dump_log(uint64_t start, uint64_t end)
{
	LOG_RQ_MSG 	log_msg;

    memset(&log_msg, 0, sizeof(log_msg));
	log_msg.type = DUMP_LOG;
	log_msg.sys_log.timestamp = start;
	log_msg.sys_log.timestamp_end = end;
	xQueueSend(log_rq_handle, &log_msg, portMAX_DELAY);

	return 0;
}

int uplink_log(void)
{
	LOG_RQ_MSG 	log_msg;

    memset(&log_msg, 0, sizeof(log_msg));
	log_msg.type = UPLINK_LOG;
	xQueueSend(log_rq_handle, &log_msg, 0);

	return 0;
}
