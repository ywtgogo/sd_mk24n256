#ifndef __PLC_K24_API_H_
#define __PLC_K24_API_H_
#include "uart_plc.h"

#define MAX_PLC_DATA_LEN	248



typedef struct
{
	uint16_t	header;
	uint8_t     data[MAX_PLC_DATA_LEN-2];
}HEADER_PART;

typedef struct
{
	uint32_t 	len;
	union{
		HEADER_PART header_msg;
		uint8_t     plc_data[MAX_PLC_DATA_LEN];
	};
}PLC_DATA_MSG;

extern QueueHandle_t plc_data_queue;
extern PLC_DATA_MSG rev_data;

uint32_t send_plc_msg(uint8_t *pBuf, uint32_t length);

#endif
