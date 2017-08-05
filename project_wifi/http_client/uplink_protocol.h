/*
 * uplink_task.h
 *
 *  Created on: 2016Äê9ÔÂ2ÈÕ
 *      Author: yiqiu.yang
 */

#ifndef HTTP_UPLINK_PROTOCOL_H_
#define HTTP_UPLINK_PROTOCOL_H_
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "timers.h"
#include "fsl_device_registers.h"
#include "fsl_debug_console.h"
#include "board.h"
#include "semphr.h"
#include "message.h"
#include "event_groups.h"

#include "pin_mux.h"
#include "clock_config.h"
#include "SPI_A7139.h"
#include "uplink_protocol.h"
#include "spiflash_config.h"
#include "fota.h"
#include "hbb_config.h"
/********define*******************/
#define UPLINK_PROTOCOL_DEBUG 	1//1:Turn on  0:Turn off
#if UPLINK_PROTOCOL_DEBUG
#define UplinkProtoDbPrint			printf
#else
#define UplinkProtoDbPrint
#endif

#define UPLINK_PROTOCOL_TRACE 	1//1:Turn on  0:Turn off
#if UPLINK_PROTOCOL_TRACE
#define UplinkProtoTracePrint		printf
#else
#define UplinkProtoTracePrint
#endif

#define TEST 0       //debug in uplink 0:turn off  1:turn on

#define EXAMPLE_ENET ENET
//define MOB_PH_NUM_LENGTH 11+1//Mobile phone number length

#define JSON_BUF_SIZE 1000 // json data can't more then 1000 BYTE !!!!!!!
//#define MQTT
#define UPLINK_Q_LENGTH 32

#define UPLINK_HANDLE_OK      0
#define UPLINK_HANDLE_ERR    -1


#define ENCROPT  //IF define ,the json body will ENCROPT with aes256

#define FAIL_LIST_MAX_NUM    MAX_SENSOR_COUNT
 //version: a.b.c.d
#define VER_TO_UINT32(ver, a,b,c,d) \
        		ver = ((uint32_t)((a) & 0xff) << 24) | \
                         ((uint32_t)((b) & 0xff) << 16) | \
                         ((uint32_t)((c) & 0xff) << 8)  | \
                          (uint32_t)((d) & 0xff)

/*-------define HBB error code ------
 */
enum HBB_ERR_CODE
{
	HBB_ERR_RF = 30002,
	HBB_ERR_SPI_FLASH = 30003,
	HBB_ERR_PLC = 30004,
	HBB_ERR_WIFI = 30005,
	HBB_ERR_ZIGBEE = 30006,
};
/*-------define HBB warning code ------
 */
enum HBB_WARNING_CODE
{
	//Reasons to homebox starting up
	HBB_WARNING_FOTA = 10101,
	HBB_WARNING_LOW_BATTERY = 10102,
	HBB_WARNING_WATCHDOG = 10103,
	HBB_WARNING_RESET_PRESSED = 10104,
	HBB_WARNING_SORFTWARE_RESET = 10105,
	//Reasons to homebox shutdown
	HBB_WARNING_BATTERT_LOW_VALT = 10201,
	HBB_WARNING_POWER_OFF_BUTTON = 10202,
	//Power switch
	HBB_WARNING_AD_DC_SWITCH = 10302,
};


enum CLIENT
{
	CLIENT_OK,
	CLIENT_RETRY,
	CLIENT_CACHE,
	CLIENT_ERR,
	SERVER_ERR,
};

enum JASON_PARSE_STATUS
{
	JSON_PARSE_ERR,
	JSON_PARSE_OK,
};

enum RESULT_CODE
{
	/* Http status code */
	REQ_SUCC = 200,//1
	BAD_REQ = 400,
	ERR_404 = 404,
	ERR_403 = 403,
	/*FOTA*/
	GET_METADATA_DECRPT_FAIL   = 440,//2
	GET_METADATA_INVAL = 441,		//The getMetaData response data invalid, eg. required parameter missing.
	DEV_NOT_EXIST = 442,			//Device does not exist according to getMetaData info.
	FW_IMG_DOWN_FAIL = 443,			//Upgrade firmware img download fail
	FW_IMG_DECTYP_FAIL = 444,		//The firmware img decryption fail.
	FW_IMG_CRC_FAIL = 445,			//The firmware img CRC fail
	ALL_DEV_FOTA_FAIL = 446,		//The upgrade operation of all this type devices fail.
	PART_OF_DEV_FAIL = 447,			//The upgrade operation of parts of this type devices fail.
	UPDATE_PARA_ERR = 448,//

	DOWN_IMAGE_ABORT = 449,			//download image abort, maybe net disconnect, NOT CANCELL RULE
	DOWN_SHA1_ERR = 450,			//sha1 blockN image check error
	DOWN_HBB_SWVER_ERR = 451,		//HBB software VER check error
	DOWN_HBB_HWVER_ERR = 452,		//HBB hardware VER check error
	DOWN_HBB_DEVTYPE_ERR = 453,		//HBB device type check error
	DOWN_SENSOR_DEVTYPE_ERR = 454,
	BOOT_HBB_BLK_SHA1_ERR = 455,
	BOOT_HBB_ALL_SHA1_ERR = 456,
	BOOT_HBB_VER_ROOLBACK = 457,
	NOR_SENSOR_BLK_SHA1_ERR = 458,
	NOR_SENSOR_ALL_SHA1_ERR = 459,
	HBB_LOW_POWER = 460,			//HBB NOT ENOUGH POWER, NOT CANCELL RULE
	HBB_CALL_INCOMING = 461,		//NOT CANCELL RULE

	FOTA_REQ_DECRYP_FAIL = 520,		//Fota client request decryption fail.
	FOTA_REQ_DATA_INVAL = 521,		//The fota client request data invalid.
	NO_DEV_UPD = 530,				//No device need to upgrade in the homebox.
	PARAM_NULL_OR_INVAL = 531,		//Some parameter of the device to upgrade is null or invalid.
	DEV_NOT_FOUND = 532,			//The upgraded device can not found in database.
	SIGNATURE_NOT_GET = 533,		//Signature key can't get from database.
	SIGNATURE_NOT_MATCH = 534,		//The signature key doesn't match.
	IMG_NOT_EXIST = 535,			//The img need to download does not exist.
	SERVER_UNEXPECT_EER = 540,		//The service occurred unexpected error.
	SERVER_UNSUP_ENCODING = 541,	//The service occurred unexpected unsupported encoding exception.
	SERVER_IOEXCEPTION = 542,		//The service occurred IOException.
	/* Control */
	REQ_DEV_NOT_FOUND = 550,		//The requested device couldn't be found
	REQ_DATA_INVAL = 551,			//The request data invalid.
	SERVER_OC_UNEXP_ERR = 553,		//The service occurred unexpected error.
	NO_DEV_FOUND = 554,				//No device be found by the DID.
	SERVER_OC_UNSUP_ERR = 555,		//The service occurred unexpected unsupported encoding exception.
	SERVER_OC_IOE = 556,			//The service occurred IOException.
	SIG_KEY_NOT_MATCH = 557,		//The signature key doesn't match.
};

/*-------define uplink queue content------
 *
 *
 *
 */
typedef struct
{
	uint32_t idh;
	uint32_t idm;
	uint32_t id;
}LONG_ID96;

enum UPLINK_CMD
{
	//common
	UPLINK_SEND_EVENT,
	//http cmd
	UPLINK_FW_UPD_REQ,
	UPLINK_DOWNLOAD_IMG,
	UPLINK_FW_UPD_RESULT,
	UPLINK_GET_SUB_DEVICES,
	//mqtt cmd
	MQTT_GET_DEV_LIST,
	MQTT_SEND_DEV_LIST_RESULT,
	MQTT_SUB_ALL,
	MQTT_UPLOADING_LOG,
	MQTT_SEND_DATA,
};

typedef struct
{
	LONG_ID96 slid;
	uint32_t status;
	uint32_t device_type;
	uint64_t ts;
	int p;
	uint32_t fwv;
	uint32_t hwv;
	char data[50];
}UPLINK_EVENT_STRUCT;

typedef struct
{
	uint32_t bid;
}UPLINK_FW_UPD_REQ_STRUCT;

typedef struct
{
	uint32_t rid;
	uint32_t img_blk_num;
	uint32_t back_cmd;
	char *key;//it is a string
	char *url;//it is a string
}UPLINK_FW_DOWNLOAD_IMG_STRUCT;

typedef struct
{
	uint32_t rid;
	uint32_t rc;
	LONG_ID96 *fl;//only use when rc = 447
	int flnum;//only use when rc = 447
	uint32_t ts;
}UPLINK_FW_UPD_RESULT_STRUCT;

typedef struct
{
	uint32_t pos;

}UPLING_SUB_DEV_REQ_STRUCT;

typedef struct
{
	enum UPLINK_CMD cmd;
	union
	{
		UPLINK_EVENT_STRUCT event;
		UPLINK_FW_UPD_REQ_STRUCT fw_req;
		UPLINK_FW_DOWNLOAD_IMG_STRUCT fw_down;
		UPLINK_FW_UPD_RESULT_STRUCT fw_upd_result;
		UPLING_SUB_DEV_REQ_STRUCT sub_dev_req;
	};

}UPLINK_Q_MSG;

/*---------define uplink protocol----------
 *
 *
 */


enum ENCRYPTION_TYPE
{
	NOT_ENC = 0,
	AES128,
	AES256,
};
enum SIGNATURE_TYPE
{
	NOT_SIG = 0,
	MD5,
	SHA1,
};

typedef struct
{
	char prot_ver[16];
	char hard_ver[16];
	char fw_ver[16];
}HOMEBOX_VERSION;



/************************Event protocol*******************************/
enum EVENT_TYPE
{
	ALERT = 10,
	UNALERT = 11,
	INFO = 20,
	ERROR = 30,
	WARNING = 40,
};

 typedef struct
 {
	// char PV[8];//protocol version ex:0.1.2.3
	 int enc; //encryption setting   0:not encryption 1: AES128 2:AES256
	 int sig;//signature setting   0 not signature 1:MD5 2:SHA1
	 uint32_t sts_s;//2016/06/05/03:04 \0
	 uint32_t sts_ms;
 }EVENT_REQ_HEADER;

 typedef struct
 {
	 char msgid[33];//message id
	 uint32_t dt;//device type 110:homebox 201:smoke 202:water
	 uint32_t t;//event type
	// uint32_t sid;// sensor device id
	// uint32_t did;
	 LONG_ID96 *slid;
	 uint32_t ts_s; //time stamp of seconds
	 uint32_t ts_ms;//time stamp of ms
	 char src[16];//ip address 192.225.225.225
	 uint32_t p;//priority 1-5
	 char *data;
//	 uint32_t hwv;
	 char fwv[16];
	 char hwv[16];
//	 char *m; //Mobile phone number pointer
 }EVENT_REQ_BODY;
/*113 byte*/
 typedef struct
 {
	 EVENT_REQ_HEADER header;
	 EVENT_REQ_BODY body;
 }EVENT_REQ_STRUCT;


 /******************************Fota protocol**************************/
 enum TYPE
 {
	 IMAGE = 1,
	 PARAMETER = 2,
	 MODULE = 3,
 };

 enum PARAMETER_TYPE
 {
	 PARA_URL_FOTA = 1,
	 PARA_URL_EVENT,
	 PARA_URL_ALERT,
	 PARA_URL_CTRL,
	 PARA_AES_KEY,
	 PARA_HEART_BEAT,
	 PARA_SOS_PHONE,
	 PARA_AUTO_ANSWER,
	 PARA_FOTA_INTERVAL,
 };

 typedef struct
 {
	 int enc; //encryption setting   0:not encryption 1: AES128 2:AES256
	 int sig;//signature setting   0 not signature 1:MD5 2:SHA1
 }FW_UPDATE_REQ_HEADER;

 typedef struct
 {
	 char msgid[33];//message id
	 uint32_t bid;//BatchId
 }FW_UPDATE_REQ_BODY;

 typedef struct
 {
	 FW_UPDATE_REQ_HEADER header;
	 FW_UPDATE_REQ_BODY body;
 }FW_UPDATE_REQ_STRUCT;

 typedef struct
 {
	 char pv[8];
	 int sig;
	 int enc;
 }FW_UPDATE_RESP_HEADER;

 typedef struct
 {
 	 uint32_t mid;
 	 uint32_t fws;
 	 uint32_t fwblks;
 	 char *url;
 	 char *key;

 }FW_UPDATE_BODY_DOWNLOAD_IMG;

 typedef struct
 {
	 unsigned char newkey[40];
	 unsigned char newVector[20];
	 char newEncvr[4];//enc version
 }FW_UPDATE_BODY_PARA_AES;

 typedef struct
 {
	 int para_type;
	 union{
		 uint32_t heartbeat;
		 uint32_t fota_interval;
		 char url[128];
		 uint32_t auto_answer;
		 char sos[50];
		 FW_UPDATE_BODY_PARA_AES aes;
	 };
 }FW_UPDATE_BODY_PARA;

 typedef struct
 {
 	 uint32_t fwuv;
 	 uint32_t dt;
 	 char msgid[33];
 	 uint32_t t;
 	 uint32_t rid;
 	 uint32_t p;
 	 uint32_t hdv;
 	 uint32_t fwpv;
 	 uint32_t rc;
 	 uint32_t ifm;
 	 union{
 	 FW_UPDATE_BODY_DOWNLOAD_IMG img;
 	 FW_UPDATE_BODY_PARA parameter;
 	 };

  }FW_UPDATE_RESP_BODY;

 typedef struct
 {
	 FW_UPDATE_RESP_HEADER header;
	 FW_UPDATE_RESP_BODY body;
 }FW_UPDATE_RESP_STRUCT;

 typedef struct
 {
	 char pv[8];
	 int sig;
	 int enc;
 }FW_IMAGE_REQ_HEADER;

 typedef struct
 {
	 char msgid[33];
	 uint32_t rid;
	 uint32_t img_blk_num;
	 char sign[41];
	 char *url;
 }FW_IMAGE_REQ_BODY;

 typedef struct
 {
	 FW_IMAGE_REQ_HEADER header;
	 FW_IMAGE_REQ_BODY body;
 }FW_IMAGE_REQ_STRUCT;

 typedef struct
 {
	 int sig;
	 int enc;
 }FW_UPDATE_RESULT_REQ_HEADER;

 typedef struct
 {
	 char msgid[33];
	 uint32_t rid;
	 uint32_t did;
	 uint32_t rc;
 }FW_UPDATE_RESULT_REQ_BODY;

 typedef struct
 {
	 FW_UPDATE_RESULT_REQ_HEADER header;
	 FW_UPDATE_RESULT_REQ_BODY body;
 }FW_UPDATE_RESULT_REQ_STRUCT;


 /********************Control protocol**************************/
 typedef struct
 {
	 int sig;
	 int enc;
 }GET_SUB_DEV_REQ_HEADER;

 typedef struct
 {
	 char msgid[33];
//	 uint32_t dt;
	 uint32_t pos;
 }GET_SUB_DEV_REQ_BODY;

 typedef struct
 {
	 GET_SUB_DEV_REQ_HEADER header;
	 GET_SUB_DEV_REQ_BODY body;
 }GET_SUB_DEV_REQ_STRUCT;

 typedef struct
 {
	 char pv[8];
	 int sig;
	 int enc;
 }GET_SUB_DEV_RESP_HEADER;

 typedef struct
 {
	 LONG_ID96 lid;
	 uint32_t dt;
 }LONGID_AND_DT;

 typedef struct
 {
	LONGID_AND_DT idn[MAX_SENSOR_COUNT+1];
 	int num;
 }DEVICE_LIST;

 typedef struct
 {
	 uint32_t rc;
	 uint32_t cts;
	 DEVICE_LIST sids;
	 uint32_t ifm;
 }GET_SUB_DEV_RESP_BODY;

 typedef struct
 {
	 GET_SUB_DEV_RESP_HEADER header;
	 GET_SUB_DEV_RESP_BODY body;
 }GET_SUB_DEV_RESP_STRUCT;

typedef struct
{
	unsigned char *key;
	unsigned char *ivv;
	char *version;

}AES_STRUCT;

extern QueueHandle_t uplink_q_handle;
extern char json_buf[JSON_BUF_SIZE];
extern char img_down_url[128];

bool uplink_protocol_init();
bool get_version_string(uint32_t version, char *str, int len);

int parse_sids(const char *start_ptr, const char *end_ptr, DEVICE_LIST *sdl);
unsigned char *get_guid(unsigned char *input, unsigned int ilen, unsigned char output[33]);

int8_t check_fota_resp_json_parse(char *indata, FW_UPDATE_RESP_STRUCT *outdata);
int8_t get_sub_devices_resp_json_parse(char *indata, GET_SUB_DEV_RESP_STRUCT *out_data);
//int8_t get_sids(const char *start_ptr, const char *end_ptr);
int8_t get_pui(const char *start_ptr, const char *end_ptr, FW_UPDATE_RESP_STRUCT *update_resp);

void get_resp_update_result_json(UPLINK_Q_MSG *q_ptr, unsigned char *aeskey, unsigned char *aesivv, char *aesvs);
void get_event_request_json(UPLINK_Q_MSG *q_ptr, unsigned char *aeskey, unsigned char *aesivv, char *aesvs);
void get_fw_update_request_json(UPLINK_Q_MSG *q_ptr, unsigned char *aeskey, unsigned char *aesivv, char *aesvs, LONG_ID96 *did);
void get_sensor_image_request_json(UPLINK_Q_MSG *q_ptr, unsigned char *aeskey, unsigned char *aesivv, char *aesvs);
void get_sub_devices_request_json(UPLINK_Q_MSG *q_ptr, unsigned char *aeskey, unsigned char *aesivv, char *aesvs);




#endif /* HTTP_UPLINK_TASK_H_ */
