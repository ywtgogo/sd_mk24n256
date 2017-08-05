/**
* @file    SDK_BasicGeneric_A.c
* @author  VMA division - AMS
* @version 3.2.1
* @date    May 1, 2016
* @brief   Example of transmission of SPIRIT Basic packets.
* @details
*
* THE PRESENT FIRMWARE WHICH IS FOR GUIDANCE ONLY AIMS AT PROVIDING CUSTOMERS
* WITH CODING INFORMATION REGARDING THEIR PRODUCTS IN ORDER FOR THEM TO SAVE
* TIME. AS A RESULT, STMICROELECTRONICS SHALL NOT BE HELD LIABLE FOR ANY
* DIRECT, INDIRECT OR CONSEQUENTIAL DAMAGES WITH RESPECT TO ANY CLAIMS ARISING
* FROM THE CONTENT OF SUCH FIRMWARE AND/OR THE USE MADE BY CUSTOMERS OF THE
* CODING INFORMATION CONTAINED HEREIN IN CONNECTION WITH THEIR PRODUCTS.
*
* THIS SOURCE CODE IS PROTECTED BY A LICENSE.
* FOR MORE INFORMATION PLEASE CAREFULLY READ THE LICENSE AGREEMENT FILE LOCATED
* IN THE ROOT DIRECTORY OF THIS FIRMWARE PACKAGE.
*
* <h2><center>&copy; COPYRIGHT 2016 STMicroelectronics</center></h2>
*/


/* Includes ------------------------------------------------------------------*/
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "timers.h"
#include "event_groups.h"

#include "board.h"
#include "fsl_port.h"

#include "SPIRIT_Config.h"
#include "SPIRIT_SDK_Util.h"
#include "SDK_Configuration_Common.h"
#include "string.h"


#define USE_VCOM




/**
* @addtogroup SDK_Examples SDK Examples
* @{
*/


/**
* @addtogroup SDK_Basic_Generic        SDK Basic Generic
* @{
*/

/**
* @addtogroup SDK_Basic_Generic_A                                      SDK Basic Generic A
* @brief Device A configured as a transmitter.
* @details This code explains how to configure and manage
* in the simpler way a transmitter of basic packets.
*
* The user can change the Basic packet configuration parameters editing the defines at the beginning of the file.
* @{
*/



/**
* @defgroup Basic_Generic_A_Private_Variables                                  Basic Generic A Private Variables
* @{
*/

/**
* @brief Radio structure fitting
*/
SRadioInit xRadioInit = {
  XTAL_OFFSET_PPM,
  BASE_FREQUENCY,
  CHANNEL_SPACE,
  CHANNEL_NUMBER,
  MODULATION_SELECT,
  DATARATE,
  FREQ_DEVIATION,
  BANDWIDTH
};
  
/**
* @brief Packet Basic structure fitting
*/
PktBasicInit xBasicInit={
  PREAMBLE_LENGTH,
  SYNC_LENGTH,
  SYNC_WORD,
  LENGTH_TYPE,
  LENGTH_WIDTH,
  CRC_MODE,
  CONTROL_LENGTH,
  EN_ADDRESS,
  EN_FEC,
  EN_WHITENING
};


/**
* @brief Address structure fitting
*/
PktBasicAddressesInit xAddressInit={
  EN_FILT_MY_ADDRESS,
  MY_ADDRESS,
  EN_FILT_MULTICAST_ADDRESS,
  MULTICAST_ADDRESS,
  EN_FILT_BROADCAST_ADDRESS,
  BROADCAST_ADDRESS
};


/**
* @brief GPIO structure fitting
*/
SGpioInit xGpioIRQ={
  SPIRIT_GPIO_3,
  SPIRIT_GPIO_MODE_DIGITAL_OUTPUT_LP,
  SPIRIT_GPIO_DIG_OUT_IRQ
};


/**
* @brief Declare the Tx done flag
*/
FlagStatus xTxDoneFlag = RESET;


/**
* @brief IRQ status struct declaration
*/
SpiritIrqs xIrqStatus;

//ywt code
uint16_t crc_cal(uint8_t* ptr, uint32_t len)
{
	static const  unsigned short crc_ta_8[256] = {
		0x0000, 0x1021, 0x2042, 0x3063, 0x4084, 0x50A5, 0x60C6, 0x70E7,
		0x8108, 0x9129, 0xA14A, 0xB16B, 0xC18C, 0xD1AD, 0xE1CE, 0xF1EF,
		0x1231, 0x0210, 0x3273, 0x2252, 0x52B5, 0x4294, 0x72F7, 0x62D6,
		0x9339, 0x8318, 0xB37B, 0xA35A, 0xD3BD, 0xC39C, 0xF3FF, 0xE3DE,
		0x2462, 0x3443, 0x0420, 0x1401, 0x64E6, 0x74C7, 0x44A4, 0x5485,
		0xA56A, 0xB54B, 0x8528, 0x9509, 0xE5EE, 0xF5CF, 0xC5AC, 0xD58D,
		0x3653, 0x2672, 0x1611, 0x0630, 0x76D7, 0x66F6, 0x5695, 0x46B4,
		0xB75B, 0xA77A, 0x9719, 0x8738, 0xF7DF, 0xE7FE, 0xD79D, 0xC7BC,
		0x48C4, 0x58E5, 0x6886, 0x78A7, 0x0840, 0x1861, 0x2802, 0x3823,
		0xC9CC, 0xD9ED, 0xE98E, 0xF9AF, 0x8948, 0x9969, 0xA90A, 0xB92B,
		0x5AF5, 0x4AD4, 0x7AB7, 0x6A96, 0x1A71, 0x0A50, 0x3A33, 0x2A12,
		0xDBFD, 0xCBDC, 0xFBBF, 0xEB9E, 0x9B79, 0x8B58, 0xBB3B, 0xAB1A,
		0x6CA6, 0x7C87, 0x4CE4, 0x5CC5, 0x2C22, 0x3C03, 0x0C60, 0x1C41,
		0xEDAE, 0xFD8F, 0xCDEC, 0xDDCD, 0xAD2A, 0xBD0B, 0x8D68, 0x9D49,
		0x7E97, 0x6EB6, 0x5ED5, 0x4EF4, 0x3E13, 0x2E32, 0x1E51, 0x0E70,
		0xFF9F, 0xEFBE, 0xDFDD, 0xCFFC, 0xBF1B, 0xAF3A, 0x9F59, 0x8F78,
		0x9188, 0x81A9, 0xB1CA, 0xA1EB, 0xD10C, 0xC12D, 0xF14E, 0xE16F,
		0x1080, 0x00A1, 0x30C2, 0x20E3, 0x5004, 0x4025, 0x7046, 0x6067,
		0x83B9, 0x9398, 0xA3FB, 0xB3DA, 0xC33D, 0xD31C, 0xE37F, 0xF35E,
		0x02B1, 0x1290, 0x22F3, 0x32D2, 0x4235, 0x5214, 0x6277, 0x7256,
		0xB5EA, 0xA5CB, 0x95A8, 0x8589, 0xF56E, 0xE54F, 0xD52C, 0xC50D,
		0x34E2, 0x24C3, 0x14A0, 0x0481, 0x7466, 0x6447, 0x5424, 0x4405,
		0xA7DB, 0xB7FA, 0x8799, 0x97B8, 0xE75F, 0xF77E, 0xC71D, 0xD73C,
		0x26D3, 0x36F2, 0x0691, 0x16B0, 0x6657, 0x7676, 0x4615, 0x5634,
		0xD94C, 0xC96D, 0xF90E, 0xE92F, 0x99C8, 0x89E9, 0xB98A, 0xA9AB,
		0x5844, 0x4865, 0x7806, 0x6827, 0x18C0, 0x08E1, 0x3882, 0x28A3,
		0xCB7D, 0xDB5C, 0xEB3F, 0xFB1E, 0x8BF9, 0x9BD8, 0xABBB, 0xBB9A,
		0x4A75, 0x5A54, 0x6A37, 0x7A16, 0x0AF1, 0x1AD0, 0x2AB3, 0x3A92,
		0xFD2E, 0xED0F, 0xDD6C, 0xCD4D, 0xBDAA, 0xAD8B, 0x9DE8, 0x8DC9,
		0x7C26, 0x6C07, 0x5C64, 0x4C45, 0x3CA2, 0x2C83, 0x1CE0, 0x0CC1,
		0xEF1F, 0xFF3E, 0xCF5D, 0xDF7C, 0xAF9B, 0xBFBA, 0x8FD9, 0x9FF8,
		0x6E17, 0x7E36, 0x4E55, 0x5E74, 0x2E93, 0x3EB2, 0x0ED1, 0x1EF0
	};
	uint16_t crc = 0;
	while(len--)
	{
		crc = (crc << 8 ) ^ crc_ta_8[((crc>>8) ^ *ptr)];
		ptr++;
	}
	return crc;
}

typedef struct rf_packet_t
{
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
} RF_MSG;

/**
* @brief Tx buffer declaration: data to transmit
*/
//uint8_t vectcTxBuff[20]={0x0,0x1,0x2,0x3,0x4,0x5,0x6,0x7,0x8,0x9,0x0,0x1,0x2,0x3,0x4,0x5,0x6,0x7,0x8,0x9};
uint8_t vectcTxBuff[256]={0x69,0x00,0x02,0x50, 0x11,0x22,0x33,0x44, 0x12,0x12,0x12,0x12 ,0x8a, 0,0,0,0,0, 0x1c,0x10};
//ebe01f99e9803523e273d8327c9a5905446c3fcb79
//uint8_t vectcTxBuff[20]={0xeb,0xe0,0x1f,0x99,0xe9,0x80,0x35,0x23,0xe2,0x73,0xc2,0x28,0x62,0x84,0x43,0x1f,0x42,0x6a,0x35,0xc1};

/**
*@}
*/


/**
* @defgroup Basic_Generic_A_Private_Functions                                                  Basic Generic A Private Functions
* @{
*/

/**
* @brief  This function handles External interrupt request (associated with Spirit GPIO 3).
* @param  None
* @retval None
*/

EventGroupHandle_t A7139EventGroup = NULL;
#define B_GIO2_FYNC 	(1 << 0)
EventGroupHandle_t RfFlagEventGroup = NULL;
#define B_TX_DONE 	    (1 << 0)
#define B_RF_DONE    	(1 << 1)

void PORTC_IRQHandler(void)
{
    BaseType_t xHigherPriorityTaskWoken, xResult;
	xHigherPriorityTaskWoken = pdFALSE;
    GPIO_ClearPinsInterruptFlags(A7139_GIO2_GPIO, 1U << A7139_GIO2_GPIO_PIN);
    
    xResult = xEventGroupSetBitsFromISR(A7139EventGroup,  B_GIO2_FYNC, &xHigherPriorityTaskWoken );
    if( xResult != pdFAIL )
    {
        portYIELD_FROM_ISR( xHigherPriorityTaskWoken );
    }
}

void EInt_GIO2_Init(void)
{
    gpio_pin_config_t gio2_config = {
        kGPIO_DigitalInput, 0,
    };
    PORT_SetPinInterruptConfig(A7139_GIO2_PORT, A7139_GIO2_GPIO_PIN, kPORT_InterruptFallingEdge);
    GPIO_PinInit(A7139_GIO2_GPIO, A7139_GIO2_GPIO_PIN, &gio2_config);
    
    NVIC_SetPriority(A7139_GIO2_IRQ, 7);
    EnableIRQ(A7139_GIO2_IRQ);
}

uint8_t vectcRxBuff[96], cRxData;
void status_spirit_task(void *p)
{
    A7139EventGroup = xEventGroupCreate();
    while(1)
    {
        xEventGroupWaitBits(A7139EventGroup,    	/* The event group handle. */
                        B_GIO2_FYNC,			/* The bit pattern the event group is waiting for. */
                        pdTRUE,         		/* BIT will be cleared automatically. */
                        pdTRUE,         		/* Don't wait for both bits, either bit unblock task. */
                        portMAX_DELAY); 		/* Block indefinitely to wait for the condition to be met. */
    
        /* Get the IRQ status */
        SpiritIrqGetStatus(&xIrqStatus);
/* for tx */            
        /* Check the SPIRIT TX_DATA_SENT IRQ flag */
        if(xIrqStatus.IRQ_TX_DATA_SENT)
        {
          /* set the tx_done_flag to manage the event in the main() */
          xTxDoneFlag = SET;
          
          /* toggle LED1 */
          //SdkEvalLedToggle(LED1);
        }
/* for rx */
        if(xIrqStatus.IRQ_RX_DATA_DISC)
        {
          /* toggle LED1 */
          //SdkEvalLedToggle(LED1);
          
          /* RX command - to ensure the device will be ready for the next reception */
          SpiritCmdStrobeRx();
          printf("DATA DISC\r\n");
        }
        
        /* Check the SPIRIT RX_DATA_READY IRQ flag */
        if(xIrqStatus.IRQ_RX_DATA_READY)
        {
            /* Get the RX FIFO size */
            cRxData=SpiritLinearFifoReadNumElementsRxFifo();

            /* Read the RX FIFO */
            SpiritSpiReadLinearFifo(cRxData, vectcRxBuff);

            /* Flush the RX FIFO */
            SpiritCmdStrobeFlushRxFifo();
          
            /*  A simple way to check if the received data sequence is correct (in this case LED5 will toggle) */
            {
                SpiritBool xCorrect=S_TRUE;

                for(uint8_t i=0 ; i<cRxData ; i++)
                if(vectcRxBuff[i] != i+1)
                xCorrect=S_FALSE;

                if(xCorrect) {
                /* toggle LED2 */
                //SdkEvalLedToggle(LED2);
                    printf("DATA CORRECT\r\n");
                }
            }
          
            /* RX command - to ensure the device will be ready for the next reception */
            SpiritCmdStrobeRx();

            /* print the received data */
            printf("B data received: [");
            for(uint8_t i=0 ; i<cRxData ; i++)
            printf("%02x ", vectcRxBuff[i]);
            printf("]\r\n");
        }
    }
}

void set_receiver(void)
{
    //rx irq init 
    SpiritIrqDeInit(NULL);
    //SpiritIrq(RX_DATA_DISC,S_ENABLE);
    SpiritIrq(RX_DATA_READY,S_ENABLE); 
    SpiritIrqClearStatus();
    //rx com
    SpiritCmdStrobeRx();
}

void send_packet(uint8_t *data, uint16_t length)
{
    SpiritIrqDeInit(NULL);
    SpiritCmdStrobeSabort();
    do
    {
        SpiritRefreshStatus();
    }while(g_xStatus.MC_STATE!=MC_STATE_READY); 
    SpiritIrq(TX_DATA_SENT , S_ENABLE);   
    SpiritIrqClearStatus();
    
    /* fit the TX FIFO */
    SpiritCmdStrobeFlushTxFifo();
    SpiritSpiWriteLinearFifo(length, data);
    
    /* send the TX command */
    SpiritCmdStrobeTx();
    
    /* wait for TX done */
    while(!xTxDoneFlag){
        vTaskDelay(3);
    };
    xTxDoneFlag = RESET;
}

void spirit1_main (void)
{
    uint32_t  center_f;
  //HAL_Init();
  
  //SDK_SYSTEM_CONFIG();
    SpiritSpiInit();
      
#ifdef USE_VCOM
  //SdkEvalComInit();
#endif
  
  /* Spirit ON */
  LED1_YELLOW_INIT(LOGIC_LOW);   //0
  LED1_YELLOW_OFF();    //1
  LED1_YELLOW_ON();     //0
  for(volatile uint32_t i=0;i<0x2E00;i++);
  //SpiritEnterShutdown();
  //SpiritExitShutdown();
  SpiritManagementWaExtraCurrent();

  SpiritManagementIdentificationRFBoard();
  
  /* if the board has eeprom, we can compensate the offset calling SpiritManagementGetOffset
  (if eeprom is not present this fcn will return 0) */
  xRadioInit.lFrequencyBase = xRadioInit.lFrequencyBase + SpiritManagementGetOffset();
  
  /* Initialize the signals to drive the range extender application board */
  SpiritManagementRangeExtInit();  

  /* Spirit IRQ config */
  SpiritGpioInit(&xGpioIRQ);  
  
  /* uC IRQ enable */
  EInt_GIO2_Init();
  
  /* Spirit Radio config */
  SpiritRadioInit(&xRadioInit);
  /* get center F */
  center_f = SpiritRadioGetCenterFrequency();
  printf("CenterFrequency[%d]hz\n\r", center_f);
  
  /* Spirit Radio set power */
  SpiritRadioSetPALeveldBm(7,POWER_DBM);
  SpiritRadioSetPALevelMaxIndex(7);
  
  /* Spirit Packet config */
  SpiritPktBasicInit(&xBasicInit);
  SpiritPktBasicAddressesInit(&xAddressInit);

  /* Spirit IRQs enable */
  SpiritIrqDeInit(&xIrqStatus);
  SpiritIrq(RX_DATA_DISC,S_ENABLE);
  SpiritIrq(TX_DATA_SENT, S_ENABLE);
  SpiritIrq(RX_DATA_READY,S_ENABLE);
  
  /* payload length config */
  SpiritPktBasicSetPayloadLength(20);
  
  /* enable SQI check */
  SpiritQiSetSqiThreshold(SQI_TH_0);
  SpiritQiSqiCheck(S_ENABLE);

  /* RX timeout config */
  //SpiritTimerSetRxTimeoutMs(100.0);
  SET_INFINITE_RX_TIMEOUT();
  SpiritTimerSetRxTimeoutStopCondition(SQI_ABOVE_THRESHOLD);  
  
  /* IRQ registers blanking */
  SpiritIrqClearStatus();
  
  /* destination address.
  By default it is not used because address is disabled, see struct xAddressInit*/
  SpiritPktBasicSetDestinationAddress(0x35);

  /* IRQ registers blanking */
  SpiritIrqClearStatus();
  
  /* RX command */
  SpiritCmdStrobeRx();
  
  /* infinite loop */
  while (1){
#ifdef USE_VCOM
    printf("A data to transmit: [");
    for(uint8_t i=0 ; i<20 ; i++)
      printf("%02x ", vectcTxBuff[i]);
    printf("]\n\r");
#endif
    vectcTxBuff[18] = crc_cal(vectcTxBuff, 18);
    vectcTxBuff[19] = crc_cal(vectcTxBuff, 18)>>8;

    send_packet(vectcTxBuff, 20);
    
    set_receiver();

    /* pause between two transmissions */
    vTaskDelay(1000);
  }
}



#ifdef  USE_FULL_ASSERT
/**
* @brief  Reports the name of the source file and the source line number
*         where the assert_param error has occurred.
* @param file: pointer to the source file name
* @param line: assert_param error line source number
* @retval : None
*/
void assert_failed(uint8_t* file, uint32_t line)
{
  /* User can add his own implementation to report the file name and line number */
  printf("Wrong parameters value: file %s on line %d\r\n", file, line);
  
  /* Infinite loop */
  while (1)
  {
  }
}
#endif



/**
*@}
*/

/**
*@}
*/

/**
*@}
*/

/**
*@}
*/


/******************* (C) COPYRIGHT 2016 STMicroelectronics *****END OF FILE****/
