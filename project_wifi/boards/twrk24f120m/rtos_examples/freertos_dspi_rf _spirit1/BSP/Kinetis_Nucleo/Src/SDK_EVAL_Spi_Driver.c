/**
* @file    SDK_EVAL_Spi_Driver.c
* @author  High End Analog & RF BU - AMS
* @version 3.2.0
* @date    April 01, 2013
* @brief   This file provides all the low level SPI API to access to the device using a software watchdog timer to avoid stuck situation.
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
* <h2><center>&copy; COPYRIGHT 2013 STMicroelectronics</center></h2>
*
*/

/* Includes ------------------------------------------------------------------*/
#include "fsl_dspi_freertos.h"
#include "fsl_debug_console.h"

#include "MCU_Interface.h"
#include "board.h"



/** @addtogroup SDK_EVAL_NUCLEO
* @{
*/


/** @addtogroup SDK_EVAL_Spi                    SDK EVAL Spi
* @brief SPI functions implementation.
* @details This file implements the SPI interface functions. Please see the MCU_Interface.h file of the device library for more details.
* @{
*/


/** @defgroup SPI_Private_Defines
* @{
*/


#define CS_TO_SCLK_DELAY  0x0001
#define CLK_TO_CS_DELAY   0x0001

/** @defgroup SPI_Headers
* @{
*/
#define HEADER_WRITE_MASK     0x00 /*!< Write mask for header byte*/
#define HEADER_READ_MASK      0x01 /*!< Read mask for header byte*/
#define HEADER_ADDRESS_MASK   0x00 /*!< Address mask for header byte*/
#define HEADER_COMMAND_MASK   0x80 /*!< Command mask for header byte*/

#define LINEAR_FIFO_ADDRESS 0xFF  /*!< Linear FIFO address*/


/**
* @}
*/


/** @defgroup SDK_EVAL_Spi_Peripheral_Gpio
* @{
*/



/**
* @}
*/

/**
* @}
*/


/** @defgroup SPI_Private_Macros
* @{
*/
#define BUILT_HEADER(add_comm, w_r) (add_comm | w_r)  /*!< macro to build the header byte*/
#define WRITE_HEADER    BUILT_HEADER(HEADER_ADDRESS_MASK, HEADER_WRITE_MASK) /*!< macro to build the write header byte*/
#define READ_HEADER     BUILT_HEADER(HEADER_ADDRESS_MASK, HEADER_READ_MASK)  /*!< macro to build the read header byte*/
#define COMMAND_HEADER  BUILT_HEADER(HEADER_COMMAND_MASK, HEADER_WRITE_MASK) /*!< macro to build the command header byte*/


/**
* @}
*/



/** @defgroup SPI_Private_Variables
* @{
*/

//static SPI_HandleTypeDef SpiHandle;
static dspi_rtos_handle_t SPIRIT1_SPI_Handle;
#define TRANSFER_BAUDRATE 10000000U /*! baudrate - 10M */
#define SPIRIT1_SPI_MASTER_BASEADDR ((SPI_Type *)SPI0_BASE)

/**
* @}
*/


/** @defgroup SPI_Private_FunctionPrototypes
* @{
*/


//#define SPI_ENTER_CRITICAL()           __disable_irq()
//#define SPI_EXIT_CRITICAL()            __enable_irq()

/**
* @}
*/


/** @defgroup SPI_Private_Functions
* @{
*/

#define NUCLEO_SPI_PERIPH_CS_PORT       GPIOD
#define NUCLEO_SPI_PERIPH_CS_PIN        0U
#define SdkEvalSPICSInit()              GPIO_PinInit(NUCLEO_SPI_PERIPH_CS_PORT, NUCLEO_SPI_PERIPH_CS_PIN, &(gpio_pin_config_t){kGPIO_DigitalOutput, (LOGIC_HIGH)})
#define SdkEvalSPICSLow()               GPIO_ClearPinsOutput(NUCLEO_SPI_PERIPH_CS_PORT, 1<<NUCLEO_SPI_PERIPH_CS_PIN)
#define SdkEvalSPICSHigh()              GPIO_SetPinsOutput(NUCLEO_SPI_PERIPH_CS_PORT, 1<<NUCLEO_SPI_PERIPH_CS_PIN)

/**
* @brief  Deinitializes the SPI for SPIRIT
* @param  None
* @retval None
*/
void SdkEvalSpiDeinit(void)
{
    DSPI_RTOS_Deinit(&SPIRIT1_SPI_Handle);
}


/**
* @brief  Write single or multiple SPIRIT register
* @param  baudrate_prescaler: baudrate prescaler to be used
*               Must be one of the @ref SPI_BaudRate_Prescaler .
* @retval None
*/
//void SdkEvalSpiSetBaudrate(uint32_t baudrate_prescaler)
//{
//  __HAL_SPI_DISABLE(&SpiHandle);    
//  
//  SpiHandle.Init.BaudRatePrescaler = baudrate_prescaler;
//
//  if(HAL_SPI_Init(&SpiHandle) != HAL_OK) {
//    return;
//  } 
//  __HAL_SPI_ENABLE(&SpiHandle);    
//  
//}

/**
* @brief  Initializes the SPI for SPIRIT
* @param  None
* @retval None
*/
void SdkEvalSpiInit(void)
{
    dspi_master_config_t masterConfig;
    uint32_t sourceClock;
    status_t status;

    /*Master config*/
    masterConfig.whichCtar = kDSPI_Ctar0;
    masterConfig.ctarConfig.baudRate = 10000000U;//default:500000U
    masterConfig.ctarConfig.bitsPerFrame = 8;
    masterConfig.ctarConfig.cpol = kDSPI_ClockPolarityActiveHigh;
    masterConfig.ctarConfig.cpha = kDSPI_ClockPhaseFirstEdge;
    masterConfig.ctarConfig.direction = kDSPI_MsbFirst;
    masterConfig.ctarConfig.pcsToSckDelayInNanoSec = 100U;// / TRANSFER_BAUDRATE;//2000
    masterConfig.ctarConfig.lastSckToPcsDelayInNanoSec = 100U;//0000000U / TRANSFER_BAUDRATE;//2000
    masterConfig.ctarConfig.betweenTransferDelayInNanoSec = 50U;//0000000U / TRANSFER_BAUDRATE;//1000

    masterConfig.whichPcs = kDSPI_Pcs0;
    masterConfig.pcsActiveHighOrLow = kDSPI_PcsActiveLow;

    masterConfig.enableContinuousSCK = false;
    masterConfig.enableRxFifoOverWrite = false;
    masterConfig.enableModifiedTimingFormat = false;
    masterConfig.samplePoint = kDSPI_SckToSin0Clock;

    NVIC_SetPriority(SPI0_IRQn, 6);

    sourceClock = CLOCK_GetFreq(DSPI0_CLK_SRC);
    status = DSPI_RTOS_Init(&SPIRIT1_SPI_Handle, SPIRIT1_SPI_MASTER_BASEADDR, &masterConfig, sourceClock);
    if (status != kStatus_Success)
    {
        PRINTF("DSPI master: error during initialization. \r\n");
    }
    
    //SdkEvalSPICSInit();
}


/**
* @brief  Write single or multiple SPIRIT register
* @param  cRegAddress: base register's address to be write
* @param  cNbBytes: number of registers and bytes to be write
* @param  pcBuffer: pointer to the buffer of values have to be written into registers
* @retval SPIRIT status
*/
StatusBytes SdkEvalSpiWriteRegisters(uint8_t cRegAddress, uint8_t cNbBytes, uint8_t* pcBuffer)
{
    dspi_transfer_t masterXfer;
    uint8_t tx_buff[2]={WRITE_HEADER,cRegAddress};
    uint8_t rx_buff[255];
    StatusBytes status;
    /* Puts the SPI chip select low to start the transaction */
    //SdkEvalSPICSLow();
    
    masterXfer.txData = tx_buff;
    masterXfer.rxData = rx_buff;
    masterXfer.dataSize = 2;
    masterXfer.configFlags = kDSPI_MasterCtar0 | kDSPI_MasterPcs0 | kDSPI_MasterPcsContinuous | kDSPI_MasterActiveAfterTransfer;
    DSPI_RTOS_Transfer(&SPIRIT1_SPI_Handle, &masterXfer);
    
    masterXfer.txData = pcBuffer;
    masterXfer.rxData = &rx_buff[2];
    masterXfer.dataSize = cNbBytes;
    masterXfer.configFlags = kDSPI_MasterCtar0 | kDSPI_MasterPcs0 | kDSPI_MasterPcsContinuous;
    DSPI_RTOS_Transfer(&SPIRIT1_SPI_Handle, &masterXfer);
    
    /* Puts the SPI chip select high to end the transaction */
    //SdkEvalSPICSHigh();
    ((uint8_t*)&status)[1]=rx_buff[0];
    ((uint8_t*)&status)[0]=rx_buff[1];

    return status;

}

/**
* @brief  Read single or multiple SPIRIT register
* @param  cRegAddress: base register's address to be read
* @param  cNbBytes: number of registers and bytes to be read
* @param  pcBuffer: pointer to the buffer of registers' values read
* @retval SPIRIT status
*/
StatusBytes SdkEvalSpiReadRegisters(uint8_t cRegAddress, uint8_t cNbBytes, uint8_t* pcBuffer)
{
    dspi_transfer_t masterXfer;
    uint8_t tx_buff[255]={READ_HEADER,cRegAddress};
    uint8_t rx_buff[2];
    StatusBytes status;
    /* Puts the SPI chip select low to start the transaction */
    //SdkEvalSPICSLow();
        
    masterXfer.txData = tx_buff;
    masterXfer.rxData = rx_buff;
    masterXfer.dataSize = 2;
    masterXfer.configFlags = kDSPI_MasterCtar0 | kDSPI_MasterPcs0 | kDSPI_MasterPcsContinuous | kDSPI_MasterActiveAfterTransfer;
    DSPI_RTOS_Transfer(&SPIRIT1_SPI_Handle, &masterXfer);
    
    masterXfer.txData = tx_buff;
    masterXfer.rxData = pcBuffer;
    masterXfer.dataSize = cNbBytes;
    masterXfer.configFlags = kDSPI_MasterCtar0 | kDSPI_MasterPcs0 | kDSPI_MasterPcsContinuous;    
    DSPI_RTOS_Transfer(&SPIRIT1_SPI_Handle, &masterXfer);    
    
    /* Puts the SPI chip select high to end the transaction */
    //SdkEvalSPICSHigh();
    ((uint8_t*)&status)[1]=rx_buff[0];
    ((uint8_t*)&status)[0]=rx_buff[1];  

    return status;
}

/**
* @brief  Send a command
* @param  cCommandCode: command code to be sent
* @retval SPIRIT status
*/
StatusBytes SdkEvalSpiCommandStrobes(uint8_t cCommandCode)
{
    dspi_transfer_t masterXfer;
    uint8_t tx_buff[2]={COMMAND_HEADER,cCommandCode};
    uint8_t rx_buff[2];
    StatusBytes status;

    masterXfer.configFlags = kDSPI_MasterCtar0 | kDSPI_MasterPcs0 | kDSPI_MasterPcsContinuous;    
    masterXfer.txData = tx_buff;
    masterXfer.rxData = rx_buff;
    masterXfer.dataSize = 2;
    DSPI_RTOS_Transfer(&SPIRIT1_SPI_Handle, &masterXfer);    

    ((uint8_t*)&status)[1]=rx_buff[0];
    ((uint8_t*)&status)[0]=rx_buff[1];

    return status;
}


/**
* @brief  Write data into TX FIFO
* @param  cNbBytes: number of bytes to be written into TX FIFO
* @param  pcBuffer: pointer to data to write
* @retval SPIRIT status
*/
StatusBytes SdkEvalSpiWriteFifo(uint8_t cNbBytes, uint8_t* pcBuffer)
{
    dspi_transfer_t masterXfer;
    uint8_t tx_buff[2]={WRITE_HEADER,LINEAR_FIFO_ADDRESS};
    uint8_t rx_buff[130];
    StatusBytes status;
 
    masterXfer.txData = tx_buff;
    masterXfer.rxData = rx_buff;
    masterXfer.dataSize = 2;
    masterXfer.configFlags = kDSPI_MasterCtar0 | kDSPI_MasterPcs0 | kDSPI_MasterPcsContinuous | kDSPI_MasterActiveAfterTransfer;
    DSPI_RTOS_Transfer(&SPIRIT1_SPI_Handle, &masterXfer);
    
    masterXfer.txData = pcBuffer;
    masterXfer.rxData = &rx_buff[2];
    masterXfer.dataSize = cNbBytes;
    masterXfer.configFlags = kDSPI_MasterCtar0 | kDSPI_MasterPcs0 | kDSPI_MasterPcsContinuous;   
    DSPI_RTOS_Transfer(&SPIRIT1_SPI_Handle, &masterXfer);    

    ((uint8_t*)&status)[1]=rx_buff[0];
    ((uint8_t*)&status)[0]=rx_buff[1];

    return status;
}


/**
* @brief  Read data from RX FIFO
* @param  cNbBytes: number of bytes to read from RX FIFO
* @param  pcBuffer: pointer to data read from RX FIFO
* @retval SPIRIT status
*/
StatusBytes SdkEvalSpiReadFifo(uint8_t cNbBytes, uint8_t* pcBuffer)
{
    dspi_transfer_t masterXfer;
    uint8_t tx_buff[130]={READ_HEADER,LINEAR_FIFO_ADDRESS};
    uint8_t rx_buff[2];
    StatusBytes status;

    masterXfer.txData = tx_buff;
    masterXfer.rxData = rx_buff;
    masterXfer.dataSize = 2;
    masterXfer.configFlags = kDSPI_MasterCtar0 | kDSPI_MasterPcs0 | kDSPI_MasterPcsContinuous | kDSPI_MasterActiveAfterTransfer;    
    DSPI_RTOS_Transfer(&SPIRIT1_SPI_Handle, &masterXfer);
    
    masterXfer.txData = tx_buff;
    masterXfer.rxData = pcBuffer;
    masterXfer.dataSize = cNbBytes;
    masterXfer.configFlags = kDSPI_MasterCtar0 | kDSPI_MasterPcs0 | kDSPI_MasterPcsContinuous;    
    DSPI_RTOS_Transfer(&SPIRIT1_SPI_Handle, &masterXfer);        

    ((uint8_t*)&status)[1]=rx_buff[0];
    ((uint8_t*)&status)[0]=rx_buff[1];

    return status;
}

//SPI_HandleTypeDef* SdkEvalSpiGetStruct(void)
//{
//  return &SpiHandle;
//}

/**
* @}
*/


/**
* @}
*/


/**
* @}
*/



/******************* (C) COPYRIGHT 2013 STMicroelectronics *****END OF FILE****/