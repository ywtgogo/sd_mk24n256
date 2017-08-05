#include <stdint.h>

//#include "A7139_CONFIG.h"

//#if CONFIG_DATA_RATE == 2

//const uint16_t CODE A7139Config[]=
//{
//	0x6221,             //[0x00] System Clock register
//	0x0A24,             //[0x01] PLL I register
//	0xB805,             //[0x02] PLL II register
//	0x0000,             //[0x03] PLL III register
//	0x0A20,             //[0x04] PLL IV register
//	0x0024,             //[0x05] PLL V register
//	0x0000,             //[0x06] PLL VI register
//	0x0011,             //[0x07] Crystal register
//	0x0000,             //PAGE A
//	0x0000,			//PAGE B
//	0x18D4,             //[0x0A] RX I register
//	0x7009,             //[0x0B] RX II register
//	0xC400,             //[0x0C] ADC register
//	0x0800,             //[0x0D] Pin Control register
//	0x4845,             //[0x0E] Calibration register
//	0x20C0              //[0x0F] Mode Control register
//};

//const uint16_t CODE A7139Config_PageA[]=
//{
//	0xF505,             //[0x08][Page0] TX I register
//	0x0000,             //[0x08][Page1] WOR I register
//	0x0000,            //[0x08][Page2] WOR II register
//	0x1107,             //[0x08][Page3] RF Current register
//	0x8170,             //[0x08][Page4] Power Manage register
//	0x0201,             //[0x08][Page5] AGC RSSI Threshold register
//	0x400F,             //[0x08][Page6] AGC Control register
//	0x2AC0,             //[0x08][Page7] AGC Control II register
//	0x0045,             //[0x08][Page8] GPIO register
//	0xD281,             //[0x08][Page9] CKO register
//	0x0004,             //[0x08][Page10] VCO Current register
//	0x0825,             //[0x08][Page11] Channel Group I register
//	0x0127,             //[0x08][Page12] Channel Group II register
//	0x003F,             //[0x08][Page13] FIFO register
//	0x155F,             //[0x08][Page14] CODE register
//	0x0000              //[0x08][Page15] WCAL register
//};
//
//const uint16_t CODE A7139Config_PageB[]=
//{
//	0x037F,             //[0x09][Page0] TX II register
//	0x8400,             //[0x09][Page1] IF I register
//	0x0000,             //[0x09][Page2] IF II register
//	0x0000,             //[0x09][Page3] ACK register
//	0x0000              //[0x09][Page4] ART register
//};
//
//#elif CONFIG_DATA_RATE == 10
//
//const uint16_t CODE A7139Config[]=		//868MHz, 10kbps (IFBW = 100KHz, Fdev = 37.5KHz), Crystal=12.8MHz
//{
//	0x1221,		//SYSTEM CLOCK register,
//	0x0A43,		//PLL1 register,
//	0xA805,		//PLL2 register,	866.001MHz
//	0x0000,		//PLL3 register,
//	0x1A20,		//PLL4 register,
//	0x0024,		//PLL5 register,
//	0x0000,		//PLL6 register,
//	0x0011,		//CRYSTAL register,
//	0x0000,		//PAGEA,
//	0x0000,		//PAGEB,
//	0x18D4,		//RX1 register, 	IFBW=100KHz
//	0x7009,		//RX2 register, 	by preamble
//	0x4000,		//ADC register,
//	0x0800,		//PIN CONTROL register,		Use Strobe CMD
//	0x4C45,		//CALIBRATION register,
//	0x20C0		//MODE CONTROL register, 	Use FIFO mode
//};
//
//const uint16_t CODE A7139Config_PageA[]=   //868MHz, 10kbps (IFBW = 100KHz, Fdev = 37.5KHz), Crystal=12.8MHz
//{
//	0xF706,		//TX1 register, 	Fdev = 37.5kHz
//	0x0000,		//WOR1 register,
//	0xF800,		//WOR2 register,
//	0x1107,		//RFI register, 	Enable Tx Ramp up/down
//	0x8970,		//PM register,		CST=1
//	0x0201,		//RTH register,
//	0x400F,		//AGC1 register,
//	0x6AC0,		//AGC2 register,
//	0x0040,		//GIO register, 	GIO1=Disable, GIO2=WTR
//	0xD181,		//CKO register
//	0x0004,		//VCB register,
//	0x0343,		//CHG1 register, 	860MHz
//	0x0044,		//CHG2 register, 	870MHz
//	0x003F,		//FIFO register, 	FEP=63+1=64bytes
//	0x1507,		//CODE register, 	Preamble=4bytes, ID=4bytes
//	0x0000		//WCAL register,
//};
//
//const uint16_t CODE A7139Config_PageB[]=   //868MHz, 10kbps (IFBW = 100KHz, Fdev = 37.5KHz), Crystal=12.8MHz
//{
//	0x0337,		//TX2 register,
//	0x8400,		//IF1 register, 	Enable Auto-IF, IF=200KHz
//	0x0000,		//IF2 register,
//	0x0000,		//ACK register,
//	0x0000		//ART register,
//};

//#elif CONFIG_DATA_RATE == 38

const uint16_t A7139Config[]=   //868MHz, 38.4kbps (IFBW = 100KHz, Fdev = 20.02KHz), Crystal=12.8MHz
{
	0x02C4,		//SYSTEM CLOCK register,
	0x0A43,		//PLL1 register,
	0x9405,     //0x93B8,		//PLL2 register,0xdd00,868.650Mhz	866.001MHz//0x9405 865.001 //0X93D1 864.991 //0x93DC 864.993 //0x93EB 864.996
	0x0000,		//PLL3 register,
	0x1020,		//PLL4 register,
	0x0048,		//PLL5 register,
	0x0000,		//PLL6 register,
	0x0013,		//CRYSTAL register,  ¸ßbyte±ØÐëÎª0
	0x0000,		//PAGEA,
	0x0000,		//PAGEB,
	0x18D4,		//RX1 register, 	IFBW=100KHz
	0x7009,  //0x5009,		//RX2 register, 	by preamble
	0xC400,//0x4400,		//ADC register,
	0x0800,		//PIN CONTROL register,		Use Strobe CMD
	0x4C45,		//CALIBRATION register,
	0x20C0,   //0x2040		//MODE CONTROL register, 	Use FIFO mode
};

const uint16_t A7139Config_PageA[]=   //868MHz, 38.4kbps (IFBW = 100KHz, Fdev = 20.02KHz), Crystal=12.8MHz
{
	0xF1CD,		//TX1 register, 	Fdev = 20.02kHz
	0x6038,		//WOR1 register,
	0xF800,		//WOR2 register,
	0x1907,		//RFI register, 	Enable Tx Ramp up/down
	0x1970,		//PM register,		CST=1
	0x0201,		//RTH register,
	0x400F,		//AGC1 register,
	0x6AC0,		//AGC2 register,
	0xc159,//0xC04D,		//GIO register, 	GIO1=Disable, GIO2=WTR
	0xE203,     //DELAY 1MS,//0xB203,//0xC783,2.5ms WSEL,10001,//0xB303, 2.0ms WSEL,01100 XTAL//0xBA83,		//CKO register
	0x00E4,		//VCO register,
	0x0343,		//CHG1 register, 	860MHz
	0x0044,		//CHG2 register, 	870MHz
	0x0014,		//FIFO register, 	FEP=20+1=21bytes
	0x1507,     //0x5501,//0x1507,		//CODE register, 	Preamble=4bytes, ID=4bytes
	0x0000		//WCAL register,
};

const uint16_t A7139Config_PageB[]=   //868MHz, 38.4kbps (IFBW = 100KHz, Fdev = 20.02KHz), Crystal=12.8MHz
{
		//TX2 register,
	0xD30F,//10DB
	//0xD017,//11.7DB
	//0xD01E,//11.8DB
	//0xD01F,//12.5DB
	0x8312,		//IF1 register, 	Enable Auto-IF, IF=200KHz
	0x0000,		//IF2 register,
	0x0000,		//ACK register,
	0x0000		//ART register,
};

//#elif CONFIG_DATA_RATE == 50//DR=50,IFBW=50
//
//const uint16_t CODE A7139Config[]=
//{
//	0x0023,             //[0x00] System Clock register
//	0x0A24,             //[0x01] PLL I register
//	0xB805,             //[0x02] PLL II register
//	0x0000,             //[0x03] PLL III register
//	0x0A20,             //[0x04] PLL IV register
//	0x0024,             //[0x05] PLL V register
//	0x0000,             //[0x06] PLL VI register
//	0x0001,             //[0x07] Crystal register
//	0x0000,		     //[0x08] PAGE A
//	0x0000,	          //[0x07] PAGE B
//	0x18D0,             //[0x0A] RX I register
//	0x7009,             //[0x0B] RX II register
//	0x4400,             //[0x0C] ADC register
//	0x0800,             //[0x0D] Pin Control register
//	0x4845,             //[0x0E] Calibration register
//	0x20C0              //[0x0F] Mode Control register
//};
//
//const uint16_t CODE A7139Config_PageA[]=
//{
//	0xF606,             //[0x08][Page0] TX I register
//	0x0000,             //[0x08][Page1] WOR I register
//	0xF800,             //[0x08][Page2] WOR II register
//	0x1107,             //[0x08][Page3] RF Current register
//	0x0170,             //[0x08][Page4] Power Manage register
//	0x0201,             //[0x08][Page5] AGC RSSI Threshold register
//	0x400F,             //[0x08][Page6] AGC Control register
//	0x2AC0,             //[0x08][Page7] AGC Control II register
//	0x0045,             //[0x08][Page8] GPIO register
//	0xD281,             //[0x08][Page9] CKO register
//	0x0004,             //[0x08][Page10] VCO Current register
//	0x0825,             //[0x08][Page11] Channel Group I register
//	0x0127,             //[0x08][Page12] Channel Group II register
//	0x003F,             //[0x08][Page13] FIFO register
//	0x155F,             //[0x08][Page14] CODE register
//	0x0000              //[0x08][Page15] WCAL register
//};
//
//const uint16_t CODE A7139Config_PageB[]=
//{
//	0x037F,             //[0x09][Page0] TX II register
//	0x8200,             //[0x09][Page1] IF I register
//	0x0000,             //[0x09][Page2] IF II register
//	0x0000,             //[0x09][Page3] ACK register
//	0x0000              //[0x09][Page4] ART register
//};
//
//#elif CONFIG_DATA_RATE == 100//DR=100,IFBW=100
//
//const uint16_t CODE A7139Config[]=
//{
//	0x0021,             //[0x00] System Clock register
//	0x0A24,             //[0x01] PLL I register
//	0xB805,             //[0x02] PLL II register
//	0x0000,             //[0x03] PLL III register
//	0x0A20,             //[0x04] PLL IV register
//	0x0024,             //[0x05] PLL V register
//	0x0000,             //[0x06] PLL VI register
//	0x0001,             //[0x07] Crystal register
//	0x0000,		     //PAGE A
//	0x0000,		     //PAGE B
//	0x18D4,             //[0x0A] RX I register
//	0x7009,             //[0x0B] RX II register
//	0x4400,             //[0x0C] ADC register
//	0x0800,             //[0x0D] Pin Control register
//	0x4845,             //[0x0E] Calibration register
//	0x20C0              //[0x0F] Mode Control register
//};
//
//const uint16_t CODE A7139Config_PageA[]=
//{
//	0xF706,             //[0x08][Page0] TX I register
//	0x0000,             //[0x08][Page1] WOR I register
//	0x0000,             //[0x08][Page2] WOR II register
//	0x1107,             //[0x08][Page3] RF Current register
//	0x0170,             //[0x08][Page4] Power Manage register
//	0x0201,             //[0x08][Page5] AGC RSSI Threshold register
//	0x400F,             //[0x08][Page6] AGC Control register
//	0x2AC0,             //[0x08][Page7] AGC Control II register
//	0x0045,             //[0x08][Page8] GPIO register
//	0xD281,             //[0x08][Page9] CKO register
//	0x0004,             //[0x08][Page10] VCO Current register
//	0x0825,             //[0x08][Page11] Channel Group I register
//	0x0127,             //[0x08][Page12] Channel Group II register
//	0x003F,             //[0x08][Page13] FIFO register
//	0x155F,             //[0x08][Page14] CODE register
//	0x0000              //[0x08][Page15] WCAL register
//};
//
//const uint16_t CODE A7139Config_PageB[]=
//{
//	0x037F,             //[0x09][Page0] TX II register
//	0x8400,             //[0x09][Page1] IF I register
//	0x0000,             //[0x09][Page2] IF II register
//	0x0000,             //[0x09][Page3] ACK register
//	0x0000              //[0x09][Page4] ART register
//};
//
//#elif CONFIG_DATA_RATE == 150//DR=150,IFBW=150
//
//const uint16_t CODE A7139Config[]=
//{
//	0x007B,             //[0x00] System Clock register
//	0x0A24,             //[0x01] PLL I register
//	0xB805,             //[0x02] PLL II register
//	0x0000,             //[0x03] PLL III register
//	0x0A20,             //[0x04] PLL IV register
//	0x0024,             //[0x05] PLL V register
//	0x0000,             //[0x06] PLL VI register
//	0x0003,             //[0x07] Crystal register
//	0x0000,
//	0x0000,
//	0x18D8,             //[0x0A] RX I register
//	0x7009,             //[0x0B] RX II register
//	0x4400,             //[0x0C] ADC register
//	0x0800,             //[0x0D] Pin Control register
//	0x4845,             //[0x0E] Calibration register
//	0x20C0              //[0x0F] Mode Control register
//};
//
//const uint16_t CODE A7139Config_PageA[]=
//{
//	0xF709,             //[0x08][Page0] TX I register
//	0x0000,             //[0x08][Page1] WOR I register
//	0x0000,             //[0x08][Page2] WOR II register
//	0x1107,             //[0x08][Page3] RF Current register
//	0x0170,             //[0x08][Page4] Power Manage register
//	0x0201,             //[0x08][Page5] AGC RSSI Threshold register
//	0x400F,             //[0x08][Page6] AGC Control register
//	0x2AC0,             //[0x08][Page7] AGC Control II register
//	0x4045,             //[0x08][Page8] GPIO register
//	0xD281,             //[0x08][Page9] CKO register
//	0x0004,             //[0x08][Page10] VCO Current register
//	0x0825,             //[0x08][Page11] Channel Group I register
//	0x0127,             //[0x08][Page12] Channel Group II register
//	0x003F,             //[0x08][Page13] FIFO register
//	0x155F,             //[0x08][Page14] CODE register
//	0x0000,             //[0x08][Page15] WCAL register
//};
//
//const uint16_t CODE A7139Config_PageB[]=
//{
//	0x037F,             //[0x09][Page0] TX II register
//	0x8600,             //[0x09][Page1] IF I register
//	0x0000,             //[0x09][Page2] IF II register
//	0x0000,             //[0x09][Page3] ACK register
//	0x0000              //[0x09][Page4] ART register
//};
//
//#endif
