/*
 * hb_protocol.c
 *
 *  Created on: 2015/7/18
 *      Author: zhanghong
 *
 */

#include <string.h>
#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <stdint.h>
#include <fsl_common.h>
#include "A7139.h"
#include "SPI_A7139.h"
#include "fsl_debug_console.h"
#include "supervisor.h"
#include "log_task.h"
#include "aes256.h"
#include "crc.h"


#define BUILD_SENSOR_HUB 1


#include "message.h"

#include "hb_protocol.h"

extern AES_KEY enc_key;
#define CRC16_BITS 8
#define BATTERY_CHECK_COUNTER 24

uint32_t local_seq = 0;
#ifdef DEBUG
int enable_rf_resp = 1;
#endif

//skip 1st 4 meaningless bytes
RF_MSG recv_packet;
RF_MSG send_packet = { 0x00, 0x00, 0x00, 0x14};
SENSOR_STATUS_STORAGE_BLOCK sensor_status[MAX_SENSOR_COUNT];
#ifdef PLC
plc_bridge_node bridge_list[MAX_BRIDGE_COUNT];
#endif
uint16_t saved_lptmr_ms=0;
uint16_t saved_standby_ms = 0;
uint16_t check_batt_count=0;
uint16_t pir_int_count = 0;
uint8_t  pir_que[7] ={0};
uint8_t is_battery_too_low = 0;
uint16_t rf_rx_standby = 0;
uint16_t rf_rx_standby_time = 0;
uint8_t RX_TIMEOUT_FLAG = 0;
uint8_t RX_5MS_FLAG = 0;
uint8_t RX_WAKEUP_FLAG = 0;
uint8_t RX_FSYNC_FLAG = 0;
struct sensor_hw
{
	bool Peripheral_active;
	bool RF_active;
};
volatile struct sensor_hw HW_status;

void init_A7139()
{
    uint32_t ID;
    //SPI_A7139_INIT();
    A7139_StrobeCmd(0x55);
    A7139_Init(hbb_info.config.freq);
    A7139_SetCID(0x78877887);
    ID = A7139_GetCID();
    //PRINTF("\r\nA7139_ID: 0x%x \r\n", kxx_htonl(ID));
#ifdef CPU_MK24FN256VDC12
    if (0x78877887 == ID)
    {
    	post_status.a7139_check = true;
    }
#endif
    A7139_StrobeCmd(CMD_STBY);
    vTaskDelay(10);
	A7139_SetPackLen(PACKET_CONTENT_LENGTH);
    A7139_SetGIO2_Func(e_GIOS_FSYNC);
    A7139_StrobeCmd(CMD_RX);

}

bool is_seq_match()
{
	//I don't think we need check seq id.
//	return !memcmp(&recv_packet.sequence_id,&send_packet.sequence_id, ID_LENGTH);
	return 1;
}

void sleep_with_led_out()
{
//	Cpu_SetOperationMode(DOM_SLEEP, NULL,NULL);
}

void lp_sleep_for_ms(int ms)
{
	vTaskDelay(ms);
}

void random_delay()
{
	if(SENSOR_NUMBER_BROADCAST ==(recv_packet.sn_dest & SENSOR_NUMBER_MASK))
	{
		int del;
		del = rand() & 0x37F;
		lp_sleep_for_ms(del);
	}
	else
	{
		lp_sleep_for_ms(20);
	}
}

#if BUILD_MAGNETIC_SENSOR || BUILD_PIR_SENSOR
void lp_sleep_for_s(int s)
{
	//Divide count to s
	if(s>=1)
	{
		//start timer
		LPTMR_PDD_EnableDevice(LPTMR0_DEVICE,PDD_DISABLE);
		LPTMR_PDD_SetPrescaler(LPTMR0_DEVICE,LPTMR_PDD_PRESCALER_1024);
		LPTMR_PDD_WriteCompareReg(LPTMR0_DEVICE, s);
		LPTMR_PDD_ClearInterruptFlag(LPTMR0_DEVICE);
		LPTMR_PDD_EnableDevice(LPTMR0_DEVICE,PDD_ENABLE);
		Cpu_SetOperationMode(DOM_SLEEP, NULL,NULL);
	}
}
#endif


void rf_send_message(void* msg, uint32_t length)
{
	send_preamble_ms(430);
	A7139_StrobeCmd(CMD_STBY);
	lp_sleep_for_ms(2);
	A7139_StrobeCmd(CMD_TFR);
	A7139_WriteFIFO(msg, length);
	A7139_SetGIO2_Func(e_GIOS_WTR);
	A7139_SetPackLen(length);

	GIO2S = TXIN;
	GIO2_FLAG = 0;
	A7139_StrobeCmd(CMD_TX);
}

void rf_send_message_no_preamble(void* msg, uint32_t length)
{
	A7139_StrobeCmd(CMD_STBY);
	lp_sleep_for_ms(2);
	A7139_StrobeCmd(CMD_TFR);
	A7139_WriteFIFO(msg, length);
	A7139_SetGIO2_Func(e_GIOS_WTR);
	A7139_SetPackLen(length);
	GIO2S = TXIN;
	GIO2_FLAG = 0;
	A7139_StrobeCmd(CMD_TX);
}

void rf_receive()
{
	A7139_StrobeCmd(CMD_STBY);
	vTaskDelay(2);
	A7139_SetGIO2_Func(e_GIOS_FSYNC);
	A7139_StrobeCmd(CMD_RX);
}

void rf_send_then_receive(RF_MSG* rf_msg)
{
	xSemaphoreTake(rf_message_sem, portMAX_DELAY);

	if (GIO2S == TXIN)
	{
		PRINTF("Send RF Busy Now\r\n");
		return ;
	}else{
		GIO2S  = TXIN;
		GIO2_FLAG = 0;
	}
	RF_MSG rf_tmp_msg;
	memcpy(&rf_tmp_msg,rf_msg,sizeof(RF_MSG));
	rf_tmp_msg.sn_mitt = hbb_info.config.id;
	rf_tmp_msg.crc = Crc16_Calc((uint8_t*)&rf_tmp_msg.sn_dest, PACKET_CRC_LENGTH);

	if (rf_msg->header == 0x16)
	{
	AES_encrypt((uint8_t*)&rf_tmp_msg.sn_mitt, (uint8_t*)&rf_tmp_msg.sn_mitt, &enc_key);
	rf_tmp_msg.crc2 = Crc16_Calc((uint8_t*)&rf_tmp_msg.sn_dest, 20);
	}
	rf_send_message(&rf_tmp_msg.header, PACKET_CONTENT_LENGTH);

	while(GIO2S == TXIN)
	{
		vTaskDelay(3);
	}
	if(GIO2S == TXEND) {
		GIO2S = RX;
		GIO2_FLAG = 0;
		rf_receive();
	}
//	RX_FSYNC_FLAG = 0;
//	rf_receive();
//	saved_standby_ms = 0;
//	rf_rx_standby = 1;

	xSemaphoreGive(rf_message_sem);
}

void rf_send_then_receive_no_preamble(RF_MSG* rf_msg)
{
	xSemaphoreTake(rf_message_sem, portMAX_DELAY);

	if (GIO2S == TXIN)
	{
		PRINTF("SendNoPreamble RF Busy Now\r\n");
		return ;
	}else{
		GIO2S  = TXIN;
		GIO2_FLAG = 0;
	}

	RF_MSG rf_tmp_msg;
	memcpy(&rf_tmp_msg,rf_msg,sizeof(RF_MSG));
	rf_tmp_msg.sn_mitt = hbb_info.config.id;
	rf_tmp_msg.crc = Crc16_Calc((uint8_t*)&rf_tmp_msg.sn_dest, PACKET_CRC_LENGTH);

	if (rf_msg->header == 0x16)
	{
	AES_encrypt((uint8_t*)&rf_tmp_msg.sn_mitt, (uint8_t*)&rf_tmp_msg.sn_mitt, &enc_key);
	rf_tmp_msg.crc2 = Crc16_Calc((uint8_t*)&rf_tmp_msg.sn_dest, 20);
	}

	rf_send_message_no_preamble(&rf_tmp_msg.header, PACKET_CONTENT_LENGTH);

	while(GIO2S == TXIN)
	{
		vTaskDelay(3);
	}
	if(GIO2S == TXEND) {
		GIO2S = RX;
		GIO2_FLAG = 0;
		rf_receive();
	}
//	RX_FSYNC_FLAG = 0;
//	rf_receive();
	xSemaphoreGive(rf_message_sem);
}

uint8_t rf_send_data_block(uint8_t* buf)
{
	TickType_t	start_tick;
	A7139_SetPackLen(1024+8);
	A7139_SetGIO2_Func(e_GIOS_FPF);
	uint16_t crc[4] = {0, 0, 0, 0};
	for(int i=0;i<4;i++)
	{
		crc[i] = Crc16_Calc(&buf[i*256],256);
	}
	uint8_t* ptr = buf;
	A7139_StrobeCmd(CMD_TFR);
	A7139_WriteFIFO(ptr, 64);
	ptr+=64;
	A7139_StrobeCmd(CMD_TX);
	start_tick = xTaskGetTickCount();
	for(int i=0;i<20;i++)
	{
#ifndef CPU_MK24FN256VDC12
		while(!GPIO_ReadPinInput(GPIOB, 2));
#else
		while(!GPIO_ReadPinInput(A7139_GIO2_GPIO, A7139_GIO2_GPIO_PIN)){
			if ((xTaskGetTickCount() - start_tick) >= 500) {
				PRINTF("RF MSG BLOCK TIMEOUT0\r\n");
				break;
			}
		}
#endif
		A7139_WriteFIFO(ptr, 48);
		ptr+=48;
	}
#ifndef CPU_MK24FN256VDC12
	while(!GPIO_ReadPinInput(GPIOB, 2));
#else
	start_tick = xTaskGetTickCount();
	while(!GPIO_ReadPinInput(A7139_GIO2_GPIO, A7139_GIO2_GPIO_PIN)){
		if ((xTaskGetTickCount() - start_tick) >= 500) {
			PRINTF("RF MSG BLOCK TIMEOUT1\r\n");
			break;
		}
	}
#endif
	A7139_WriteFIFO((uint8_t*)crc, 8);
	PRINTF("send %d done\r\n", 1024);
	return 0;
}

#define RECV_BLOCK_TIMEOUT 100*400

int rf_recv_data_block(uint8_t* buf, uint16_t crc[])
{
	A7139_SetPackLen(1024+8);
	A7139_SetGIO2_Func(e_GIOS_FPF);
	uint8_t* ptr = buf;
	A7139_StrobeCmd(CMD_RFR);
	A7139_StrobeCmd(CMD_RX);
	for(int i=0;i<21;i++)
	{
		volatile uint32_t counter = 0;
//		while(!EInt_GIO2_GetVal(EInt_GIO2_DeviceData))
		{
			counter++;
			if(counter>=RECV_BLOCK_TIMEOUT)
			{
				return false;
			}
		}
		A7139_ReadFIFO(ptr, 48);
		ptr+=48;
	}
	A7139_SetGIO2_Func(e_GIOS_WTR);
	volatile uint32_t counter = 0;
	//while(EInt_GIO2_GetVal(EInt_GIO2_DeviceData))
	{
		counter++;
		if(counter>=RECV_BLOCK_TIMEOUT)
		{
			return false;
		}
	}
	A7139_ReadFIFO(ptr, 16);
	A7139_ReadFIFO((uint8_t*)crc, 8);
	for(int i=0;i<4;i++)
	{
		if(crc[i] != Crc16_Calc(&buf[i*256],256))
		{
			return false;
		}
	}
	DBG_MESSAGE("recv %d done", 1024);
	return true;
}
#if BUILD_SENSOR_HUB
//void rf_send_image()
//{
//	UPDATE_HEADER 		update_header;
//	update_header.addr		= 0x4400;
//	update_header.length 	= 0x323c;
//
//	uint16_t crc = crc_cal((uint8_t*)&update_header.addr, 8);
//
//	EInt_GIO2_Disable();
//	A7139_SetPackLen(10);  // work around for not long enough WTR
//	A7139_StrobeCmd(CMD_TFR);
//	A7139_WriteFIFO((uint8_t*)&update_header.addr, 8);
//	A7139_WriteFIFO((uint8_t*)&crc, 2);
//	A7139_SetGIO2_Func(e_GIOS_WTR);
//	A7139_StrobeCmd(CMD_TX);
//	lp_sleep_for_ms(100);
//
//	PRINTF("send header done\r\n");
//
//	uint32_t length = (update_header.length+0x3FF)>>10;
//	//uint32_t length = 13;
//	uint8_t* addr = (uint8_t*)0x32000;
//	for(int i=0;i<length;i++)
//	{
//		rf_send_data_block(addr);
//		addr += 1024;
//		lp_sleep_for_ms(100);
//	}
//
//	A7139_SetPackLen(21);
//	EInt_GIO2_Enable();
//	GIO2_FLAG  = 0;
//}

//#include "spiflash_config.h"
//void rf_send_image_norflash()
//{
//	uint32_t 			length;
//	uint32_t  			addr;
//
//	uint16_t 			crc;
//    uint8_t             fota_rf_cell[1024]={0};
//	UPDATE_HEADER 		update_header;
//
//
//	RF_MSG rf_msg_test;
//	rf_msg_test.header	= 0x14;
//	rf_msg_test.sn_dest = 0x31000004;//0x0aaaaaaa;//0x50020023, 0x31000004
//	rf_msg_test.sn_mitt = hbb_info.id;
//	rf_msg_test.fw_major = 2;
//	rf_msg_test.fw_minor = 0;
//
//    rf_msg_test.cmd = RF_CMD_GENERIC_REQUEST;
//	rf_msg_test.sequence = local_seq++;
//	rf_send_then_receive(&rf_msg_test);
//    vTaskDelay(500);
//
//	rf_msg_test.cmd = RF_CMD_UPDATE_IMAGE;
//	rf_msg_test.sequence = local_seq++;
//	rf_send_then_receive(&rf_msg_test);
//
//	vTaskDelay(100);
//	/* image 0 */
//	EInt_GIO2_Disable();
//	norflash_read_data_ll(&flash_master_rtos_handle, 0x0f0000+0x00, 4, (uint8_t*)&update_header.addr);
//	norflash_read_data_ll(&flash_master_rtos_handle, 0x0f0000+0x04, 4, (uint8_t*)&update_header.length);
//	crc = crc_cal((uint8_t*)&update_header.addr, 8);
//
//	A7139_SetPackLen(10);  // work around for not long enough WTR
//	A7139_StrobeCmd(CMD_TFR);
//	A7139_WriteFIFO((uint8_t*)&update_header.addr, 8);
//	A7139_WriteFIFO((uint8_t*)&crc, 2);
//	A7139_SetGIO2_Func(e_GIOS_WTR);
//	A7139_StrobeCmd(CMD_TX);
//	lp_sleep_for_ms(100);
//	PRINTF("send header done\r\n");
//
//	length = (update_header.length+0x3FF)>>10;
//	addr   = 0x0f0000+0x08;
//	for(int i=0;i<length;i++)
//	{
//        norflash_read_data_ll(&flash_master_rtos_handle, addr, 0x400, fota_rf_cell);
//		rf_send_data_block(fota_rf_cell);
//		addr += 1024;
//		lp_sleep_for_ms(100);
////		for(uint32_t j=0;j<1024;j++){
////			printf("%02x", fota_rf_cell[j]);
////		}
//	}
//	
//	/* image 0 rf send done */
//	A7139_SetPackLen(21);
//	rf_receive();
//    EInt_GIO2_Enable();
//	lp_sleep_for_ms(1000);
//
//	rf_msg_test.header	= 0x14;
//	rf_msg_test.sn_dest = 0x31000004;//0x0aaaaaaa;//0x50020023, 0x31000004
//	rf_msg_test.sn_mitt = hbb_info.id;
//	rf_msg_test.fw_major = 2;
//	rf_msg_test.fw_minor = 0;
//
//    rf_msg_test.cmd = RF_CMD_GENERIC_REQUEST;
//	rf_msg_test.sequence = local_seq++;
//	rf_send_then_receive(&rf_msg_test);
//    vTaskDelay(500);
//
//	rf_msg_test.cmd = RF_CMD_UPDATE_IMAGE;
//	rf_msg_test.sequence = local_seq++;
//	rf_send_then_receive(&rf_msg_test);
//
//	vTaskDelay(100);
//	/* image 1 */
//    EInt_GIO2_Disable();
//	norflash_read_data_ll(&flash_master_rtos_handle, 0x0f4000+0x00, 4, (uint8_t*)&update_header.addr);
//	norflash_read_data_ll(&flash_master_rtos_handle, 0x0f4000+0x04, 4, (uint8_t*)&update_header.length);
//	crc = crc_cal((uint8_t*)&update_header.addr, 8);
//
//	A7139_SetPackLen(10);  // work around for not long enough WTR
//	A7139_StrobeCmd(CMD_TFR);
//	A7139_WriteFIFO((uint8_t*)&update_header.addr, 8);
//	A7139_WriteFIFO((uint8_t*)&crc, 2);
//	A7139_SetGIO2_Func(e_GIOS_WTR);
//	A7139_StrobeCmd(CMD_TX);
//	lp_sleep_for_ms(100);
//	PRINTF("send header done\r\n");
//
//	length = (update_header.length+0x3FF)>>10;
//	addr   = 0x0f4000+0x08;
//	for(int i=0;i<length;i++)
//	{
//        norflash_read_data_ll(&flash_master_rtos_handle, addr, 0x400, fota_rf_cell);
//		rf_send_data_block(fota_rf_cell);
//		addr += 1024;
//		lp_sleep_for_ms(100);
////		for(uint32_t j=0;j<1024;j++){
////			printf("%02x", fota_rf_cell[j]);
////		}
//	}
//	/* image1 rf send done */
//	A7139_SetPackLen(21);
//	rf_receive();
//    EInt_GIO2_Enable();
//	lp_sleep_for_ms(1000);
//
//}

#else
void rf_recv_image()
{
	UART_SENSOR_MSG_fota_update_header header;
	uint16 crc;

	EInt_GIO1_Disable(EInt_GIO1_DeviceData);
	EInt_GIO2_Disable(EInt_GIO2_DeviceData);
	A7139_SetPackLen(10);
	A7139_SetGIO2_Func(e_GIOS_WTR);
	A7139_StrobeCmd(CMD_RX);
	volatile uint32 counter = 0;
	while(EInt_GIO2_GetVal(EInt_GIO2_DeviceData))
	{
		counter++;
		if(counter>=RECV_BLOCK_TIMEOUT)
		{
			goto IMAGE_CORRUPT;
		}
	}
	A7139_StrobeCmd(CMD_RFR);
	A7139_ReadFIFO((uint8*)&header.addr, 8);
	A7139_ReadFIFO((uint8*)&crc, 2);

	if(crc != Crc16_Calc((uint8*)&header.addr, 8))
	{
		DBG_MESSAGE("recv header fail");
		goto IMAGE_CORRUPT;
	}

	uint32 length = (header.length+0x3FF)>>10;
	if(length > 15)
	{
		goto IMAGE_CORRUPT;
	}

	uint32 addr = header.addr;
	if((SCB_VTOR == addr) ||
		((0x0800 != addr) && (0x4400 != addr)))
	{
		goto IMAGE_CORRUPT;
	}

	for(int i=0;i<length;i++)
	{
		uint16 crc[4];
		WTDG_clear();
		if(!rf_recv_data_block(uart_msg.arg.fota_update_data.data, crc) ||
			!flash_update_block(addr, crc))
		{
			DBG_MESSAGE("recv block fail");
			goto IMAGE_CORRUPT;
		}
		addr += 1024;
	}

	uint32* boot_entry = (uint32*)0x600;
	while(boot_entry[0] != 0xFFFFFFFF)
	{
		boot_entry++;
		if(boot_entry == (uint32*)0x7F4)
			goto IMAGE_CORRUPT;
	}

	Cpu_SetClockConfiguration(CPU_CLOCK_CONFIG_1);

	FLASH1_Init(NULL);
	FLASH1_Write(FLASH1_DeviceData, &header.addr, (uint32)boot_entry, sizeof(header.addr));
	extern int flash_done;
	flash_done = 0;
	while(!flash_done)
	    FLASH1_Main(FLASH1_DeviceData);

	Cpu_SetClockConfiguration(CPU_CLOCK_CONFIG_0);

	Cpu_SystemReset();

	return;

IMAGE_CORRUPT:
	A7139_SetPackLen(21);
	rf_wait();
}

typedef struct
{
	uint32_t id;
	uint32_t manufacture_date;
	uint32_t manufacture_time;
	uint32_t manufacturer;
	uint32_t base;
	uint32_t nominal_version;
	uint32_t svn_version;
	uint32_t build_date;
	uint16_t crc;
} sensor_info;

void rf_reply_sensor_info_then_wait()
{
	sensor_info info;
	info.id               = *(uint32_t*)0x0410;
	info.manufacture_date = *(uint32_t*)0x0414;
	info.manufacture_time = *(uint32_t*)0x0418;
	info.manufacturer     = *(uint32_t*)0x041c;
	info.base             = 0;
	info.nominal_version  = ((send_packet.fw_major<<16) | send_packet.fw_minor);
	info.svn_version      = SVN_VERSION;
	info.build_date       = BUILD_DATE;
	info.crc = Crc16_Calc((uint8_t*)&info,sizeof(info)-2);

	rf_send_message(&info, sizeof(info));
	RX_FSYNC_FLAG=0;
	rf_wait();
}
#endif

