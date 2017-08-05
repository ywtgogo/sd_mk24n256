#ifndef _LOG_TASK_H_
#define _LOG_TASK_H_

#include "uplink_protocol.h"
#include "uart_plc.h"

#define SECTOR_SIZE					0x1000

#define SYS_LOG_ADDR_START			0x200000
#define SYS_LOG_ADDR_END			0x202000
#define SYS_LOG_UNIT				128U
#define SYS_LOG_ENTRY_COUNT	        ((SYS_LOG_ADDR_END - SYS_LOG_ADDR_START)/SYS_LOG_UNIT)
#define SYS_LOG_SECTOR_COUNT	    ((SYS_LOG_ADDR_END - SYS_LOG_ADDR_START)/SECTOR_SIZE)
#define SYS_LOG_OFFSET_COUNT		(SECTOR_SIZE/SYS_LOG_UNIT)
#define SYS_LOG_BODY_OFF			16

#define DEFER_UPLINK_ADDR_START		0x380000
#define DEFER_UPLINK_ADDR_END		0x400000
#define DEFER_UPLINK_SECTOR_COUNT	((DEFER_UPLINK_ADDR_END-DEFER_UPLINK_ADDR_START)/SECTOR_SIZE)
#define DEFER_UPLINK_MSG_UNIT		128U
//#define DEFER_UPLINK_MSG
#ifdef BRIDGE
#define LOG_MSG_MAX_LEN				PLC_PACKAGE_LEN
#else
#define LOG_MSG_MAX_LEN				120
#endif
#define INFO_OFFSET (FLASH_SECTORSIZE-DEFER_UPLINK_MSG_UNIT)

#define ERROR_CODE "%d\1"
#define NOERROR_CODE "0\1"

#define HBLOG_INFO    "1\1"
#define HBLOG_ERROR   "2\1"
#define HBLOG_WARNING "3\1"

#ifndef PR_MODULE
#define PR_MODULE "sys\1"
#endif

#define pr_fmt(fmt) fmt

#define PR_ERROR(fmt, ...)		print_log(PR_MODULE ERROR_CODE HBLOG_ERROR pr_fmt(fmt), ##__VA_ARGS__)
#define PR_INFO(fmt, ...)		print_log(PR_MODULE NOERROR_CODE HBLOG_INFO pr_fmt(fmt), ##__VA_ARGS__)
#define PR_WARNING(fmt, ...)	print_log(PR_MODULE NOERROR_CODE HBLOG_WARNING pr_fmt(fmt), ##__VA_ARGS__)


enum LOG_type
{
	SYS_LOG,
	DUMP_LOG,
	DEFER_UPLINK,
	NEW_LINK_ON,
	PLC_DEFER_MSG,
	PLC_MSG_ON,
	UPLINK_LOG,
};

typedef struct
{
	uint64_t	timestamp;
	uint64_t	timestamp_end;
	char 	sys_log_body[LOG_MSG_MAX_LEN - 16];
} SYS_LOG_STRUCT;

typedef struct
{
	enum LOG_type type;
	union
	{
		SYS_LOG_STRUCT	 		sys_log;
		PLC_MSG			  plc_defer_msg;
		UPLINK_Q_MSG 		  defer_msg;


	};
} LOG_RQ_MSG;


#define SECTOR_MSG_MAX_COUNT	31U

typedef struct
{
	uint32_t	offset_sector_num_w;	//tail
	uint32_t	offset_sector_num_r;	//head
	uint32_t	info_num_w;
	uint32_t	info_num_r;
} SECTOR_INFO_STRUCT;

typedef struct
{
	uint32_t	head_log_offset;
	uint32_t	tail_log_offset;
} LOG_INFO_STRUCT;

typedef struct
{
	char msgid[33];
	uint64_t st;
	uint64_t et;
	unsigned char *log_ptr;
	unsigned int log_len;
	unsigned int batch_count;
	unsigned int batch_id;
}LOG_UPLOADING_STRUCT;

extern LOG_UPLOADING_STRUCT uploading_log;
extern SECTOR_INFO_STRUCT	sector_info;
extern LOG_INFO_STRUCT sys_log_info;
int save_defer_log(	UPLINK_Q_MSG *uplink_msg);
int plc_save_defer_log(PLC_MSG *plc_msg);
int defer_log_uplink();
int plc_defer_log_on();
int print_log(char *fmt_ptr, ...);
int test_print_log(int i, char *fmt_ptr, ...);
int uplink_log(void);



#endif

