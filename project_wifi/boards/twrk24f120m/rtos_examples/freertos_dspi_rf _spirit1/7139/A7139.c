#include "A7139.h"
#include "fsl_dspi_freertos.h"
#include "SPI_A7139.h"
#include "A7139_CONFIG.h"
#include "task.h"
//#define SPI_DELAY_US 20

extern dspi_rtos_handle_t A7139_SPI_Handle;

uint8_t A7139_SetCID(uint32_t id);
uint32_t A7139_GetCID(void);
void A7139_StrobeCmd(uint8_t cmd);
uint8_t A7139_Init(uint16_t freq);

static uint8_t A7139_Cal(void);
static void   A7139_Config(uint16_t config2);
static void   A7139_WriteReg(uint8_t regAddr, uint16_t regVal);
static uint16_t A7139_ReadReg(uint8_t regAddr);
static uint16_t A7139_ReadPageA(uint8_t pagAddr);
static uint16_t A7139_ReadPageB(uint8_t pagAddr);
static void   A7139_WritePageA(uint8_t pagAddr, uint16_t pagVal);
static void   A7139_WritePageB(uint8_t pagAddr, uint16_t pagVal);
static void  A7139_RCOSC_Cal(void);

/*power on only*/
void A7139_POR()
{
	//power on only
    vTaskDelay(10);   			//for regulator settling time (power on only)

    A7139_StrobeCmd(CMD_RF_RST);  	//reset A7139 chip

    A7139_WritePageA(PM_PAGEA, A7139Config_PageA[PM_PAGEA] | 0x1000);   //STS=1
    vTaskDelay(2);

    A7139_StrobeCmd(CMD_RF_RST);              //RF reset
    A7139_WriteReg(PIN_REG, A7139Config[PIN_REG] | 0x0800);             //SCMDS=1
    A7139_WritePageA(PM_PAGEA, A7139Config_PageA[PM_PAGEA] | 0x0010);   //QDS=1
    A7139_StrobeCmd(CMD_SLEEP);               //entry sleep mode
    //Delay_us(600);                      //delay 600us for VDD_A shutdown, C load=0.1uF
    vTaskDelay(1);
    //A7139_StrobeCmd(CMD_DEEP_SLEEP);          //entry deep sleep mode,会使GIO2被拉高一下
    //Delay_us(200); 		//deep sleep
    vTaskDelay(1);
    vTaskDelay(2);
    A7139_StrobeCmd(CMD_STBY);    //wake up
    vTaskDelay(2); 	//wake up

    A7139_StrobeCmd(CMD_RF_RST);  	//reset A7139 chip
    A7139_WritePageA(PM_PAGEA, A7139Config_PageA[PM_PAGEA] | 0x1000);   //STS=1
    vTaskDelay(2);
}
//All these functions has been reformed to adapt VLPR mode. Do not use them in RUN mode.

/*
** ===================================================================
**     Method      :  A7139_WriteReg
*/
/*!
**     @brief
**         Write a Register on A7139
**     @param
**         regAddr         - Register address 0-15
**                           Do not directly write register 8 and 9.
**                           They are handled by dedicated function A7139_WritePageA and A7139_WritePageB
**     @param
**         regVal          - The value to be written into register.
**
**     @return
**                         - void
**
*/
/* ===================================================================*/
static void A7139_WriteReg(uint8_t regAddr, uint16_t regVal)
{
    uint8_t regval[2];
    regval[0] = (uint8_t)(regVal>>8);
    regval[1] = (uint8_t)regVal;
    
	regAddr |= CMD_Reg_W;
	SPI_RTOS_Write(&A7139_SPI_Handle, &regAddr, 1, true);
    SPI_RTOS_Write(&A7139_SPI_Handle, regval, 2, false);
}
/*
** ===================================================================
**     Method      :  A7139_ReadReg
*/
/*!
**     @brief
**         Read a Register on A7139
**     @param
**         regAddr         - Register address 0-15
**                           Do not directly read register 8 and 9.
**                           They are handled by dedicated function A7139_ReadPageA and A7139_ReadPageB
**
**     @return
**                         - The register value
*/
/* ===================================================================*/
static uint16_t A7139_ReadReg(uint8_t regAddr)
{
    uint8_t regValL[2];
	regAddr |= CMD_Reg_R;

    SPI_RTOS_Write(&A7139_SPI_Handle, &regAddr, 1, true);
    SPI_RTOS_Read(&A7139_SPI_Handle, regValL, 2);
    
	return (regValL[0]<<8)|regValL[1];
}
/*
** ===================================================================
**     Method      :  A7139_WritePageA
*/
/*!
**     @brief
**         Write Register 08h on A7139
**     @param
**         address         - Page number
**     @param
**         dataWord        - The value to be written into register.
**
**     @return
**                         - void
*/
/* ===================================================================*/
static void A7139_WritePageA(uint8_t address, uint16_t dataWord)
{
	uint16_t tmp;
	tmp = address;
	tmp = ((tmp << 12) | A7139Config[CRYSTAL_REG]);
	A7139_WriteReg(CRYSTAL_REG, tmp);
	A7139_WriteReg(PAGEA_REG, dataWord);
}
/*
** ===================================================================
**     Method      :  A7139_WritePageB
*/
/*!
**     @brief
**         Write Register 09h on A7139
**     @param
**         address         - Page number
**     @param
**         dataWord        - The value to be written into register.
**
**     @return
**                         - void
*/
/* ===================================================================*/
static void A7139_WritePageB(uint8_t address, uint16_t dataWord)
{
	uint16_t tmp;
	tmp = address;
	tmp = ((tmp << 7) | A7139Config[CRYSTAL_REG]);
	A7139_WriteReg(CRYSTAL_REG, tmp);
	A7139_WriteReg(PAGEB_REG, dataWord);
}
/*
** ===================================================================
**     Method      :  A7139_ReadPageA
*/
/*!
**     @brief
**         Read Register 08h on A7139
**     @param
**         address         - Page number
**
**     @return
**                         - The register value
*/
/* ===================================================================*/
static uint16_t A7139_ReadPageA(uint8_t address)
{
	uint16_t tmp;
	tmp = address;
	tmp = ((tmp << 12) | A7139Config[CRYSTAL_REG]);
	A7139_WriteReg(CRYSTAL_REG, tmp);
	tmp = A7139_ReadReg(PAGEA_REG);
	return tmp;
}
/*
** ===================================================================
**     Method      :  A7139_ReadPageB
*/
/*!
**     @brief
**         Read Register 09h on A7139
**     @param
**         address         - Page number
**
**     @return
**                         - The register value
*/
/* ===================================================================*/
static uint16_t A7139_ReadPageB(uint8_t address)
{
	uint16_t tmp;
	tmp = address;
	tmp = ((tmp << 7) | A7139Config[CRYSTAL_REG]);
	A7139_WriteReg(CRYSTAL_REG, tmp);
	tmp = A7139_ReadReg(PAGEB_REG);
	return tmp;
}
/*
** ===================================================================
**     Method      :  A7139_Config
*/
/*!
**     @brief
**         Write all the config data in A7139Config[]/A7139Config_PageA[]/A7139Config_PageB[] into A7139 registers
**     @param
**
**     @return
**                         - void
*/
/* ===================================================================*/
static void A7139_Config(uint16_t config2)
{
	uint8_t i;
	for(i=0; i<2; i++)
		A7139_WriteReg(i, A7139Config[i]);
//#ifdef PRODUCTION
//	if(*(uint16_t*)0x420!=0xFFFF)
//		A7139_WriteReg(i, *(uint16_t*)0x420);
//	else
//#endif
//		if (config2 != 0)
//			A7139_WriteReg(i, config2);
//		else
			A7139_WriteReg(i, A7139Config[2]);
	for(i=3; i<7; i++)
		A7139_WriteReg(i, A7139Config[i]);
	for(i=10; i<16; i++)
		A7139_WriteReg(i, A7139Config[i]);
	for(i=0; i<16; i++)
		A7139_WritePageA(i, A7139Config_PageA[i]);
	for(i=0; i<5; i++)
		A7139_WritePageB(i, A7139Config_PageB[i]);
}

bool SPI_verify()
{
	if(A7139Config[SYSTEMCLOCK_REG] == A7139_ReadReg(SYSTEMCLOCK_REG))
		return 0;
	else return 1;
}

/*
** ===================================================================
**     Method      :  A7139_Cal
*/
/*!
**     @brief
**         Calibrate A7139
**     @param
**
**     @return
**                         - 0 ERR_OK  Calibration done
**                         - 4 ERR_CAL Calibration Error
*/
/* ===================================================================*/
static uint8_t A7139_Cal(void)
{
    uint8_t fbcf;	//IF Filter
	uint8_t vbcf;			//VCO Current
	//uint8_t vcb, vccf;		//VCO Band
	uint16_t tmp;

	A7139_StrobeCmd(CMD_PLL);

    //IF calibration procedure @STB state
	A7139_WriteReg(MODE_REG, A7139Config[MODE_REG] | 0x0802);			//IF Filter & VCO Current Calibration
    do{
        tmp = A7139_ReadReg(MODE_REG);
    }while(tmp & 0x0802);

    //for check(IF Filter)
    tmp = A7139_ReadReg(CALIBRATION_REG);
//    fb = tmp & 0x0F;
//	fcd = (tmp>>11) & 0x1F;
    fbcf = (tmp>>4) & 0x01;
    if(fbcf)
    {
    	return ERR_CAL;
    }

	//for check(VCO Current)

    //RSSI Calibration procedure @STB state
	A7139_WriteReg(ADC_REG, 0x4C00);									//set ADC average=64
    A7139_WriteReg(MODE_REG, A7139Config[MODE_REG] | 0x1000);			//RSSI Calibration
    do{
        tmp = A7139_ReadReg(MODE_REG);
    }while(tmp & 0x1000);
	A7139_WriteReg(ADC_REG, A7139Config[ADC_REG]);


    //VCO calibration procedure @STB state
	A7139_WriteReg(PLL1_REG, A7139Config[PLL1_REG]);
//#ifdef PRODUCTION
//	if(*(uint16_t*)0x420!=0xFFFF)
//		A7139_WriteReg(PLL2_REG, *(uint16_t*)0x420);
//	else
//#endif

//		A7139_WriteReg(PLL2_REG, A7139Config[PLL2_REG]);

	A7139_WriteReg(MODE_REG, A7139Config[MODE_REG] | 0x0004);		//VCO Band Calibration
	do{
		tmp = A7139_ReadReg(MODE_REG);
	}while(tmp & 0x0004);

	//for check(VCO Band)
	tmp = A7139_ReadReg(CALIBRATION_REG);
//	vb = (tmp >>5) & 0x07;
	vbcf = (tmp >>8) & 0x01;
	if(vbcf)
	{
		return ERR_CAL;
	}

	A7139_StrobeCmd(CMD_STBY);
	return 0;
}

/*
** ===================================================================
**     Method      :  A7139_RCOSC_Cal
*/
/*!
**     @brief
**         Calibrate A7139's RC Oscillator
**     @param
**
**     @return
**                         - 5 OK_RCOSC_CAL  Calibration done
**                         - 4 ERR_RCOSC_CAL Calibration Error
*/
/* ===================================================================*/
static void A7139_RCOSC_Cal(void)
{
	uint16_t tmp;
	A7139_WritePageA(WOR2_PAGEA, A7139Config_PageA[WOR2_PAGEA] | 0x0010);

	while(1)
	{
		A7139_WritePageA(WCAL_PAGEA, A7139Config_PageA[WCAL_PAGEA] | 0x0001);
		do{
			tmp = A7139_ReadPageA(WCAL_PAGEA);
		}while(tmp & 0x0001);

		tmp = (A7139_ReadPageA(WCAL_PAGEA)& 0x03FF);
		tmp >>=1;
        //if((tmp > 183) && (tmp < 205))      //NUMLH[8:0]=194+-10 (PF8M=6.4M)
		if((tmp>232) &&(tmp<254))
		{
			break;
		}
	}
}

/*
** ===================================================================
**     Method      :  A7139_StrobeCmd
*/
/*!
**     @brief
**         Send a single byte command to A7139
**     @param
**         cmd             - The command byte to be sent.
**
**     @return
**                         - void
**
*/
/* ===================================================================*/
//uint32_t id;
void A7139_StrobeCmd(uint8_t cmd)
{
////    uint32_t id;
	//SCS_A7139_OUT(LOW);
	SPI_RTOS_Write(&A7139_SPI_Handle , &cmd, 1, false);
	//SCS_A7139_OUT(HIGH);


    
//    A7139_WriteReg(0x00, 0xC04D);
//    A7139_ReadReg(0x80);
    
//    A7139_SetCID(0x11223344);
//    id = A7139_GetCID();
//    printf("\r\n 0x%x \r\n", id);
}

/*
** ===================================================================
**     Method      :  A7139_Init
*/
/*!
**     @brief
**         Initialize A7139 by this sequence:
**         Init MCU SPI
**         Reset A7139
**         Set a preset CID
**         Set A7139 RF frequency
**         Calibrate A7139
**     @param
**         rfFreq          - The RF frequency to be set.
**         						Note: Set RF is temporarily disabled
**
**     @return
**                         - 0 ERR_OK  Calibration done, everything is OK
**                         - 4 ERR_CAL Calibration Error
**
*/
/* ===================================================================*/
uint8_t A7139_Init(uint16_t freq)
{
	//A7139_POR();
	A7139_StrobeCmd(CMD_RF_RST);	  //reset A7139 chip
	vTaskDelay(10);
	A7139_Config(freq);		        //config A7139 chip
	vTaskDelay(10);			  //for crystal stabilized
	return A7139_Cal();		  //IF and VCO calibration
}
/*
** ===================================================================
**     Method      :  A7139_SetCID
*/
/*!
**     @brief
**         Set a 4 byte CID to A7139
**     @param
**         id              - The CID to be set.
**
**     @return
**                         - always 0
**
*/
/* ===================================================================*/
uint8_t A7139_SetCID(uint32_t id)
{
    uint8_t buf[5];
    buf[0] = CMD_CID_W;
    //*(uint32_t*)(buf+1) = id;
    buf[1] = id>>24;
    buf[2] = id>>16;
    buf[3] = id>>8;
    buf[4] = id;
        
	//SCS_A7139_OUT(LOW);
    //SPI_RTOS_Write(&A7139_SPI_Handle, &regAddr, 1);
    SPI_RTOS_Write(&A7139_SPI_Handle, buf, sizeof(buf), false);
	//SCS_A7139_OUT(HIGH);
	return 0;
}


uint32_t A7139_GetCID(void)
{
    uint8_t cmd_cid_r = CMD_CID_R;
	uint8_t id[4];

	SPI_RTOS_Write(&A7139_SPI_Handle, &cmd_cid_r, sizeof(uint8_t), true);
    SPI_RTOS_Read(&A7139_SPI_Handle, id, 4);

	return *(uint32_t*)id;
}

uint32_t ID;
uint16_t ST;
void SPI_A7139_TEST(void)
{
    A7139_Init(0);
    A7139_SetCID(0x33441122);
    ID = A7139_GetCID();
    ST = A7139_ReadReg(SYSTEMCLOCK_REG);
}

void A7139_SetPackLen(uint32_t len)
{
	uint16_t pagVal;
	A7139_StrobeCmd(CMD_STBY);
	//FEP[7:0]
	pagVal = (A7139Config_PageA[FIFO_PAGEA]&0xFF00) | ((len-1)&0xFF);
	if(len>64)
	{
		pagVal = (pagVal&0x3FFF) | 0xC000;
	}
	else
	{
		pagVal = (pagVal&0x3FFF) | 0x0000;
	}
	A7139_WritePageA(FIFO_PAGEA,pagVal);
	//FEP[13:8]
	pagVal = (A7139Config_PageA[VCB_PAGEA]&0xC0FF) | ((len-1)&0x3F00);
	A7139_WritePageA(VCB_PAGEA,pagVal);
}

void A7139_SetPrmLength(uint32_t len)
{
	switch(len)
	{
	case 64:
		A7139_WritePageA(CODE_PAGEA,A7139Config_PageA[CODE_PAGEA] | 0x8003);
		break;
	case 4:
	default:
		A7139_WritePageA(CODE_PAGEA,(A7139Config_PageA[CODE_PAGEA] & 0x7FFC)|0x0003);
		break;
	}
}

void A7139_WriteFIFO(uint8_t *buf,uint32_t bufSize)
{
    uint8_t cmd_fifo_w = CMD_FIFO_W;
	//SCS_A7139_OUT(LOW);
    SPI_RTOS_Write(&A7139_SPI_Handle, &cmd_fifo_w, 1, true);
    SPI_RTOS_Write(&A7139_SPI_Handle, buf, bufSize, false);
	//SCS_A7139_OUT(HIGH);	
}

void A7139_ReadFIFO(uint8_t *buf,uint32_t bufSize)
{
    uint8_t cmd_fifo_r = CMD_FIFO_R;
	//SCS_A7139_OUT(LOW);
    SPI_RTOS_Write(&A7139_SPI_Handle, &cmd_fifo_r, 1, true);
    //SPI0->C2 &= (~SPI_C2_BIDIROE_MASK);
    SPI_RTOS_Read(&A7139_SPI_Handle, buf, bufSize);    
	//SPI0->C2 |= SPI_C2_BIDIROE_MASK;
	//SCS_A7139_OUT(HIGH);							 
}

void A7139_SetGIO1_Func(uint8_t func)
{
	//GIO1=func
	A7139_WritePageA(GIO_PAGEA, A7139Config_PageA[GIO_PAGEA] | ((func&0xF)<<2));
}

void A7139_SetGIO2_Func(uint8_t func)
{
	//GIO2=func
	A7139_WritePageA(GIO_PAGEA, A7139Config_PageA[GIO_PAGEA] | ((func&0xF)<<8));
}

void A7139_Enable_ARKS(uint8_t cycles, uint8_t ard)
{
	uint16_t ack = 0x0021; //VKM=by event, ARTMS=fixed interval, EARKS=Enable
	if(cycles>15)
		cycles=15;

	A7139_WritePageB(ACK_PAGEB, ack|(cycles<<1));
	//ARD=??ms
	A7139_WritePageB(ART_PAGEB, (uint16_t)ard);
}

uint16_t A7139_Get_VPOAK()
{
	uint16_t ret_val = A7139_ReadPageB(ACK_PAGEB);
	return (ret_val>>5)&0x0001;
}

uint8_t A7139_WOR(uint16_t wor_method)
{
	A7139_StrobeCmd(CMD_STBY);
	A7139_SetGIO2_Func(e_GIOS_FSYNC);	//GIO1=DISABLE, GIO2=PMDO

	//Real WOR Active Period = (WOR_AC[5:0]+1) x 244us,XTAL and Regulator Settling Time
	//Note : Be aware that Xtal settling time requirement includes initial tolerance, 
	//       temperature drift, aging and crystal loading.
//	A7139_WritePageA(WOR1_PAGEA, 0x6040);	//setup WOR Sleep time and Rx time
	//RC Oscillator Calibration
	A7139_RCOSC_Cal();
	A7139_WritePageA(WOR2_PAGEA, A7139Config_PageA[WOR2_PAGEA] | wor_method);	//enable RC OSC & WOR by preamble

	A7139_WriteReg(MODE_REG, A7139Config[MODE_REG] | 0x0200);				//WORE=1 to enable WOR function

	//while(GIO1==0);		//Stay in WOR mode until receiving preamble(preamble ok)
	return 0;
}

void A7139_Cancel_WOR()
{
	//EInt_GIO2_Disable(EInt_GIO2_DeviceData);
	A7139_WriteReg(MODE_REG, A7139Config[MODE_REG] & ~0x0200);
}


#define INFS_OFF 0
#define INFS_ON  1

void A7139_INFS_Mode(int key)
{
	if (INFS_ON == key)
	{
		A7139_StrobeCmd(CMD_PLL);
		A7139_WritePageA(VCB_PAGEA, (A7139Config_PageA[VCB_PAGEA] & 0xC0FF));
		A7139_WritePageA(FIFO_PAGEA, (A7139Config_PageA[FIFO_PAGEA] & 0x3F00) | 0xC014);	//0xc01F
		A7139_WriteReg(PIN_REG,A7139Config[PIN_REG] | 0x0200);
		A7139_SetCID(0x55555555);
		return;
	}
	if (INFS_OFF == key)
	{
		A7139_SetCID(0x78877887);
		A7139_WriteReg(PIN_REG,A7139Config[PIN_REG]);
		vTaskDelay(14);
		A7139_WritePageA(VCB_PAGEA, (A7139Config_PageA[VCB_PAGEA]));
		A7139_WritePageA(FIFO_PAGEA, A7139Config_PageA[FIFO_PAGEA]);
//		A7139_StrobeCmd(CMD_PLL);
	}
}

void send_preamble_ms(int ms)
{
	uint8_t vpal[64];
	for(int i=0;i<64;i++)
	vpal[i]=0x55;
	A7139_INFS_Mode(INFS_ON);				//set INFS
	A7139_StrobeCmd(CMD_TFR);
	A7139_WriteFIFO(vpal, 64);
	A7139_StrobeCmd(CMD_TX);
	vTaskDelay(ms);
	A7139_INFS_Mode(INFS_OFF);
}

uint8_t A7139_WOR_ByPreamble(void)
{
	A7139_StrobeCmd(CMD_STBY);
	//A7139_SetGIO2_Func(e_GIOS_PMDO);	//GIO1=DISABLE, GIO2=PMDO
	A7139_SetGIO2_Func(e_GIOS_FSYNC);	//GIO1=DISABLE, GIO2=PMDO
	//Real WOR Active Period = (WOR_AC[5:0]+1) x 244us,XTAL and Regulator Settling Time
	//Note : Be aware that Xtal settling time requirement includes initial tolerance,
	//       temperature drift, aging and crystal loading.
	A7139_WritePageA(WOR1_PAGEA, 0x4035);	//setup WOR Sleep time and Rx time
	//RC Oscillator Calibration
	A7139_RCOSC_Cal();
	A7139_WritePageA(WOR2_PAGEA, A7139Config_PageA[WOR2_PAGEA] | 0x0030);	//enable RC OSC & WOR by preamble

	A7139_WriteReg(MODE_REG, A7139Config[MODE_REG] | 0x0200);				//WORE=1 to enable WOR function
	 A7139_StrobeCmd(CMD_SLEEP);               //entry sleep mode
	//while(GIO1==0);		//Stay in WOR mode until receiving preamble(preamble ok)
	return 0;
}

uint8_t A7139_Direct()
{

//	A7139_WritePageA(GIO_PAGEA, 0x025D);    //GIO1=TRXD, GIO2=TMEO
//	A7139_WritePageA(CKO_PAGEA, 0xD283);    //CKO=DCK
	A7139_StrobeCmd(CMD_STBY);  //Direct mode, TX
	//A7139_WritePageB(TX2_PAGEB, 0xD30B);  //2.2dbm
	A7139_WritePageB(TX2_PAGEB, 0xD31B);  //4.2dbm
	A7139_WriteReg(MODE_REG, 0x2040);   //set FMS=0, Direct mode
	A7139_WritePageA(TX1_PAGEA, 0xE1CD);   //set TME=0,

	A7139_StrobeCmd(CMD_TX);  //Direct mode, TX
	//while(1);
    return 0;
}

uint8_t A7139_FIFO_Init()
{
	A7139_StrobeCmd(CMD_STBY);  // FIFO mode, TX
	A7139_WritePageB(TX2_PAGEB, 0xD30F);
	A7139_WriteReg(MODE_REG, 0x20C0);   //set FMS=1, FIFO mode
	A7139_WritePageA(TX1_PAGEA, 0xF1CD);   //set TME=1,
    return 0;
}

uint32_t RSSI_measurement()
{
	uint32_t RSSI = 0;
//	A7139_StrobeCmd(CMD_STBY);
//	A7139_WriteReg(ADC_REG,A7139_ReadReg(ADC_REG) & (~0x0300));		//set RSSI to ADC input
//	A7139_WriteReg(ADC_REG, A7139Config[ADC_REG] | 0x8000);
//	A7139_StrobeCmd(CMD_RX);
//	A7139_WriteReg(MODE_REG,A7139_ReadReg(MODE_REG) | 0x0001);		//enable ADC measurment
//	while((A7139_ReadReg(MODE_REG) & 0x0001));
	RSSI = (A7139_ReadReg(ADC_REG) & 0x00FF);
	return RSSI;
}

/* 
#define TEST_preamble    send_preamble_ms(1000)
void TEST_write(void)
{
        A7139_WriteFIFO(&send_packet, 21);
        A7139_SetGIO2_Func(e_GIOS_WTR);
        A7139_SetPackLen(21);
        A7139_StrobeCmd(CMD_TX);
}

void  TEST_read(void)
{
        A7139_StrobeCmd(CMD_STBY);
        vTaskDelay(10);
        A7139_SetGIO2_Func(e_GIOS_FSYNC);
        A7139_SetPackLen(21);
        A7139_StrobeCmd(CMD_RX);
        vTaskDelay(1000);
        A7139_ReadFIFO(fifobuf, 22);
}*/

