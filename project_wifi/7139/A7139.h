#ifndef  __A7139_H__
#define  __A7139_H__

#include "fsl_common.h"

void   A7139_StrobeCmd(uint8_t cmd);
uint8_t  A7139_Init(uint16_t rfFreq);
uint8_t  A7139_SetCID(uint32_t id);
uint32_t A7139_GetCID();
uint8_t  A7139_SetDataRate(uint8_t datRate);
void  A7139_SetPackLen(uint32_t len);
void   A7139_SetPrmLength(uint32_t len);
uint8_t  A7139_WOR(uint16_t wor_method);
void   A7139_Cancel_WOR();
void   A7139_Enable_ARKS(uint8_t cycles, uint8_t ard);
void   A7139_SetGIO1_Func(uint8_t func);
void   A7139_SetGIO2_Func(uint8_t func);
uint16_t A7139_Get_VPOAK();
void   A7139_WriteFIFO(uint8_t *buf,uint32_t bufSize);
void   A7139_ReadFIFO(uint8_t *buf,uint32_t bufSize);
uint8_t A7139_WOR_ByPreamble();
uint8_t A7139_Direct();
void send_preamble_ms(int ms);
uint32_t RSSI_measurement(void);
bool SPI_verify(void);

#define SYSTEMCLOCK_REG 	     0x00
#define PLL1_REG 			0x01
#define PLL2_REG 			0x02
#define PLL3_REG 			0x03
#define PLL4_REG			0x04
#define PLL5_REG			0x05
#define PLL6_REG			0x06
#define CRYSTAL_REG			0x07
#define PAGEA_REG	  		0x08
#define PAGEB_REG			0x09
#define RX1_REG  			0x0A
#define RX2_REG  			0x0B
#define ADC_REG  			0x0C
#define PIN_REG 			0x0D
#define CALIBRATION_REG  	0x0E
#define MODE_REG  			0x0F

#define TX1_PAGEA             0x00
#define WOR1_PAGEA            0x01
#define WOR2_PAGEA            0x02
#define RFI_PAGEA             0x03
#define PM_PAGEA              0x04
#define RTH_PAGEA             0x05
#define AGC1_PAGEA            0x06
#define AGC2_PAGEA            0x07
#define GIO_PAGEA             0x08
#define CKO_PAGEA             0x09
#define VCB_PAGEA             0x0A
#define CHG1_PAGEA            0x0B
#define CHG2_PAGEA            0x0C
#define FIFO_PAGEA			0x0D
#define CODE_PAGEA			0x0E
#define WCAL_PAGEA			0x0F

#define TX2_PAGEB			0x00
#define IF1_PAGEB			0x01
#define IF2_PAGEB			0x02
#define ACK_PAGEB			0x03
#define ART_PAGEB			0x04


#define CMD_Reg_W			0x00	//000x,xxxx	control register write
#define CMD_Reg_R			0x80	//100x,xxxx	control register read
#define CMD_CID_W			0x20	//001x,xxxx	ID write
#define CMD_CID_R			0xA0	//101x,xxxx	ID Read
#define CMD_FIFO_W			0x40	//010x,xxxx	TX FIFO Write
#define CMD_FIFO_R			0xC0	//110x,xxxx	RX FIFO Read
#define CMD_RF_RST			0xFF	//x111,xxxx	RF reset
#define CMD_TFR			0x60	//0110,xxxx	TX FIFO address pointrt reset
#define CMD_RFR			0xE0	//1110,xxxx	RX FIFO address pointer reset

#define CMD_SLEEP			0x10	//0001,0000	SLEEP mode
#define CMD_IDLE			0x12	//0001,0010	IDLE mode
#define CMD_STBY			0x14	//0001,0100	Standby mode
#define CMD_PLL			0x16	//0001,0110	PLL mode
#define CMD_RX				0x18	//0001,1000	RX mode
#define CMD_TX				0x1A	//0001,1010	TX mode
#define CMD_DEEP_SLEEP		0x1F	//0001,1111    Deep Sleep mode(pull-high)

#define ERR_PARAM 			0x01
#define ERR_PLATFORM		0x02
#define ERR_UNK			0x03
#define ERR_CAL			0x04
#define ERR_TMO			0xFF		
#define ERR_RCOSC_CAL		0x04
#define OK_RCOSC_CAL		0x05
#define ERR_GET_RSSI		0x00

enum
{
	e_GIOS_WTR 		= 0x0,
	e_GIOS_FSYNC	        = 0x1,
	e_GIOS_PMDO		= 0x3,
    e_GIOS_SDO              = 0x6,
	e_GIOS_FPF		= 0xd,
};

#define WOR_BY_PREAMBLE_MASK 0x0030
#define WOR_BY_FSYNC_MASK 0x0010

static inline uint32_t
kxx_htonl(uint32_t n)
{
  return ((n & 0xff) << 24) |
    ((n & 0xff00) << 8) |
    ((n & 0xff0000UL) >> 8) |
    ((n & 0xff000000UL) >> 24);
}

#endif
