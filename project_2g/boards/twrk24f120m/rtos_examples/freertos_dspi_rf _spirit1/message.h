#ifndef MESSAGE_H
#define MESSAGE_H

#include <stdint.h>
#include "portmacro.h"
#include "timers.h"

#ifdef WIN32
typedef signed char             int8;
typedef signed short int        int16;
typedef signed long int         int32;

typedef unsigned char           uint8_t;
typedef unsigned short int      uint16_t;
typedef unsigned long int       uint32_t;

#pragma pack(push,8)

#endif

#define SENSOR_NUMBER_MASK      0x0FFFFFFF
#define SENSOR_NUMBER_BROADCAST 0x0AAAAAAA
#define SENSOR_TYPE_MASK        0xF0000000

typedef enum
{
	SENSOR_TYPE_OBU			= 0x00,
	SENSOR_TYPE_GAS			= 0x01,
	SENSOR_TYPE_SMOKE		= 0x02,
	SENSOR_TYPE_WATER		= 0x03,
	SENSOR_TYPE_PIR			= 0x04,
	SENSOR_TYPE_MAGNETIC	= 0x05,
	SENSOR_TYPE_PIR_TX		= 0x06,
	SENSOR_TYPE_SEG_3		= 0x07,
	SENSOR_TYPE_SEG_2 		= 0x08,
	SENSOR_TYPE_REMOTE		= 0x09,
	SENSOR_TYPE_BROADCAST	= 0x0a,
	SENSOR_TYPE_SMOKE_TX	= 0x0b,
	SENSOR_TYPE_WATER_TX	= 0x0c,
	SENSOR_TYPE_MAGNETIC_TX	= 0x0d,
	SENSOR_TEST_AGENT       = 0x10
} SENSOR_TYPE;

enum
{
	RF_CMD_INVALID					= 0x00,
	RF_CMD_GENERIC_REQUEST			= 0x01,
	RF_CMD_GENERIC_REQUEST_ACK		= 0x02,
	RF_CMD_UNSOLICITED_STATUS		= 0x03,
	RF_CMD_UNSOLICITED_STATUS_ACK	= 0x04,
	RF_CMD_ALARM_REARM				= 0x05,
	RF_CMD_ALARM_REARM_ACK			= 0x06,
	RF_CMD_PUSH_ACTIVATION			= 0x07,
	RF_CMD_LED_CONTROL				= 0x08,
	RF_CMD_SEQ_RESYNC				= 0x09,
	RF_CMD_UPDATE_IMAGE 			= 0x8a,
	RF_CMD_QUERY_INFO				= 0x8b,
};

typedef struct rf_packet_t
{
	uint32_t		age;
	uint16_t		retry;
	uint8_t		type;
	uint8_t		header;
	uint32_t		sn_dest;
	uint32_t 		sn_mitt;
	uint32_t 		sequence;
	uint8_t		cmd;
	uint8_t		vbatt : 2;
	uint8_t		wor : 1;
	uint8_t		alarm_status : 1;
	uint8_t		alarm_sent : 1;
	uint8_t		bbr_corrupted : 1;
	uint8_t		enable_cmd : 1;
	uint8_t		enable_data : 1;
	uint8_t		fw_major;
	uint8_t		data;
	uint8_t		fw_minor;
	uint8_t		rf_reset_cnt;
	uint16_t		crc;
} RF_MSG

#ifdef __GNU__
__attribute__((aligned(4)))
#endif
;

enum
{
	UART_CMD_QUERY_TYPE = 100,
};

enum
{
	UART_SENSOR_CMD_INVALID,
	UART_SENSOR_CMD_LOG,
	UART_SENSOR_CMD_QUERY,
	UART_SENSOR_CMD_QUERY_RESP,
	UART_SENSOR_CMD_ALARM,
	UART_SENSOR_CMD_DISALARM,
	UART_SENSOR_CMD_ENABLE,
	UART_SENSOR_CMD_DISABLE,
	UART_SENSOR_CMD_UPDATE_IMAGE,
	UART_SENSOR_CMD_RAPID,	//set chk rate rapid
	UART_SENSOR_CMD_LOW_POWER,//low power alarm
	UART_SENSOR_CMD_LED_FLASH,//led flashing
	UART_SENSOR_CMD_BUTTON,
	UART_TEST_AGENT_SEND
};

typedef struct
{
	uint32_t addr;
	uint32_t length;
} UART_SENSOR_MSG_fota_update_header;

typedef struct
{
	uint8_t  data[1024];
} UART_SENSOR_MSG_fota_update_data;

typedef struct
{
	uint16_t preamble;
	uint8_t data[18];
} UART_TEST_AGENT_send;


typedef struct
{
	union {
		UART_SENSOR_MSG_fota_update_header     fota_update_header;
		UART_SENSOR_MSG_fota_update_data       fota_update_data;
		UART_TEST_AGENT_send                    test_agent_send;
   } arg;
} UART_SENSOR_MSG;

enum
{
	UART_HUB_CMD_IINVALID,
	UART_HUB_CMD_LOG,
	UART_HUB_CMD_SET_KEY,
	UART_HUB_CMD_SENSOR_LIST,

	UART_HUB_CMD_SENSOR_STATUS_REQ,
	UART_HUB_CMD_SENSOR_STATUS_RESP,
	UART_HUB_CMD_SENSOR_CHANGED,
	UART_HUB_CMD_SENSOR_CHANGED_RESP,
	UART_HUB_CMD_SENSOR_RESTART_REQ,
	UART_HUB_CMD_SENSOR_RESTART_RESP,
	UART_HUB_CMD_SENSOR_ACTIVATION_REQ,
	UART_HUB_CMD_SENSOR_ACTIVATION_RESP,
	UART_HUB_CMD_FOTA_UPDATE_IMAGE,
	UART_HUB_CMD_FOTA_UPDATE_IMAGE_RESP,
	UART_HUB_CMD_SENSOR_INFO_QUERY_REQ,
	UART_HUB_CMD_FOTA_UPDATE_BLOCK_TEST,
	UART_HUB_CMD_FOTA_UPDATE_LESS_IMAGE,
	UART_HUB_CMD_FOTA_UPDATE_BIT_ERR_IMAGE,
	UART_HUB_CMD_FOTA_UPDATE_IMAGE_CRC_ERR,
	UART_HUB_CMD_CPURESET	
};


typedef struct
{
	uint8_t key[8];
} UART_HUB_MSG_set_key;

typedef struct
{
	uint32_t sensor_cnt;
	uint32_t id[16];
} UART_HUB_MSG_sensor_list;

typedef struct
{
	uint32_t id;
	uint32_t broadcast;
} UART_HUB_MSG_sensor_status_req;

typedef struct
{
	uint32_t id;
	uint8_t  response;
	uint8_t  type;
	uint8_t  alarm : 4;
	uint8_t  battery : 4;
	uint8_t  rf_reset_cnt;
	uint32_t fw_version;
	uint32_t build_version;
} UART_HUB_MSG_sensor_status_resp;

typedef struct
{
	uint32_t id;
	uint8_t  type;
	uint8_t  alarm : 4;
	uint8_t  battery : 4;
	uint8_t  rf_reset_cnt;
	uint32_t fw_version;
	uint32_t build_version;
} UART_HUB_MSG_sensor_changed;

typedef struct
{
	uint32_t id;
} UART_HUB_MSG_sensor_changed_resp;

typedef struct
{
	uint32_t id;
} UART_HUB_MSG_sensor_restart_req;

typedef struct
{
	uint32_t id;
	uint8_t  response;
	uint8_t  rf_reset_cnt;
} UART_HUB_MSG_sensor_restart_resp;

typedef struct
{
	uint32_t id;
	uint8_t  state;
} UART_HUB_MSG_sensor_activation_req;

typedef struct
{
	uint32_t id;
	uint8_t  response;
	uint8_t  rf_reset_cnt;
} UART_HUB_MSG_sensor_activation_resp;

typedef struct
{
	uint32_t id;
	uint16_t crc[2];
	uint32_t addr;
	uint32_t length;
	
} UART_HUB_MSG_fota_update_header;

typedef struct
{
	uint8_t  data[1024];
} UART_HUB_MSG_fota_update_data;

typedef struct
{
	union {
		UART_HUB_MSG_set_key                    set_key;
		UART_HUB_MSG_sensor_list			    sensor_list;
		UART_HUB_MSG_sensor_status_req		    sensor_status_req;
		UART_HUB_MSG_sensor_status_resp	        sensor_status_resp;
		UART_HUB_MSG_sensor_changed		        sensor_changed;
		UART_HUB_MSG_sensor_changed_resp	    sensor_changed_resp;
		UART_HUB_MSG_sensor_restart_req	        sensor_restart_req;
		UART_HUB_MSG_sensor_restart_resp	    sensor_restart_resp;
		UART_HUB_MSG_sensor_activation_req	    sensor_activation_req;
		UART_HUB_MSG_sensor_activation_resp     sensor_activation_resp;
		UART_HUB_MSG_fota_update_header         fota_update_header;
		UART_HUB_MSG_fota_update_data           fota_update_data;

   } arg;
} UART_HUB_MSG
#ifdef __GNU__
__attribute__((aligned(4)))
#endif
;

typedef struct
{
	uint32_t	id;
	uint32_t	rf_status;
} SENSOR_STATUS_HEAD

#ifdef __GNU__
__attribute__((aligned(4)))
#endif
;

typedef struct
{
	char name[4];
	RF_MSG send_message;
	uint8_t     heartbeat_count;
	uint8_t     connect_status;
	uint8_t     disegage_count;
	TickType_t  x_tick;
	TimerHandle_t alarm_timers;
} SENSOR_STATUS_STORAGE_BLOCK

#ifdef __GNU__
__attribute__((aligned(4)))
#endif
;


#ifdef WIN32

#pragma pack(pop)

#endif
#endif
