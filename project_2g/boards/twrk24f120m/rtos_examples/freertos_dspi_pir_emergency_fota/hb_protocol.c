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
#include "board.h"


#define BUILD_SENSOR_HUB 1


#include "message.h"

#include "hb_protocol.h"

#define SVN_VERSION 889
#define BUILD_DATE 0x20160503


#define RX_WAIT_TIME_MS 400
#define TX_WAIT_TIME_MS 10
#define ALARM_TRIGGER_COUNT 1

#if BUILD_WATER_SENSOR
#define SENSOR_TYPE_CURRENT SENSOR_TYPE_WATER
#elif BUILD_SMOKE_SENSOR
#define SENSOR_TYPE_CURRENT SENSOR_TYPE_SMOKE
#elif BUILD_MAGNETIC_SENSOR
#define SENSOR_TYPE_CURRENT SENSOR_TYPE_MAGNETIC
#elif BUILD_PIR_SENSOR
#define SENSOR_TYPE_CURRENT SENSOR_TYPE_PIR
#elif BUILD_SENSOR_HUB
#define SENSOR_TYPE_CURRENT 0x00
#endif

#define CRC16_BITS 8
#define BATTERY_CHECK_COUNTER 24

uint32_t local_seq = 0;
#ifdef DEBUG
int enable_rf_resp = 1;
#endif
volatile int uart_receive_state = 0;
uint8_t uart_cmd = 0;
uint8_t swi = 0;
#if BUILD_SENSOR_HUB
UART_HUB_MSG uart_msg;
#else
UART_SENSOR_MSG uart_msg;
#endif

#ifndef PRODUCTION
#if BUILD_SENSOR_HUB
uint32_t serial_n = 0x00ABCDEF;
#elif BUILD_MAGNETIC_SENSOR
uint32_t serial_n = 0x51000001;
#elif BUILD_SMOKE_SENSOR
uint32_t serial_n = 0x20121937;
#elif BUILD_WATER_SENSOR
uint32_t serial_n = 0x31000004;
#elif BUILD_PIR_SENSOR
uint32_t serial_n = 0x41000001;
#else
uint32_t serial_n;
#endif
#else
uint32_t serial_n;
#endif


#ifndef VER_MAJOR
#define VER_MAJOR 2
#endif

#ifndef VER_MINOR
#define VER_MINOR 0
#endif

const uint32_t fw_v_major = VER_MAJOR;
const uint32_t fw_v_minor = VER_MINOR;


//skip 1st 4 meaningless bytes
RF_MSG recv_packet;
RF_MSG send_packet = { 0x00, 0x00, 0x00, 0x14};
SENSOR_STATUS_STORAGE_BLOCK sensor_status[MAX_SENSOR_COUNT];

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
void init_state()
{
#ifdef PRODUCTION
	serial_n = *(uint32_t*)0x410;
#endif
	srand(serial_n);

	memset(&send_packet.sn_dest, 0, offsetof(RF_MSG,crc)-offsetof(RF_MSG,sn_dest));
	send_packet.wor = 1;
	send_packet.sn_mitt = serial_n;
	send_packet.sn_dest = (SENSOR_TYPE_OBU<<28)|SENSOR_NUMBER_BROADCAST;
	send_packet.fw_major = fw_v_major;
	send_packet.fw_minor = fw_v_minor;
#if BUILD_MAGNETIC_SENSOR || BUILD_PIR_SENSOR
	send_packet.enable_data = 1;
#endif
#if BUILD_SMOKE_SENSOR
	HW_status.Peripheral_active = HW_status.RF_active = 0;
#endif
}


void init_A7139()
{
    uint32_t ID;
    SPI_A7139_INIT();
    A7139_StrobeCmd(0x55);
    A7139_Init(865.001);
    A7139_SetCID(0x78877887);
    ID = A7139_GetCID();

    A7139_StrobeCmd(CMD_STBY);
    vTaskDelay(10);
    A7139_SetGIO2_Func(e_GIOS_FSYNC);
    A7139_StrobeCmd(CMD_RX);

}

#if CRC16_BITS==4
uint16_t crc_cal(uint8_t* ptr, uint32_t len)
{
	static const  unsigned int crc_ta_4[16]={
		0x0000,0x1021,0x2042,0x3063,0x4084,0x50a5,0x60c6,0x70e7,
		0x8108,0x9129,0xa14a,0xb16b,0xc18c,0xd1ad,0xe1ce,0xf1ef,
	};
	uint16_t crc = 0;
	unsigned char high;
	while(len--)
	{
		high = (unsigned char)(crc/4096);
		crc <<= 4;
		crc ^= crc_ta_4[high^(*ptr/16)];

		high = (unsigned char)(crc/4096);
		crc <<= 4;
		crc ^= crc_ta_4[high^(*ptr&0x0f)];

		ptr++;
	}
	return crc;
}
#elif CRC16_BITS==8
uint16_t crc_cal(uint8_t* ptr, uint32_t len)
{
	static const  unsigned short crc_ta_8[256] = {
		0x0000, 0x1021, 0x2042, 0x3063, 0x4084, 0x50A5, 0x60C6, 0x70E7,
		0x8108, 0x9129, 0xA14A, 0xB16B, 0xC18C, 0xD1AD, 0xE1CE, 0xF1EF,
		0x1231, 0x0210, 0x3273, 0x2252, 0x52B5, 0x4294, 0x72F7, 0x62D6,
		0x9339, 0x8318, 0xB37B, 0xA35A, 0xD3BD, 0xC39C, 0xF3FF, 0xE3DE,
		0x2462, 0x3443, 0x0420, 0x1401, 0x64E6, 0x74C7, 0x44A4, 0x5485,
		0xA56A, 0xB54B, 0x8528, 0x9509, 0xE5EE, 0xF5CF, 0xC5AC, 0xD58D,
		0x3653, 0x2672, 0x1611, 0x0630, 0x76D7, 0x66F6, 0x5695, 0x46B4,
		0xB75B, 0xA77A, 0x9719, 0x8738, 0xF7DF, 0xE7FE, 0xD79D, 0xC7BC,
		0x48C4, 0x58E5, 0x6886, 0x78A7, 0x0840, 0x1861, 0x2802, 0x3823,
		0xC9CC, 0xD9ED, 0xE98E, 0xF9AF, 0x8948, 0x9969, 0xA90A, 0xB92B,
		0x5AF5, 0x4AD4, 0x7AB7, 0x6A96, 0x1A71, 0x0A50, 0x3A33, 0x2A12,
		0xDBFD, 0xCBDC, 0xFBBF, 0xEB9E, 0x9B79, 0x8B58, 0xBB3B, 0xAB1A,
		0x6CA6, 0x7C87, 0x4CE4, 0x5CC5, 0x2C22, 0x3C03, 0x0C60, 0x1C41,
		0xEDAE, 0xFD8F, 0xCDEC, 0xDDCD, 0xAD2A, 0xBD0B, 0x8D68, 0x9D49,
		0x7E97, 0x6EB6, 0x5ED5, 0x4EF4, 0x3E13, 0x2E32, 0x1E51, 0x0E70,
		0xFF9F, 0xEFBE, 0xDFDD, 0xCFFC, 0xBF1B, 0xAF3A, 0x9F59, 0x8F78,
		0x9188, 0x81A9, 0xB1CA, 0xA1EB, 0xD10C, 0xC12D, 0xF14E, 0xE16F,
		0x1080, 0x00A1, 0x30C2, 0x20E3, 0x5004, 0x4025, 0x7046, 0x6067,
		0x83B9, 0x9398, 0xA3FB, 0xB3DA, 0xC33D, 0xD31C, 0xE37F, 0xF35E,
		0x02B1, 0x1290, 0x22F3, 0x32D2, 0x4235, 0x5214, 0x6277, 0x7256,
		0xB5EA, 0xA5CB, 0x95A8, 0x8589, 0xF56E, 0xE54F, 0xD52C, 0xC50D,
		0x34E2, 0x24C3, 0x14A0, 0x0481, 0x7466, 0x6447, 0x5424, 0x4405,
		0xA7DB, 0xB7FA, 0x8799, 0x97B8, 0xE75F, 0xF77E, 0xC71D, 0xD73C,
		0x26D3, 0x36F2, 0x0691, 0x16B0, 0x6657, 0x7676, 0x4615, 0x5634,
		0xD94C, 0xC96D, 0xF90E, 0xE92F, 0x99C8, 0x89E9, 0xB98A, 0xA9AB,
		0x5844, 0x4865, 0x7806, 0x6827, 0x18C0, 0x08E1, 0x3882, 0x28A3,
		0xCB7D, 0xDB5C, 0xEB3F, 0xFB1E, 0x8BF9, 0x9BD8, 0xABBB, 0xBB9A,
		0x4A75, 0x5A54, 0x6A37, 0x7A16, 0x0AF1, 0x1AD0, 0x2AB3, 0x3A92,
		0xFD2E, 0xED0F, 0xDD6C, 0xCD4D, 0xBDAA, 0xAD8B, 0x9DE8, 0x8DC9,
		0x7C26, 0x6C07, 0x5C64, 0x4C45, 0x3CA2, 0x2C83, 0x1CE0, 0x0CC1,
		0xEF1F, 0xFF3E, 0xCF5D, 0xDF7C, 0xAF9B, 0xBFBA, 0x8FD9, 0x9FF8,
		0x6E17, 0x7E36, 0x4E55, 0x5E74, 0x2E93, 0x3EB2, 0x0ED1, 0x1EF0
	};
	uint16_t crc = 0;
	while(len--)
	{
		crc = (crc << 8 ) ^ crc_ta_8[((crc>>8) ^ *ptr)];
		ptr++;
	}
	return crc;
}
#endif


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
	if (GIO2S == TXIN) {
		PRINTF("Send RF Busy Now\r\n");
		return ;
	}else{
		GIO2S  = TXIN;
		GIO2_FLAG = 0;
	}
	rf_msg->crc = crc_cal((uint8_t*)&rf_msg->sn_dest, PACKET_CRC_LENGTH);

	rf_send_message(&rf_msg->header, PACKET_CONTENT_LENGTH);

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
}

void rf_send_then_receive_no_preamble(RF_MSG* rf_msg)
{
	if (GIO2S == TXIN) {
		PRINTF("SendNoPreamble RF Busy Now\r\n");
		return ;
	}else{
		GIO2S  = TXIN;
		GIO2_FLAG = 0;
	}
	rf_msg->crc = crc_cal((uint8_t*)&rf_msg->sn_dest, PACKET_CRC_LENGTH);

	rf_send_message_no_preamble(&rf_msg->header, PACKET_CONTENT_LENGTH);

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
}

uint8_t rf_send_data_block(uint8_t* buf)
{
	TickType_t	start_tick;
	A7139_SetPackLen(1024+8);
	A7139_SetGIO2_Func(e_GIOS_FPF);
	uint16_t crc[4] = {0, 0, 0, 0};
	for(int i=0;i<4;i++)
	{
		crc[i] = crc_cal(&buf[i*256],256);
	}
	uint8_t* ptr = buf;
	A7139_StrobeCmd(CMD_TFR);
	A7139_WriteFIFO(ptr, 64);
	ptr+=64;
	A7139_StrobeCmd(CMD_TX);
	start_tick = xTaskGetTickCount();
	for(int i=0;i<20;i++)
	{

		while(!GPIO_ReadPinInput(A7139_GIO2_GPIO, A7139_GIO2_GPIO_PIN)){
			if ((xTaskGetTickCount() - start_tick) >= 500) {
				PRINTF("RF MSG BLOCK TIMEOUT0\r\n");
				break;
			}
		}

		A7139_WriteFIFO(ptr, 48);
		ptr+=48;
	}

	start_tick = xTaskGetTickCount();
	while(!GPIO_ReadPinInput(A7139_GIO2_GPIO, A7139_GIO2_GPIO_PIN)){
		if ((xTaskGetTickCount() - start_tick) >= 500) {
			PRINTF("RF MSG BLOCK TIMEOUT1\r\n");
			break;
		}
	}

	A7139_WriteFIFO((uint8_t*)crc, 8);
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
		if(crc[i] != crc_cal(&buf[i*256],256))
		{
			return false;
		}
	}
	DBG_MESSAGE("recv %d done", 1024);
	return true;
}

#if BUILD_SENSOR_HUB

void rf_send_image(otah_t ota_header)
{
        uint8_t per = (ota_header.fwv == 3?0:50);
        EInt_GIO2_Disable();
	uint16_t crc = crc_cal((uint8_t *)&ota_header.rm_addr, 8);

//	EInt_GIO2_Disable(EInt_GIO2_DeviceData);
	A7139_SetPackLen(10);  // work around for not long enough WTR
	A7139_StrobeCmd(CMD_TFR);
	A7139_WriteFIFO((uint8_t *)&ota_header.rm_addr, 8);
	A7139_WriteFIFO((uint8_t *)&crc, 2);
	A7139_SetGIO2_Func(e_GIOS_WTR);
	A7139_StrobeCmd(CMD_TX);
	lp_sleep_for_ms(100);

	uint32_t length = (ota_header.len+0x3FF)>>10;
	uint8_t *addr = (uint8_t *)ota_header.lc_addr;
	for(int i=0;i<length;i++)
	{
		rf_send_data_block(addr);
		addr += 1024;
		lp_sleep_for_ms(100);
                PRINTF("%2d%%",per+=(50/length));
                PRINTF("\b\b\b");
	}

        EInt_GIO2_Enable();
	A7139_SetPackLen(21);
	//EInt_GIO2_Enable(EInt_GIO2_DeviceData);
}

void rf_send_less_image()
{
	UART_HUB_MSG_fota_update_header header = uart_msg.arg.fota_update_header;
	uint16_t crc = crc_cal((uint8_t *)&header.addr, 8);

//	EInt_GIO2_Disable(EInt_GIO2_DeviceData);
	A7139_SetPackLen(10);  // work around for not long enough WTR
	A7139_StrobeCmd(CMD_TFR);
	A7139_WriteFIFO((uint8_t *)&header.addr, 8);
	A7139_WriteFIFO((uint8_t *)&crc, 2);
	A7139_SetGIO2_Func(e_GIOS_WTR);
	A7139_StrobeCmd(CMD_TX);
	lp_sleep_for_ms(100);

	DBG_MESSAGE("send header done");

	uint32_t length = (header.length+0x3FF)>>10;
	uint8_t *addr = (uint8_t *)0x4000;
	for(int i=0;i<length-3;i++)
	{
		rf_send_data_block(addr);
		addr += 1024;
		lp_sleep_for_ms(100);
	}

	A7139_SetPackLen(21);
	//EInt_GIO2_Enable(EInt_GIO2_DeviceData);
}

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

	if(crc != crc_cal((uint8*)&header.addr, 8))
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
	info.crc = crc_cal((uint8_t*)&info,sizeof(info)-2);

	rf_send_message(&info, sizeof(info));
	RX_FSYNC_FLAG=0;
	rf_wait();
}
#endif
void uart_receive_message(void* buf, uint16_t count)
{

}

void uart_send_message(void* buf, uint32_t count)
{

}

void debug_printf(const char * fmt,...)
{

}

void handle_preamble_timeout()
{

}

void handle_preamble_start()
{

}
