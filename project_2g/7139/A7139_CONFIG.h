/*********************************************/
#ifndef __A7139_CONGFIG_H__
#define __A7139_CONGFIG_H__
/*********************************************/
#ifndef CONFIG_DATA_RATE
#define CONFIG_DATA_RATE 38
#endif
#define PLATEFORM_C51    0
/*********************************************/
#if  PLATEFORM_C51 == 1
	#define DATA  data
	#define IDATA idata
	#define CODE  code
	#define XDATA xdata
#else
	#define DATA
	#define IDATA
	#define CODE
	#define XDATA
#endif
/********************************************/
extern const uint16_t CODE A7139Config[];
extern const uint16_t CODE A7139Config_PageA[];
extern const uint16_t CODE A7139Config_PageB[];
#endif
/*********************************************/
