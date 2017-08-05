#ifndef __SUPERVISOR_TASK_H_
#define __SUPERVISOR_TASK_H_

enum
{
	DEVICE_ERROR_TYPE = 30001,
	RF_ERROR = 30002,
	SPI_FLASH_ERROR = 30003,
	PLC_ERROR = 30004,
	WIFI_ERROR = 30005,
	ZIGBEE_ERROR = 30006,
};

enum Peripherals_Index
{
	Peripherals_Plc,
	Peripherals_Wifi,
	Peripherals_Km,
	Peripherals_NorFlash,
	Peripherals_A7139,
	Peripherals_Zigbee,
};

typedef struct{
	volatile uint32_t	A7139;
	volatile uint32_t	NorFlash;
	volatile uint32_t	Plc;
	volatile uint32_t	Wifi;
	volatile uint32_t	Km;
	volatile uint32_t	Zigbee;
}PERIPHERALS_COUNT_t;


typedef struct{
	uint8_t		a7139_check;
	uint8_t		nor_flash_check;
	uint8_t		wifi_check;
	uint8_t		plc_check;
	uint8_t 	zigbee_check;
}POST_STATUS_t;

extern POST_STATUS_t 		post_status;
extern PERIPHERALS_COUNT_t	PeripheralsCount;
extern uint32_t	reset_reason;


void Increase_PeripheralsCntr(uint32_t Index);
uint32_t Get_PeripheralsIncreasedCntr(uint32_t Index);
void InitCheckPeripherals(void);

#endif
