#ifndef _PLC_TASK_H_
#define _PLC_TASK_H_

#include "hb_protocol.h"

#define PLC_Q_LENGTH 5
#define PLC_DATA_Q_LENGTH 1
#define PLC_BLOCK_Q_LENGTH 1
#define PLC_WAIT_SENSOR_LIST_TIME_OUT 200
#define PLC_WAIT_IMG_BLOCK_TIME_OUT 500
#define PLC_MAX_REQ_IMG_BLOCK_CNT 5

//#define PLC_ALARM_POLLING_INTERVAL 120000
//FOT TEST
#define PLC_ALARM_POLLING_INTERVAL 10000

enum
{
	PLC_CMD_INVALID					= 0x00,
	PLC_CMD_GENERIC_REQUEST			= 0x01,
	PLC_CMD_GENERIC_REQUEST_ACK		= 0x02,
	PLC_CMD_ALARM					= 0x03,
	PLC_CMD_ALARM_ACK				= 0x04,
	PLC_CMD_ALARM_CLEAR				= 0x05,
	PLC_CMD_ALARM_CLEAR_ACK			= 0x06,
	PLC_CMD_TOGGLE					= 0x07,
	PLC_CMD_TOGGLE_ACK				= 0x08,
	PLC_CMD_SET_PIR_SENSITIVITY		= 0x09,
	PLC_CMD_SET_PIR_SENSITIVITY_ACK	= 0x0A,

	PLC_CMD_SENT_SENSOR_LIST 		= 0x10,
	PLC_CMD_SENT_SENSOR_LIST_ACK 	= 0x11,
	PLC_CMD_DEPLOY_SENSOR_LIST 		= 0x12,
	PLC_CMD_DEPLOY_SENSOR_LIST_ACK  = 0x13,
	PLC_CMD_REQUEST_SENSOR_LIST		= 0x14,

	PLC_CMD_UPDATE_FW				= 0x20,
	PLC_CMD_UPDATE_FW_ACK			= 0x21,
	PLC_CMD_REQUEST_IMG_BLOCK		= 0x22,
	PLC_CMD_REQUEST_IMG_BLOCK_ACK	= 0x23,
	PLC_CMD_FW_UPDATE_RESULT		= 0x24,
};

enum
{
	RTC_SYNC,
	GENERIC_REQUEST,
};

enum
{
	DISENGAGE,
	  ENGAGE,
};

enum
{
	 LOW_SENSITIVITY,
	HIGH_SENSITIVITY,
};

enum
{
	FOTA_FAILED,
	FOTA_SUCCEED,
};

typedef struct
{
	uint16_t		header;
	uint8_t		    retry;
	uint8_t			cmd;
	uint32_t		device_type;
	uint32_t		bridge_sw;
	uint32_t		bridge_hw;
	uint32_t 		sn_mitt;
	uint32_t 		sn_dest;
	uint32_t		sn_sensor;
	uint32_t 		sequence;
	uint64_t 		time_stamp;
	uint32_t		data;
	uint16_t		reserve;
	uint16_t		crc;
}PLC_MSG

#ifdef __GNUC__
__attribute__((aligned(4)))
#endif
;
typedef struct
{
	RF_MSG msg;
}PLC_SENSOR_NODE;

typedef struct
{
	uint16_t		header;
	uint16_t		crc;
	PLC_SENSOR_NODE sensor_info[MAX_SENSOR_COUNT];
}PLC_SENSOR_LIST __attribute__((aligned(4)));

typedef struct
{
	uint16_t	header;
	uint16_t	reserve;
	uint8_t 	data[4096];
}PLC_FW_BLOCK;

typedef struct
{
	PLC_MSG    msg;
	PLC_MSG*   next;
}PLC_MSG_LINK_NODE;

enum
{
	PLC_MSG_HEADER = 0xF11F,
	PLC_SENSOR_LIST_HEADER = 0xF22F,
	PLC_FW_BLOCK_HEADER = 0xF33F,
};

#define PLC_PACKAGE_LEN sizeof(PLC_MSG)
#define PLC_SENSOR_LIST_LEN sizeof(PLC_SENSOR_LIST)
#define PLC_FW_BLOCK_LEN sizeof(PLC_FW_BLOCK)

void update_sensor_route();
void plc_status_task(void *pvParameters);
void plc_get_message_task(void *pvParameters);

extern uint32_t plc_local_seq;
extern PLC_SENSOR_LIST plc_sent_senosr_list;
extern PLC_SENSOR_LIST plc_recv_sensor_list;

extern uint32_t seq_record[MAX_SENSOR_COUNT];
extern SemaphoreHandle_t plc_fota_block_sem;

extern SemaphoreHandle_t plc_sensor_query_back_sem;
extern SemaphoreHandle_t plc_slist_back_sem;
extern SemaphoreHandle_t plc_sensor_list_back_sem;
extern EventGroupHandle_t BridgeFwReqAckEventGroup;
#define B_BRIDGE_FW_REQ_ACK 	(1 << 0)
#define B_SENSOR_FW_RET_ACK 	(1 << 1)

extern PLC_FW_BLOCK plc_recv_fw_block;
#ifndef BRIDGE
extern PLC_FW_BLOCK plc_sent_fw_block;
#endif

#endif
