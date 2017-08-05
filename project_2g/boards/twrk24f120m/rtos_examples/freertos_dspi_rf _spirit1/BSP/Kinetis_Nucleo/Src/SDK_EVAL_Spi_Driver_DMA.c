/**
* @file    SDK_EVAL_Spi_Driver.c
* @author  High End Analog & RF BU - AMS
* @version 3.2.0
* @date    April 01, 2013
* @brief   This file provides all the low level SPI API to access to SPIRIT using a software watchdog timer to avoid stuck situation.
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
#include "cube_hal.h"
#include "MCU_Interface.h"
#include "SDK_EVAL_Config.h"



/** @addtogroup SDK_EVAL_STM32L
* @{
*/


/** @addtogroup SDK_EVAL_Spi                    SDK EVAL Spi
* @brief SPI functions implementation.
* @details This file implements the SPIRIT1 Library SPI interface functions. Please see the MCU_Interface.h file of Spirit1_Library for more details.
* @{
*/


/** @defgroup SPI_Private_TypesDefinitions
* @{
*/


/**
* @}
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

static SPI_HandleTypeDef SpiHandle;
static DMA_HandleTypeDef hdma_tx;
static DMA_HandleTypeDef hdma_rx;
#define SPIx_TX_DMA_CHANNEL              DMA1_Channel3
#define SPIx_RX_DMA_CHANNEL              DMA1_Channel2
#define SPIx_DMA_TX_IRQn                 DMA1_Channel3_IRQn
#define SPIx_DMA_RX_IRQn                 DMA1_Channel2_IRQn
#define SPIx_DMA_TX_IRQHandler           DMA1_Channel3_IRQHandler
#define SPIx_DMA_RX_IRQHandler           DMA1_Channel2_IRQHandler


#define WAIT_FOR_SPI_TC()               {while(!__HAL_DMA_GET_FLAG(SpiHandle.hdmarx, __HAL_DMA_GET_TC_FLAG_INDEX(SpiHandle.hdmarx)));\
                                          HAL_DMA_IRQHandler(SpiHandle.hdmarx);\
                                          HAL_DMA_IRQHandler(SpiHandle.hdmatx);\
                                            while(HAL_SPI_GetState(&SpiHandle) != HAL_SPI_STATE_READY);}
/**
* @}
*/


/** @defgroup SPI_Private_FunctionPrototypes
* @{
*/


#define SPI_ENTER_CRITICAL()           __disable_irq()
#define SPI_EXIT_CRITICAL()            __enable_irq()


//volatile static uint8_t spi_rx_compl=0;
/**
* @}
*/


/** @defgroup SPI_Private_Functions
* @{
*/


#define SdkEvalSPICSLow()        HAL_GPIO_WritePin(NUCLEO_SPI_PERIPH_CS_PORT, NUCLEO_SPI_PERIPH_CS_PIN, GPIO_PIN_RESET)
#define SdkEvalSPICSHigh()       HAL_GPIO_WritePin(NUCLEO_SPI_PERIPH_CS_PORT, NUCLEO_SPI_PERIPH_CS_PIN, GPIO_PIN_SET)

static uint8_t tx_buff[260];
static uint8_t rx_buff[260];
  
/**
* @brief  Deinitializes the SPI for SPIRIT
* @param  None
* @retval None
*/
void SdkEvalSpiDeinit(void)
{
  GPIO_InitTypeDef GPIO_InitStructure;
  
  
  /* Enable SPI periph and SCLK, MOSI, MISO and CS GPIO clocks */
  NUCLEO_SPI_CLK_ENABLE();
  NUCLEO_SPI_PERIPH_CS_CLK_DISABLE();
  NUCLEO_SPI_PERIPH_SCLK_CLK_DISABLE();
  NUCLEO_SPI_PERIPH_MISO_CLK_DISABLE();
  NUCLEO_SPI_PERIPH_MOSI_CLK_DISABLE();
  
  
  /* Configure the AF for MOSI, MISO and SCLK GPIO pins*/
  GPIO_InitStructure.Pin       = NUCLEO_SPI_PERIPH_SCLK_PIN;
  GPIO_InitStructure.Mode      = GPIO_MODE_INPUT;
  GPIO_InitStructure.Pull      = GPIO_PULLUP;
  GPIO_InitStructure.Speed     = GPIO_SPEED_HIGH;
  
  HAL_GPIO_Init(NUCLEO_SPI_PERIPH_SCLK_PORT, &GPIO_InitStructure);
  
  GPIO_InitStructure.Pin = NUCLEO_SPI_PERIPH_MISO_PIN;
  
  HAL_GPIO_Init(NUCLEO_SPI_PERIPH_MISO_PORT, &GPIO_InitStructure);
  
  GPIO_InitStructure.Pin = NUCLEO_SPI_PERIPH_MOSI_PIN;
  
  HAL_GPIO_Init(NUCLEO_SPI_PERIPH_MOSI_PORT, &GPIO_InitStructure);
  
  GPIO_InitStructure.Pin = NUCLEO_SPI_PERIPH_CS_PIN;
  GPIO_InitStructure.Mode = GPIO_MODE_INPUT;
  HAL_GPIO_Init(NUCLEO_SPI_PERIPH_CS_PORT, &GPIO_InitStructure);
  
  /* Configure SPI peripheral */
  if(HAL_SPI_GetState(&SpiHandle) == HAL_SPI_STATE_RESET)
  {  
    /* Set the SPI parameters */
    SpiHandle.Instance               = NUCLEO_SPI_PERIPH_NB;
    SpiHandle.Init.Mode              = SPI_MODE_MASTER;
    SpiHandle.Init.BaudRatePrescaler = SDK_EVAL_SPI_PRESCALER;
    
    SpiHandle.Init.Direction         = SPI_DIRECTION_2LINES;
    SpiHandle.Init.CLKPhase          = SPI_PHASE_1EDGE;
    SpiHandle.Init.CLKPolarity       = SPI_POLARITY_LOW;
    SpiHandle.Init.CRCCalculation    = SPI_CRCCALCULATION_DISABLED;
    SpiHandle.Init.CRCPolynomial     = 7;
    SpiHandle.Init.DataSize          = SPI_DATASIZE_8BIT;
    SpiHandle.Init.FirstBit          = SPI_FIRSTBIT_MSB;
    SpiHandle.Init.NSS               = SPI_NSS_SOFT;
    SpiHandle.Init.TIMode            = SPI_TIMODE_DISABLED;
    
    
    
    
    if(HAL_SPI_DeInit(&SpiHandle) != HAL_OK) {
      return;
    } 
    __HAL_SPI_DISABLE(&SpiHandle);    
  }
  
}


//void HAL_SPI_TxRxCpltCallback(SPI_HandleTypeDef *hspi)
//{
//  spi_rx_compl=1;
//}
//
//void SPIx_DMA_RX_IRQHandler(void)
//{
//  HAL_DMA_IRQHandler(SpiHandle.hdmarx);
//}
//
//void SPIx_DMA_TX_IRQHandler(void)
//{
//  HAL_DMA_IRQHandler(SpiHandle.hdmatx);
//}

/**
* @brief  Initializes the SPI for SPIRIT
* @param  None
* @retval None
*/
void SdkEvalSpiInit(void)
{
  GPIO_InitTypeDef GPIO_InitStructure;
  
  /* Enable SPI periph and SCLK, MOSI, MISO and CS GPIO clocks */
  NUCLEO_SPI_CLK_ENABLE();
  NUCLEO_SPI_PERIPH_CS_CLK_ENABLE();
  NUCLEO_SPI_PERIPH_SCLK_CLK_ENABLE();
  NUCLEO_SPI_PERIPH_MISO_CLK_ENABLE();
  NUCLEO_SPI_PERIPH_MOSI_CLK_ENABLE();
  NUCLEO_SPI_DMA_CLK_ENABLE();
  
  /* Configure the AF for MOSI, MISO and SCLK GPIO pins*/
  GPIO_InitStructure.Pin       = NUCLEO_SPI_PERIPH_SCLK_PIN;
  GPIO_InitStructure.Mode      = GPIO_MODE_AF_PP;
  GPIO_InitStructure.Pull      = GPIO_PULLUP;
  GPIO_InitStructure.Speed     = GPIO_SPEED_HIGH;
  GPIO_InitStructure.Alternate = NUCLEO_SPI_PERIPH_SCLK_AF;
  HAL_GPIO_Init(NUCLEO_SPI_PERIPH_SCLK_PORT, &GPIO_InitStructure);
  
  GPIO_InitStructure.Pin = NUCLEO_SPI_PERIPH_MISO_PIN;
  GPIO_InitStructure.Alternate = NUCLEO_SPI_PERIPH_MISO_AF;
  HAL_GPIO_Init(NUCLEO_SPI_PERIPH_MISO_PORT, &GPIO_InitStructure);
  
  GPIO_InitStructure.Pin = NUCLEO_SPI_PERIPH_MOSI_PIN;
  GPIO_InitStructure.Alternate = NUCLEO_SPI_PERIPH_MOSI_AF;
  HAL_GPIO_Init(NUCLEO_SPI_PERIPH_MOSI_PORT, &GPIO_InitStructure);
  
  GPIO_InitStructure.Pin = NUCLEO_SPI_PERIPH_CS_PIN;
  GPIO_InitStructure.Mode = GPIO_MODE_OUTPUT_PP;
  HAL_GPIO_Init(NUCLEO_SPI_PERIPH_CS_PORT, &GPIO_InitStructure);
  
  
  /* Configure SPI peripheral */
  if(HAL_SPI_GetState(&SpiHandle) == HAL_SPI_STATE_RESET)
  {  
    /* Set the SPI parameters */
    SpiHandle.Instance               = NUCLEO_SPI_PERIPH_NB;
    SpiHandle.Init.Mode              = SPI_MODE_MASTER;
    SpiHandle.Init.BaudRatePrescaler = SDK_EVAL_SPI_PRESCALER;
    
    SpiHandle.Init.Direction         = SPI_DIRECTION_2LINES;
    SpiHandle.Init.CLKPhase          = SPI_PHASE_1EDGE;
    SpiHandle.Init.CLKPolarity       = SPI_POLARITY_LOW;
    SpiHandle.Init.CRCCalculation    = SPI_CRCCALCULATION_DISABLED;
    SpiHandle.Init.CRCPolynomial     = 7;
    SpiHandle.Init.DataSize          = SPI_DATASIZE_8BIT;
    SpiHandle.Init.FirstBit          = SPI_FIRSTBIT_MSB;
    SpiHandle.Init.NSS               = SPI_NSS_SOFT;
    SpiHandle.Init.TIMode            = SPI_TIMODE_DISABLED;
    
    /*##-3- Configure the DMA ##################################################*/
    /* Configure the DMA handler for Transmission process */
    hdma_tx.Instance                 = SPIx_TX_DMA_CHANNEL;
    hdma_tx.Init.Direction           = DMA_MEMORY_TO_PERIPH;
    hdma_tx.Init.PeriphInc           = DMA_PINC_DISABLE;
    hdma_tx.Init.MemInc              = DMA_MINC_ENABLE;
    hdma_tx.Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;
    hdma_tx.Init.MemDataAlignment    = DMA_MDATAALIGN_BYTE;
    hdma_tx.Init.Mode                = DMA_NORMAL;
    hdma_tx.Init.Priority            = DMA_PRIORITY_HIGH;

    HAL_DMA_Init(&hdma_tx);

    /* Associate the initialized DMA handle to the the SPI handle */
    __HAL_LINKDMA(&SpiHandle, hdmatx, hdma_tx);

    /* Configure the DMA handler for Transmission process */
    hdma_rx.Instance                 = SPIx_RX_DMA_CHANNEL;

    hdma_rx.Init.Direction           = DMA_PERIPH_TO_MEMORY;
    hdma_rx.Init.PeriphInc           = DMA_PINC_DISABLE;
    hdma_rx.Init.MemInc              = DMA_MINC_ENABLE;
    hdma_rx.Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;
    hdma_rx.Init.MemDataAlignment    = DMA_MDATAALIGN_BYTE;
    hdma_rx.Init.Mode                = DMA_NORMAL;
    hdma_rx.Init.Priority            = DMA_PRIORITY_HIGH;
    

    HAL_DMA_Init(&hdma_rx);

    /* Associate the initialized DMA handle to the the SPI handle */
    __HAL_LINKDMA(&SpiHandle, hdmarx, hdma_rx);
    
//    /*##-4- Configure the NVIC for DMA #########################################*/ 
//    /* NVIC configuration for DMA transfer complete interrupt (SPI3_TX) */
//    HAL_NVIC_SetPriority(SPIx_DMA_TX_IRQn, 1, 1);
//    HAL_NVIC_EnableIRQ(SPIx_DMA_TX_IRQn);
//    
//    /* NVIC configuration for DMA transfer complete interrupt (SPI3_RX) */
//    HAL_NVIC_SetPriority(SPIx_DMA_RX_IRQn, 1, 0);
//    HAL_NVIC_EnableIRQ(SPIx_DMA_RX_IRQn);
    
    if(HAL_SPI_Init(&SpiHandle) != HAL_OK) {
      return;
    } 
    __HAL_SPI_ENABLE(&SpiHandle);    
  }
  
  SdkEvalSPICSHigh();
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
  tx_buff[0]=WRITE_HEADER;
  tx_buff[1]=cRegAddress;
  
  StatusBytes status;
  
  for(uint32_t i=0;i<cNbBytes;i++)
  {
    tx_buff[i+2]=pcBuffer[i];
  }
  
  SPI_ENTER_CRITICAL();
  
  /* Puts the SPI chip select low to start the transaction */
  SdkEvalSPICSLow();

  HAL_SPI_TransmitReceive_DMA(&SpiHandle, tx_buff, rx_buff, 2+cNbBytes);
  WAIT_FOR_SPI_TC();
  
  /* Puts the SPI chip select high to end the transaction */
  SdkEvalSPICSHigh();
  
  SPI_EXIT_CRITICAL();
  
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
  tx_buff[0]=READ_HEADER;
  tx_buff[1]=cRegAddress;
  
  StatusBytes status;
  
  SPI_ENTER_CRITICAL();
  SdkEvalSPICSLow();
  
  HAL_SPI_TransmitReceive_DMA(&SpiHandle, tx_buff, rx_buff, 2+cNbBytes);
  WAIT_FOR_SPI_TC();
  
  SdkEvalSPICSHigh();
  SPI_EXIT_CRITICAL();
  
  for(uint32_t i=0;i<cNbBytes;i++)
  {
    pcBuffer[i]=rx_buff[i+2];
  }
  
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
  tx_buff[0]=COMMAND_HEADER;
  tx_buff[1]=cCommandCode;
  
  
  StatusBytes status;
  
  SPI_ENTER_CRITICAL();
  SdkEvalSPICSLow();
  
  HAL_SPI_TransmitReceive_DMA(&SpiHandle, tx_buff, rx_buff, 2);
  WAIT_FOR_SPI_TC();
  
  SdkEvalSPICSHigh();
  SPI_EXIT_CRITICAL();
  
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
  tx_buff[0]=WRITE_HEADER;
  tx_buff[1]=LINEAR_FIFO_ADDRESS;
  
  StatusBytes status;
  
  for(uint32_t i=0;i<cNbBytes;i++)
  {
    tx_buff[i+2]=pcBuffer[i];
  }
  
  SPI_ENTER_CRITICAL();
  SdkEvalSPICSLow();
  
  HAL_SPI_TransmitReceive_DMA(&SpiHandle, tx_buff, rx_buff, 2+cNbBytes);
  WAIT_FOR_SPI_TC();
  
  SdkEvalSPICSHigh();
  SPI_EXIT_CRITICAL();
  
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
StatusBytes SdkEvalSpiWriteFifoFast(uint8_t cNbBytes, uint8_t* pcBuffer)
{
  
  StatusBytes status={0};
  
  SPI_ENTER_CRITICAL();
  SdkEvalSPICSLow();
  
  HAL_SPI_TransmitReceive_DMA(&SpiHandle, pcBuffer, rx_buff, cNbBytes);
  WAIT_FOR_SPI_TC();
  
  SdkEvalSPICSHigh();
  SPI_EXIT_CRITICAL();
  
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
  tx_buff[0]=READ_HEADER;
  tx_buff[1]=LINEAR_FIFO_ADDRESS;
  
  StatusBytes status;
  
  SPI_ENTER_CRITICAL();
  SdkEvalSPICSLow();
  
  HAL_SPI_TransmitReceive_DMA(&SpiHandle, tx_buff, rx_buff, 2+cNbBytes);
  WAIT_FOR_SPI_TC();
  
  SdkEvalSPICSHigh();
  SPI_EXIT_CRITICAL();
  
  for(uint32_t i=0;i<cNbBytes;i++)
  {
    pcBuffer[i]=rx_buff[i+2];
  }
  ((uint8_t*)&status)[1]=rx_buff[0];
  ((uint8_t*)&status)[0]=rx_buff[1];
  
  
  return status;
  
}


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