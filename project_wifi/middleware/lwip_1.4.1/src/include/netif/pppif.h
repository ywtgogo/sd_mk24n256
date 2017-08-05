#ifndef PPPIF_H
#define PPPIF_H

#include "fsl_uart_freertos.h"

#ifndef TRUE
#define TRUE  1
#endif

#ifndef FALSE
#define FALSE 0
#endif

typedef unsigned char  u8_t;
typedef signed   char  int8;
typedef unsigned short u16_t;
typedef signed   short int16;
typedef unsigned int   u32_t;
typedef signed   int   int32;
typedef float          fp32;
typedef double         fp64;
typedef signed char 	s8_t;

typedef short int           INT16;
typedef char          CHAR;

#ifndef NULL
#define NULL ((void *)0)
#endif

#ifndef BOOL
#define BOOL  u32_t
#endif

/* user app defined */
#define MAX_GPRS_DIAL_TIMES  		3
#define GPRS_DIAL_TIMEOUT  			3000
#define MAX_AT_CMD_TIMES  			3
#define NOR_AT_CMD_TIMES  			2
#define MIN_AT_CMD_TIMES  			1
#define MAX_CALL_IN_TIMES  			9000
#define AT_CMD_TIMEOUT 				3000
#define AT_CMD_TIMEOUT_NOR			2
#define AT_CMD_TIMEOUT_MIN 			1
#define AT_CMD_ATH_TIMEOUT 			1000
#define AT_CIND_TIMEOUT 			2000
#define AT_APN_TIMEOUT 				3000
#define AT_CHECK_MODEM_TIMEOUT 		50000
#define SOS_CALL_TIMEOUT 			3600000
#define AT_CGDCONT_LEN 				18
#define AT_ATD_LEN 			    	3
#define AT_APN_OFFSET_LEN 			2
#define AT_APN_LEN 			    	32
#define AT_SOS_LEN 			    	32
#define MAX_MARGIN_SMS_NUMB_ME 		6
#define MAX_SMS_NUMB 		    	50

#define MAX_UART_REC_TIMEOUT 		140

#define WAIT_RETURN 	       	 	1
#define NOT_WAIT 	           	 	0
#define AT_REPEAT_MAX 				3
#define AT_REPEAT_NOR		   	 	2
#define AT_REPEAT_MIN 		   	 	1

#define AT_TIMEOUT_MAX 		   	 	1500
#define AT_TIMEOUT_NOR		    	1000
#define AT_TIMEOUT_MIN 		    	500

#define AT_DELET_SMS_TIMEOUT_MAX 	60000
#define AT_GPRS_CALLTIMEOUT_MAX 	150000

#define AT_TIMEOUT_MIPDNS_MAX 		10000
#define AT_TIMEOUT_MIPOPEN_MAX 		10000
#define AT_TIMEOUT_MIPCLOSE_MAX 	11000

#define AT_CLCC_TIMEOUT_MAX 		300
#define AT_CLCC_TIMEOUT_NOR 		200
#define AT_CLCC_TIMEOUT_MIN 		100

#define AT_CGREG_TIMEOUT_MAX 		3000
#define AT_CGREG_TIMEOUT_NOR 		2000
#define AT_CGREG_TIMEOUT_MIN 		1000
#define AT_BUF_LEN_MIN   		 	0

#ifndef PPP_MAXMRU 
   #define PPP_MAXMRU 1500
#endif

#define SEQ_001             0x0000
#define SEQ_002             0x0001
#define SEQ_003             0x0002
#define SEQ_004             0x0003
#define SEQ_005             0x0004
#define SEQ_006             0x0005
#define SEQ_007             0x0006
#define SEQ_008             0x0007
#define SEQ_009             0x0008
#define SEQ_010             0x0009
#define SEQ_011             0x000A
#define SEQ_012             0x000B
#define SEQ_013             0x000C
#define SEQ_014             0x000D
#define SEQ_015             0x000E
#define SEQ_016             0x000F
#define SEQ_017             0x0010
#define SEQ_018             0x0011
#define SEQ_019             0x0012

#define SEQ_END             0x00FF
#define SEQ_NULL            0xFFFF

#define NEXT_SEQ_001             0x0000
#define NEXT_SEQ_002             0x0001
#define NEXT_SEQ_003             0x0002
#define NEXT_SEQ_004             0x0003
#define NEXT_SEQ_005             0x0004
#define NEXT_SEQ_006             0x0005
#define NEXT_SEQ_007             0x0006
#define NEXT_SEQ_008             0x0007
#define NEXT_SEQ_009             0x0008
#define NEXT_SEQ_010             0x0009
#define NEXT_SEQ_011             0x000A
#define NEXT_SEQ_012             0x000B
#define NEXT_SEQ_013             0x000C
#define NEXT_SEQ_014             0x000D
#define NEXT_SEQ_015             0x000E
#define NEXT_SEQ_016             0x000F
#define NEXT_SEQ_017             0x0010
#define NEXT_SEQ_018             0x0011
#define NEXT_SEQ_019             0x0012

#define NEXT_SEQ_END             0x00FF
#define NEXT_SEQ_NULL            0xFFFF

#define SEQ_EX_OK          		0
#define SEQ_EX_ERR         		1
#define SEQ_EX_END        		2
#define SEQ_EX_REPEAT     		3
#define SEQ_EX_ERR_HTTP    		4
#define SEQ_EX_APN_UPDATE 		5
#define SEQ_EX_SOS_UPDATE  		6
#define SEQ_EX_SOS_CALL_UP 		7
#define SEQ_EX_SMS_UPDATE_RING  12
#define SEQ_EX_NEXT_SEQ3       	13
#define SEQ_EX_NEXT_SEQ4       	14
#define SEQ_EX_NEXT_SEQ5       	15
#define SEQ_EX_PPP_DOWN         16

#define SEQ_EX_INIT_END    		30
#define SEQ_BUSY           		40

#define SEND_AT(handle, str, len)                  	UART_RTOS_Send(handle, str, len)
#define GET_AT(handle, p ,y)                        UART_RTOS_Receive_NoBlocking_Safe(handle, p, 1)//UART_RTOS_Receive(handle, p, 1, y)
#define GET_USER_DATA(handle, p)

typedef enum
{
   AT_RET_OK = 0,
   AT_RET_FAIL,
   AT_RET_END,
   AT_RET_REPEAT = 3,
   AT_RET_FAIL_HTTP,
   AT_RET_APN_UPDATE = 5,
   AT_RET_SOS_UPDATE,
   AT_RET_SOS_CALL_UP,
   AT_RET_SOS_CALL_DOWN = 8,
   AT_RET_CALL_ING,
   AT_RET_CALL_DOWN = 10,
   AT_RET_CALL_DOWN_TIMER_OUT,
   AT_RET_SMS_UPDATE_RING = 12,
   AT_RET_NEXT_SEQ3 = 13,
   AT_RET_NEXT_SEQ4 = 14,
   AT_RET_NEXT_SEQ5 = 15,
   AT_RET_PPP_DOWN = 16,
   AT_RET_SMS_UPDATE = 17,
   AT_RET_MIPPUSH_ERR = 18,
   AT_RET_FAIL_OTHERS
}at_ret;

typedef enum
{
    PPP_DEAD = 0,
    PPP_ESTABLISHED,
    PPP_NEGOTIATE,
    UDP_ESTABLISHED,
    TCP_CONNECTING,
    TCP_ESTABLISHED,
    TCP_CLOSING,
    TCPIP_STATE_UNKNOWN, /*ppp expected unknown err, such as no response from Modem ,we must reset System*/
}LWIP_LINK_STATE;

typedef enum
{
   AT_CMD_AT = 0,
   AT_CMD_CCID,
   AT_CMD_CSQ,
   AT_CMD_CGREG = 3,
   AT_CMD_CREG,
   AT_CMD_INIT_GPRS,
   AT_CMD_DIAL_GPRS = 6,
   AT_CMD_CPMS,
   AT_CMD_CMGF = 8,
   AT_CMD_CMGL,
   AT_CMD_CMGR,  
   AT_CMD_APN = 11,  
   AT_CMD_SOS,  
   AT_CMD_ATD = 13,
   AT_CMD_CHECK_MODEM_STAT,
   AT_CMD_WAIT_SOS_END = 15,
   AT_CMD_WAIT_SOS_END_CIND,
   AT_CMD_WAIT_CALL_IN_END,
   AT_CMD_WAIT_CALL_IN_ANSWER = 18,
   AT_CMD_CIND,
   AT_CMD_CLCC = 20,
   AT_CMD_MIPCALL = 21,
   AT_CMD_MIPDNS = 22,
   AT_CMD_MIPSEND = 23,
   AT_CMD_MIPPUSH = 24,
   AT_CMD_MIPREAD = 25,
   AT_CMD_OTHER
}at_cmd_type;

typedef struct
{
    char *command; /*the AT Command we send to Modem*/
    char *result;       /*the result we expected*/
    u8_t timeout;      /*MAX time of second that we could waiting*/
    u8_t r_time;       /*MAX times that we could re_send to Modem*/
}AT_CMD;

typedef struct seq_arg_tbl{
    u32_t      arg[1];
}SEQ_ARG_TBL;


typedef struct seq_tbl
{
    u32_t      (* const func)(void);
    SEQ_ARG_TBL seq_arg;
    u32_t      next_states_no[5];
	u32_t      seq_status;
	u32_t      seq_repeat_count;
}SEQ_TBL;

/* for user app interface */

at_ret wait_at_cmd_return(uart_rtos_handle_t *handle, const CHAR *str, u32_t cmd_result_len, u32_t msecond, u32_t at_type, u32_t buf_len, u32_t *return_value, CHAR *buf, u32_t *ret_len);
at_ret modem_check_modem_status(uart_rtos_handle_t *handle);
at_ret modem_at_cmd_at(uart_rtos_handle_t *handle, u32_t repeat, u32_t timeout, u32_t wait_return);
at_ret modem_at_cmd_ata(uart_rtos_handle_t *handle, u32_t repeat, u32_t timeout, u32_t wait_return);
at_ret modem_at_cmd_at_c(uart_rtos_handle_t *handle, u32_t repeat, u32_t timeout, u32_t wait_return);
at_ret modem_at_cmd_cfun(uart_rtos_handle_t *handle, u32_t repeat, u32_t timeout, u32_t wait_return);
at_ret modem_at_cmd_ath(uart_rtos_handle_t *handle, u32_t repeat, u32_t timeout, u32_t wait_return);
at_ret modem_at_cmd_chup(uart_rtos_handle_t *handle, u32_t repeat, u32_t timeout, u32_t wait_return);
at_ret modem_at_cmd_cgatt(uart_rtos_handle_t *handle, u32_t repeat, u32_t timeout, u32_t wait_return);
at_ret modem_at_cmd_cgdcont(uart_rtos_handle_t *handle, u32_t repeat, u32_t timeout, u32_t wait_return);
at_ret modem_at_cmd_ccid(uart_rtos_handle_t *handle, u32_t repeat, u32_t timeout, u32_t wait_return);
at_ret modem_at_cmd_cgreg(uart_rtos_handle_t *handle, u32_t repeat, u32_t timeout, u32_t wait_return);
at_ret modem_at_cmd_cgatt_down(uart_rtos_handle_t *handle, u32_t repeat, u32_t timeout, u32_t wait_return);
at_ret modem_at_cmd_creg(uart_rtos_handle_t *handle, u32_t repeat, u32_t timeout, u32_t wait_return);
at_ret modem_at_cmd_csq(uart_rtos_handle_t *handle, u32_t repeat, u32_t timeout, u32_t wait_return);
at_ret modem_at_cmd_plus(uart_rtos_handle_t *handle, u32_t repeat, u32_t timeout, u32_t wait_return);
at_ret modem_at_cmd_ato(uart_rtos_handle_t *handle, u32_t repeat, u32_t timeout, u32_t wait_return);
at_ret modem_at_cmd_cind(uart_rtos_handle_t *handle, u32_t repeat, u32_t timeout, u32_t wait_return);
at_ret modem_at_cmd_vgr(uart_rtos_handle_t *handle, u32_t repeat, u32_t timeout, u32_t wait_return);
at_ret modem_at_cmd_cpms(uart_rtos_handle_t *handle, u32_t repeat, u32_t timeout, u32_t wait_return);
at_ret modem_at_cmd_cpms_incomingcall(uart_rtos_handle_t *handle, u32_t repeat, u32_t timeout, u32_t wait_return);
at_ret modem_at_cmd_cmgf(uart_rtos_handle_t *handle, u32_t repeat, u32_t timeout, u32_t wait_return);
at_ret modem_at_cmd_sos(uart_rtos_handle_t *handle, u32_t repeat, u32_t timeout, u32_t wait_return);
at_ret modem_at_cmd_gtset_iprfmt(uart_rtos_handle_t *handle, u32_t repeat, u32_t timeout, u32_t wait_return);
at_ret modem_at_cmd_cmgl(uart_rtos_handle_t *handle, u32_t repeat, u32_t timeout, u32_t wait_return);
at_ret modem_at_cmd_cmgr(uart_rtos_handle_t *handle, u32_t repeat, u32_t timeout, u32_t wait_return);
at_ret modem_at_cmd_mmicg(uart_rtos_handle_t *handle, u32_t repeat, u32_t timeout, u32_t wait_return);
at_ret modem_at_cmd_clvl(uart_rtos_handle_t *handle, u32_t repeat, u32_t timeout, u32_t wait_return);
at_ret modem_at_cmd_clcc(uart_rtos_handle_t *handle, u32_t repeat, u32_t timeout, u32_t wait_return);
at_ret modem_at_cmd_cmee(uart_rtos_handle_t *handle, u32_t repeat, u32_t timeout, u32_t wait_return);
at_ret modem_at_cmd_cnmi(uart_rtos_handle_t *handle, u32_t repeat, u32_t timeout, u32_t wait_return);
at_ret modem_at_cmd_cmgd_all(uart_rtos_handle_t *handle, u32_t repeat, u32_t timeout, u32_t wait_return);
at_ret modem_at_cmd_mipcall(uart_rtos_handle_t *handle, u32_t repeat, u32_t timeout, u32_t wait_return);
at_ret modem_at_cmd_mipopen(uart_rtos_handle_t *handle, u32_t repeat, u32_t timeout, u32_t wait_return);
at_ret modem_at_cmd_mipdns(uart_rtos_handle_t *handle, u32_t repeat, u32_t timeout, u32_t wait_return);
at_ret modem_at_cmd_mipsend(uart_rtos_handle_t *handle, u32_t repeat, u32_t len, u32_t timeout, u32_t wait_return);
at_ret modem_at_cmd_mipread(uart_rtos_handle_t *handle, u32_t repeat, u32_t timeout, u32_t wait_return, u32_t buf_len, CHAR *buf, u32_t *len_ret);
at_ret modem_at_cmd_mippush(uart_rtos_handle_t *handle, u32_t repeat, u32_t timeout, u32_t wait_return);
at_ret modem_at_cmd_miprtcp(uart_rtos_handle_t *handle, u32_t repeat, u32_t timeout, u32_t wait_return);

at_ret modem_sms_delete(u16_t sms_numb);
at_ret modem_get_sos_call_numb();
at_ret modem_sms_read_apn_pap(u16_t sms_numb);
at_ret modem_get_gprs_apn();
at_ret modem_wait_sos_call_end_clcc(uart_rtos_handle_t *handle);
at_ret modem_wait_incoming_call_end_clcc(uart_rtos_handle_t *handle);
at_ret modem_wait_incoming_call_answer_clcc(uart_rtos_handle_t *handle);
at_ret modem_dialog_gprs(uart_rtos_handle_t *handle);
u32_t gprs_modem_init();
u32_t gprs_set_mode_to_at();
u32_t gprs_check_sim();
u32_t gprs_get_gprs_sign();
u32_t gprs_get_gprs_status();
u32_t gprs_set_sms_storage();
u32_t gprs_call_gprs();
u32_t gprs_set_gprs_down();
u32_t gprs_get_gprs_sign_test();
LWIP_LINK_STATE get_lwip_link_state(void);
void set_lwip_link_state(LWIP_LINK_STATE state);

#endif




