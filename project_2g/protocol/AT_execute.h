#ifndef		_AT_EXECUTE_H
#define 	_AT_EXECUTE_H


enum {
	ERR_RTC		=30,
	ERR_2G		=31,
	ERR_WIFI	=32,
	ERR_RF		=33,
	ERR_NOR		=34,
	ERR_BAT		=37,
	ERR_W_IMG	=38,
	ERR_W_AUDIO =39,
	ERR_FREQ	=40,
	ERR_N_IMG 	=41,
	ERR_N_AUDIO =42,
};

enum {
	HW_START,
	HW_NORDATAIMG,
	HW_NORDATAAUDIO,
	HW_PRODUCEINFO,
	HW_LONGID,
	HW_FREQ,
	HW_RESET,
};


typedef struct{
	uint8_t		Nor;
	uint8_t		Audio;
	uint8_t		Rtc;
	uint8_t		Red;
	uint8_t 	Green;
	uint8_t		Wifi;
	uint8_t		Tx868;
	uint8_t 	Bat;
}HWTEST_STATUS_t;


typedef struct{
	uint8_t	cmd;
	uint16_t blk_num;
	uint8_t *data;
	int	 b64_len;
}AT_RUN_MSG_t;

extern AT_RUN_MSG_t		AtRunMsg;
extern QueueHandle_t 	at_run_handle;

int SETPRODUCEINFO(unsigned char *buf);
int SETNORDATAIMG(unsigned char *buf, int addr, int b64_len);
int SETNORDATAAUDIO(unsigned char *buf, int addr, int b64_len);
int km_uart_test(void);

#endif
