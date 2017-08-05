#ifndef _FOTA_H_
#define _FOTA_H_

#include "fsl_pflash_freertos.h"

#define FOTA_CHECK_POLLING				(300000 / portTICK_PERIOD_MS)

#define FOTA_RQ_LENGTH 					3

#define FOTA_PRAM_ADDR					0x3E000

#define FOTA_IMG_CELL_DOWN_LEN			0x1000

#define FOTA_HBB_IMG0_ADDR_START		0x100000
#define FOTA_HBB_IMG1_ADDR_START		0x140000
#define FOTA_HBB_IMG2_ADDR_START		0x180000
#define FOTA_BRIDGE_ADDR_START		    0x1C0000
#define FOTA_HBB_IMGN_OFFSET			0x040000
#define FOTA_HBB_IMG_SWVER_OFFSET		(32+32)

#define FOTA_SENSOR_IMG_ADDR_START		0x0
#define FOTA_SENSOR_IMG_ADDR_END		0x100000
#define FOTA_SENSOR_IMG_CELL_LEN		0x4000

#define FOTA_SENSOR_IMG_BLOCK_NUM 8

#define FOTA_SHA1_4KHEAD_OFFSET			0
#define FOTA_HBB_SHA1_OFFSET_START		(32+32+32+64+4+4)
#define FOTA_SENSOR_SHA1_OFFSET_START	(32+32+32+64+4+4)
#define FOTA_SENSOR_SHA1_OFFSET_SECOND	(32+32+32+64+4+4+1272+32+4+4)
#define CELL_SHA1_UNIT					20U

#define HBB_SENSOR_LIST_CONFIG			0x3D000
//#define HBB_PPP_GPRS_CONFIG				0x3C000
#define HBB_INFO_CONFIG					0x3E000

extern uint8_t     		fota_data_read[4096];
extern QueueHandle_t 	fota_rq_handle;
extern uint32_t 		bat_volt_low_flag;

enum FOTA_EC
{
	DeviceTypeErr = 1,
};

typedef enum{
	New = 1,
	Bad,
	Abort,
	Done,
}FOTA_Status;

enum FOTA_CMD
{
	FW_UPD_RESP, //fw update response
	FW_IMG_BLK,  //fw image BLK
	FW_BRIDGE_UPD_RESP,
	FW_BRIDGE_IMG_BLK,
	FW_HBB_UPD_RESP,
	FW_HBB_IMG_BLK,
	FW_TIMING_SENSOR_FOTA,
	FW_TIMING_CHECK_FOTA,
	FW_SMS_CHECK_FOTA,
	FW_TIMEOUT_FOTA,
	FW_IMG_DOWNLOAD_FAIL,
	FW_SOS_INCOMING_CALL,
};

typedef struct
{
	uint32_t 	rid;    	//rule id
	uint32_t 	t;       	//update type  ?
	uint32_t 	dt;     	//device type
	uint32_t 	hdv;    	//hardware version
	uint32_t 	fwpv;   	//fwPreviousVersion ?
	uint32_t 	fwuv;	 	//fwUpdateVersion
	uint32_t 	fws;	 	//the size of the FW/parameter image, in bytes.
	char 		*url;		//url of img storage in server.
	char 		*key;		//key : use for signature
	uint32_t 	fwblks;	 	//the numbers of full img block
	uint32_t 	ifm;	 	// if more 1: have 0:none     have other update
}FW_UPD_REQ_STRUCT;

typedef struct
{
	uint32_t 	img_blk_len;  	//img blockN len。
	//char 		*key;			//key:  use for signature
	//uint32_t 	rid;		  	// rule id : receive from server
	///char 	*url;			//url of img storage in server.
	//char 		*sha1;			//verify completeness of img
}FW_IMG_BLK_STRUCT;
typedef struct
{
	int fail_code;
}FW_FAIL_INFO_STRUCT;

typedef struct
{
	enum FOTA_CMD cmd;
	union
	{
		FW_UPD_REQ_STRUCT 		fw_upd_req;
		FW_IMG_BLK_STRUCT 		fw_blk_down;
		FW_FAIL_INFO_STRUCT		fw_fail_info;
	};
}FOTA_RQ_MSG;

#ifdef BRIDGE
typedef struct
{
	uint32_t	device_type;
	uint32_t 	fw_sw;
	uint32_t 	fw_mitt;
	uint32_t 	fw_block_num;
	uint32_t	fw_block_down;
	uint32_t	fw_block_try_cnt;
	uint32_t	fw_update_seq;
	uint32_t	fw_sflash_addr;
}PLC_FOTA_INFO;

extern PLC_FOTA_INFO fota_info;

#endif

typedef struct
{
	uint32_t dt;
	uint32_t hwver;
	uint32_t swver;
	uint32_t addr;
	uint32_t length;
}SENSOR_MSG_fota_update_header;

typedef struct
{
	uint32_t	addr;
	uint32_t	length;
	uint8_t 	block_sha1[1272];
	uint8_t		img_sha1[32];
}SENSOR_SUB_NODE;

typedef struct
{
	uint8_t     	head_sha1[32];
	uint32_t		dev_type;
	uint32_t		hw_ver;
	uint8_t			hw_info[24];
	uint32_t		sw_ver;
	uint8_t			sw_info[92];
	SENSOR_SUB_NODE	sub_node[2];
	uint8_t			pad[1312];
}FOTA_SENSOR_4KHEAD;

typedef struct
{
	uint8_t     	head_sha1[32];
	uint32_t		dev_type;
	uint32_t		hw_ver;
	uint8_t			hw_info[24];
	uint32_t		sw_ver;
	uint8_t			sw_info[92];
	/* up 160 */
	uint32_t		addr;
	uint32_t		length;
	uint8_t			block_sha1[1272];
	uint8_t			img_sha1[32];
	/* up 1312 */
	uint8_t			pad[2624];
}FOTA_HBB_4KHEAD;

/* for bootloader info */
enum
{
 FW_NOCHANGE = 0, //hb fw没有更改
 FW_UPDATE_SUCCESS,//fota更新成功
 FW_UPDATE_FAILED_BLOCK_SHA1_ERR,	//fota 更新block sha1 error
 FW_UPDATE_FAILED_IMG_SHA1_ERR,		//,fota 更新block all sha1 pass,img sha1 error
 FW_ROLLBACK,//hb fw回退成功
};

typedef struct
{
	uint32_t fw_change;
	uint8_t  fw_pre[32];
	uint8_t  fw_now[32];
	uint8_t  fw_err_block; //记录 fota更新block sha1 error时，第几块block error
	uint8_t  pad[3]; 

}BOOT_RECORD;


extern FOTA_SENSOR_4KHEAD fw_header;

void update_fota_timing(uint32_t newval);
void fota_check_sensor_status(uint32_t device_type);
extern flash_rtos_handle_t pflash_handle;


#endif
