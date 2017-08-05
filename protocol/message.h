#ifndef MESSAGE_H
#define MESSAGE_H

#ifndef BOOTLOADER
#include "FreeRTOS.h"
#include "timers.h"
#include "semphr.h"
#include "sys_arch.h"
#include "lwip/err.h"
#endif

#define SENSOR_NUMBER_MASK      0x0FFFFFFF
#define SENSOR_NUMBER_BROADCAST 0x0AAAAAAA
#define SENSOR_TYPE_MASK        0xF0000000

#define PROTOCOL_217 0x14
#define PROTOCOL_3xx 0x16
#define PROTOCOL_35  0x24


#define PROTOCOL_CALLBACK 0xAA

typedef enum
{
	SENSOR_TYPE_OBU			= 0x00,
	BRIDGE_TYPE_PLC         = 0x05,
	SENSOR_TYPE_GAS			= 0x10,
	SENSOR_TYPE_SMOKE		= 0x20,
	SENSOR_TYPE_WATER		= 0x30,
	SENSOR_TYPE_PIR			= 0x40,
	SENSOR_TYPE_MAGNETIC	= 0x50,
	SENSOR_TYPE_PIR_TX		= 0x60,
	SENSOR_TYPE_SEG_3		= 0x70,
	SENSOR_TYPE_SEG_2 		= 0x80,
	SENSOR_TYPE_REMOTE		= 0x90,
	SENSOR_TYPE_BROADCAST	= 0xA0,
	SENSOR_TYPE_SMOKE_TX	= 0xB0,
	SENSOR_TYPE_WATER_TX	= 0xC0,
	SENSOR_TYPE_MAGNETIC_TX	= 0xD0,
	SENSOR_TYPE_CONSUMPTION	= 0xE0,
	SENSOR_TYPE_CO			= 0xF0,
	SENSOR_TYPE_TEMPERATURE	= 0xF1,
	SENSOR_TYPE_SIREN		= 0xF2,
	SENSOR_TYPE_ACTUATOR	= 0xF3,
	SENSOR_TYPE_REPEATER	= 0xF4,
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
	RF_CMD_GENERIC_REQUEST2			= 0x0A,
	RF_CMD_GENERIC_REQUEST_ACK2		= 0x0B,
	RF_CMD_UNSOLICITED_STATUS2		= 0x0C,
	RF_CMD_UNSOLICITED_STATUS_ACK2	= 0x0D,
	RF_CMD_SET_CONFIGURATION		= 0x0E,
	RF_CMD_HEARTBEAT				= 0x10,
	RF_CMD_HEARTBEAT_ACK			= 0x11,
	RF_CMD_GET_ATTRIBUTE			= 0x12,
	RF_CMD_GET_ATTRIBUTE_ACK		= 0x13,
	RF_CMD_SET_ATTRIBUTE			= 0x14,
	RF_CMD_SET_ATTRIBUTE_ACK		= 0x15,
	RF_CMD_SET_START_ENTRY			= 0x16,
	RF_CMD_SET_START_ENTRY_ACK		= 0x17,

	RF_CMD_UPDATE_IMAGE 			= 0x8a,
	RF_CMD_QUERY_INFO				= 0x8b,

	RF_CMD_CHANGE_LOW_SENSITIVITY   	= 0x8c,
	RF_CMD_CHANGE_LOW_SENSITIVITY_ACK 	= 0x8d,
	RF_CMD_CHANGE_HIGH_SENSITIVITY  	= 0x8e,
	RF_CMD_CHANGE_HIGH_SENSITIVITY_ACK 	= 0x8f,
	
	RF_CMD_AGING_TEST_PREPARE			= 0xf7,
	RF_CMD_AGING_TEST_PREPARE_ACK		= 0xf8,
	RF_CMD_AGING_TEST_START				= 0xf9,
	RF_CMD_AGING_TEST_START_ACK			= 0xfa,
	RF_CMD_AGING_TEST_COMPLETE			= 0xfb,
	RF_CMD_AGING_TEST_COMPLETE_ACK 		= 0xfc,
	RF_CMD_TERM_TEST_QUERY				= 0xfd,
	RF_CMD_TERM_TEST_QUERY_ACK			= 0xfe,
	
};



enum DEVICE_TYPE_FUNCTION
{
	HOMEBOX = 0,
	BRIDGE_ = 0,
	SMOKE_SENSOR = 0x101,
	WATER_SENSOR = 0x102,
	MAGNETIC_SENSOR = 0x103,
	PIR_SENSOR = 0x104,
	VOLTAGE_SENSOR = 0x105,
	CONSUMPTION_SENSOR = 0x106,
	CO_SENSOR = 0x107,
	TEMPERATURE_SENSOR= 0x108,
	SIREN = 0x109,
	ACTUATOR = 0x10A,
	GAS_SENSOR = 0x10B,
};


#ifndef BOOTLOADER

typedef struct rf_packet_t
{
	uint8_t	    en;
	uint8_t	    query_acked;
	uint8_t		rssi;
	uint8_t		header;
	uint32_t	sn_dest;
	uint32_t 	sn_mitt;
	uint32_t 	sequence;
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
	uint16_t		crc2;
} RF_MSG

#ifdef __GNUC__
__attribute__((aligned(4)))
#endif
;

typedef struct rf_packet_ext_t
{
	uint8_t	    en;
	uint8_t	    query_acked;
	uint8_t		rssi;
	uint8_t		header;
	uint32_t	sn_dest;
	uint32_t 	sn_mitt;
	uint32_t 	sequence;
	uint8_t		cmd;
	uint8_t		index;
	uint16_t 	value;
	uint16_t 	value1;
	uint16_t	crc;
	uint16_t	crc2;
} RF_MSG_EXT
#ifdef __GNUC__
__attribute__((packed))
#endif
;

static inline void __put_unaligned_le16(uint16_t val, uint8_t *p)
{
        *p++ = val;
        *p++ = val >> 8;
}

static inline void __put_unaligned_le32(uint32_t val, uint8_t *p)
{
        __put_unaligned_le16(val >> 16, p + 2);
        __put_unaligned_le16(val, p);
}

static inline uint16_t_unaligned_le16(const uint8_t *p)
{
        return p[0] | p[1] << 8;
}

static inline uint32_t __get_unaligned_le32(const uint8_t *p)
{
        return p[0] | p[1] << 8 | p[2] << 16 | p[3] << 24;
}

typedef enum
{
	SENSOR_STATUS_NORMAL			= 0x00,
	SENSOR_STATUS_ALARMED,
	SENSOR_STATUS_ACKED,
	SENSOR_STATUS_TIMEOUT,
	SENSOR_STATUS_OFFLINE,
	SENSOR_STATUS_DL_FOTA_IMG,
	SENSOR_STATUS_FOTA_UPDATE,
	SENSOR_STATUS_WAIT_CALLBACK,

} SENSOR_STATUS;

typedef struct hbb_config_t
{
	uint32_t	longidh;
	uint32_t	longidm;
	uint32_t	id;
	uint32_t	master;
	uint32_t	heartbeat_interval;
	uint32_t	fota_check_interval;
	char alerturl[128];
	char eventurl[128];
	char controlurl[128];
	char fotaurl[128];
	unsigned char aesivv[20];
	unsigned char aeskey[40];
	char key_version[4];
	unsigned char aesivv_old[20];
	unsigned char aeskey_old[40];
	char mqtt_address[128];
	unsigned int mqtt_port;
	uint32_t freq;
	uint32_t security;
	uint32_t msg_audio_switch;
} hbb_config __attribute__((aligned(8)));

//typedef struct
//{
//	uint32_t	id;
//	uint32_t	dt;
//	uint32_t	hwverison;
//	uint32_t	heartbeat_interval;
//	uint32_t	fota_check_interval;
//	char 		nt[4];
//	char 		man[20];
//	char 		os[10];
//	char 		alerturl[128];
//	char 		eventurl[128];
//	char 		controlurl[128];
//	char 		fotaurl[128];
//	unsigned char aesivv[20];
//	unsigned char aeskey[40];
//	char key_version[4];
//	unsigned char aesivv_old[20];
//	unsigned char aeskey_old[40];
//	uint32_t pad;
//}hbb_param_t;
//#ifdef __GNUC__
//__attribute__((aligned(8)))
//#endif
//
//typedef struct _ppp_gprs_apn_pap_info_config
//{
//    CHAR 	gprs_apn[16];
//    CHAR 	ppp_pap_user[16];
//    CHAR 	ppp_pap_passwd[16];
//    u16_t 	sms_max_number_me;
//    u16_t 	sms_curr_number_me;
//}ppp_gprs_apn_pap_info_conf_t;
//#ifdef __GNUC__
//__attribute__((aligned(8)))
//#endif
//
//typedef struct hbb_config_t
//{
//	union
//	{
//		uint8_t param_dummy[2048];
//		hbb_param_t    param_real;
//	};
//	union
//	{
//		uint8_t gprs_dummy[1024];
//		ppp_gprs_apn_pap_info_conf_t  gprs_real;
//	};
//} hbb_config_t;
//#ifdef __GNUC__
//__attribute__((aligned(8)))
//#endif

typedef struct
{
	SemaphoreHandle_t xSemaphore;
	TimerHandle_t timer;
	uint32_t    msg_sent;
	uint32_t    uplink_sent;
	uint32_t    disengaged;
	uint32_t     status;
	TickType_t  hb_tick;
	uint32_t  	pversion;
	char nt[8];
	char man[20];
	char os[10];
	hbb_config config;
} HBB_STATUS_STORAGE_BLOCK
#ifdef __GNUC__
__attribute__((aligned(8)))
#endif
;

struct sensor_attribute {
	int index;
	uint32_t value;
};

struct attr_item {
	ListItem_t			xGenericListItem;
	int					nattr;
	struct sensor_attribute attr[4];
};

struct work_msg {
	ListItem_t			xGenericListItem;
	void (* function)(void *status, struct sensor_attribute *arg);
	int error;
	int retry;
	TickType_t  x_tick;
  /** arguments for this function */
	struct sensor_attribute *arg;
};

typedef struct
{
	List_t		todo_list;
	List_t 		attr_list;
	uint32_t	longidh;
	uint32_t	longidm;
	RF_MSG 		send_message;
	TickType_t  last_ack_tick;
	TickType_t  last_alarm_tick;
	uint32_t	device_type;
	char 		name[4];
	uint8_t     heartbeat_count;
	uint8_t     connect_status;
	uint8_t     qretry_count;
	uint8_t     engage;
	uint32_t	status;
	sys_sem_t 	*sem;
	void 		*arg;
	uint32_t 	arglen;
	err_t 		*err;
	uint32_t 	last_sequence;
	uint32_t	last_uplink_status;
	TickType_t  x_tick;
	TimerHandle_t alarm_timers;
	struct sensor_type *type_handle;
	struct attr_item sensor_attr;
	struct work_msg todo_work[2];
} SENSOR_STATUS_STORAGE_BLOCK

#ifdef __GNUC__
__attribute__((aligned(8)))
#endif
;

typedef struct
{
	uint32_t	longidh;
	uint32_t	longidm;
	uint32_t	id;
	uint32_t 	dt;
	uint32_t	sw;
	uint32_t	hw;
} plc_bridge_node;

struct sensor_type
{
	char *name;
	uint32_t devicetype;
	uint16_t protover;
	int (*init)(SENSOR_STATUS_STORAGE_BLOCK *status);
	int (*handle_rf_message)(SENSOR_STATUS_STORAGE_BLOCK *status, RF_MSG *msg);
	int (*receive_callback)(SENSOR_STATUS_STORAGE_BLOCK *status, RF_MSG *msg);
	int (*query)(SENSOR_STATUS_STORAGE_BLOCK *status, RF_MSG *msg);
};

#endif

/*****************************Common*********************************/

enum DEVICE_TYPE_GROUP
{
	HOMEBOX_GROUP = 0x1,
	SENSOR_GROUP = 0x2,
	BRIDGE_GROUP = 0x3,
};

#define DEVICE_TYPE(group, function, model) \
	(((group & 0xFF) << 24) | ((function & 0xFFF) << 12) | (model & 0xFFF))

#ifndef BOOTLOADER

static inline uint32_t DEVICE_TYPE_Group(uint32_t device_type)
{
	return  (device_type >> 24) & 0xFF;
}

static inline uint32_t DEVICE_TYPE_Function(uint32_t device_type)
{
	return  (device_type >> 12) & 0x00000FFF;
}

static inline uint32_t DEVICE_TYPE_Module(uint32_t device_type)
{
	return  device_type & 0xFFF;
}

static inline uint32_t GET_SENSOR_Type(SENSOR_STATUS_STORAGE_BLOCK *sensor_state)
{
	return 	(sensor_state->send_message.sn_dest>>28);
}

static inline uint32_t GET_SENSOR_SW_Ver(SENSOR_STATUS_STORAGE_BLOCK *sensor_state)
{
	return  ((((sensor_state->send_message.fw_major<<24)|(sensor_state->send_message.fw_minor<<16))&0xFFFF0000));
}

#endif

#endif
