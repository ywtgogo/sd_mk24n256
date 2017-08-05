#include <stdbool.h>
#include <stdint.h>
#include "spiflash_config.h"
#include "fsl_dspi_edma.h"
#include "fsl_edma.h"
#include "fsl_dmamux.h"
#include "fsl_device_registers.h"
#include "fsl_debug_console.h"
#include "fsl_dspi.h"
#include "fsl_dspi_freertos.h"

#include "board.h"
#include "pin_mux.h"
#include "clock_config.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/
#define EXAMPLE_DSPI_MASTER_BASEADDR SPI1
#define EXAMPLE_DSPI_MASTER_DMA_MUX_BASEADDR DMAMUX
#define EXAMPLE_DSPI_MASTER_DMA_BASEADDR DMA0
#define EXAMPLE_DSPI_MASTER_DMA_RX_REQUEST_SOURCE 14U
#define EXAMPLE_DSPI_MASTER_DMA_TX_REQUEST_SOURCE 15U
#define DSPI_MASTER_CLK_SRC DSPI1_CLK_SRC
#define EXAMPLE_DSPI_MASTER_PCS_FOR_INIT kDSPI_Pcs0
#define EXAMPLE_DSPI_MASTER_PCS_FOR_TRANSFER kDSPI_MasterPcs0
#define TRANSFER_BAUDRATE 500000U /*! Transfer baudrate - 500k */

//uint8_t masterRxData[TRANSFER_SIZE] = {0};
//uint8_t masterTxData[TRANSFER_SIZE] = {0};

dspi_master_edma_handle_t g_dspi_edma_m_handle;
edma_handle_t dspiEdmaMasterRxRegToRxDataHandle;
edma_handle_t dspiEdmaMasterTxDataToIntermediaryHandle;
edma_handle_t dspiEdmaMasterIntermediaryToTxRegHandle;

void DSPI_MasterUserCallback(SPI_Type *base, dspi_master_edma_handle_t *handle, status_t status, void *userData)
{
    if (status == kStatus_Success)
    {
        PRINTF("This is DSPI master edma transfer completed callback. \r\n\r\n");
    }
}

static int norflash_addr_to_buffer(uint32_t addr, uint8_t *buffer)
{
    int i;

    for (i = FLASH_ADDRESS_BYTES; i; i--)
    {
        buffer[i-1] = (uint8_t)(addr & 0xFF);
        addr >>= 8;
    }

    return FLASH_ADDRESS_BYTES;
}

bool norflash_set_write_latch (dspi_rtos_handle_t *spifd, bool enable)
{
    uint8_t masterTxData[1];
    dspi_transfer_t masterXfer;
    
    if (enable)
    {
        masterTxData[0] = FLASH_WRITE_LATCH_ENABLE_CMD;
    } else {
        masterTxData[0] = FLASH_WRITE_LATCH_DISABLE_CMD;
    }

    /* Start master transfer */
    masterXfer.txData = masterTxData;
    masterXfer.rxData = NULL;
    masterXfer.dataSize = 1;
    masterXfer.configFlags = kDSPI_MasterCtar0 | EXAMPLE_DSPI_MASTER_PCS_FOR_TRANSFER | kDSPI_MasterPcsContinuous;
    if( kStatus_Success != DSPI_RTOS_Transfer(spifd, &masterXfer)){ 
        PRINTF("norflash_write_latch: failed\n");
        return false;
    }
    
    return true;
}

uint8_t norflash_read_status (dspi_rtos_handle_t *spifd)
{
    uint8_t         buffer[1];
    uint8_t         state = 0xFF;
    dspi_transfer_t masterXfer;

    buffer[0] = FLASH_READ_STATUS_CMD;

    /* Start master transfer */
    masterXfer.txData = buffer;
    masterXfer.rxData = NULL;
    masterXfer.dataSize = 1;
    masterXfer.configFlags = kDSPI_MasterCtar0 | EXAMPLE_DSPI_MASTER_PCS_FOR_TRANSFER | kDSPI_MasterPcsContinuous | kDSPI_MasterActiveAfterTransfer;
    if( kStatus_Success != DSPI_RTOS_Transfer(spifd, &masterXfer)){ 
        PRINTF("norflash_read_status:write buffer failed\n");
        return false;
    }
    /* Start master transfer */
    masterXfer.txData = NULL;
    masterXfer.rxData = &state;
    masterXfer.dataSize = 1;
    masterXfer.configFlags = kDSPI_MasterCtar0 | EXAMPLE_DSPI_MASTER_PCS_FOR_TRANSFER | kDSPI_MasterPcsContinuous;
    if( kStatus_Success != DSPI_RTOS_Transfer(spifd, &masterXfer)) {
        PRINTF("norflash_read_status:read buffer failed\n");
        return false;
    }
    
    return state;
}

bool read_ID(dspi_rtos_handle_t *spifd, uint32_t*id)
{
    uint8_t         buffer[5];
    dspi_transfer_t masterXfer;
    int len;

    PRINTF("read id: ");
  
    buffer[0] = FLASH_READ_ID_CMD;
 
    norflash_addr_to_buffer(0x00000000, &(buffer[1]));

    len = 1 + FLASH_ADDRESS_BYTES;

    /* Start master transfer */
    masterXfer.txData = buffer;
    masterXfer.rxData = NULL;
    masterXfer.dataSize = len;
    masterXfer.configFlags = kDSPI_MasterCtar0 | EXAMPLE_DSPI_MASTER_PCS_FOR_TRANSFER | kDSPI_MasterPcsContinuous | kDSPI_MasterActiveAfterTransfer;
    DSPI_RTOS_Transfer(spifd, &masterXfer);
    
    len = 2;

    /* Start master transfer */
    masterXfer.txData = NULL;
    masterXfer.rxData = buffer;
    masterXfer.dataSize = len;
    masterXfer.configFlags = kDSPI_MasterCtar0 | EXAMPLE_DSPI_MASTER_PCS_FOR_TRANSFER | kDSPI_MasterPcsContinuous;
    DSPI_RTOS_Transfer(spifd, &masterXfer);

    for (uint8_t j=0; j<len; j++) 
    {
        PRINTF("%02x", buffer[j]);
    }

    return true;
}

//bool norflash_read_ID_ll (NFTL_DEV_INFO_PTR flash_ptr, MQX_FILE_PTR spifd, uint32_t*id)
//{
//    //while(1)
//    //    read_ID(spifd,id);
//    return 	read_ID(spifd,id);
//}			

bool norflash_chip_erase_ll (dspi_rtos_handle_t *spifd)
{
    dspi_transfer_t masterXfer;
    uint8_t         buffer[1];

    /* This operation must be write-enabled */
    norflash_set_write_latch(spifd, true);

    while (norflash_read_status (spifd) & 1){}

    buffer[0] = FLASH_CHIP_ERASE_CMD;

    /* Start master transfer */
    masterXfer.txData = buffer;
    masterXfer.rxData = NULL;
    masterXfer.dataSize = 1;
    masterXfer.configFlags = kDSPI_MasterCtar0 | EXAMPLE_DSPI_MASTER_PCS_FOR_TRANSFER | kDSPI_MasterPcsContinuous;
    if( kStatus_Success != DSPI_RTOS_Transfer(spifd, &masterXfer)){ 
        PRINTF("norflash_chip_erase: tx failed\n");
        return false;
    }

	//vTaskDelay (10);

    while (norflash_read_status (spifd) & 1){}

    norflash_set_write_latch (spifd, false);
    
    return true;
}

bool norflash_block_erase_ll (dspi_rtos_handle_t *spifd, uint32_t addr)
{
    dspi_transfer_t masterXfer;
    uint8_t         buffer[1 + FLASH_ADDRESS_BYTES];

    addr += FLASH_FS_START_OFFSET;

    /* This operation must be write-enabled */
    norflash_set_write_latch (spifd, true);

    norflash_read_status (spifd);

    buffer[0] = FLASH_BLOCK_ERASE_CMD;
    norflash_addr_to_buffer(addr, &(buffer[1]));

    /* Start master transfer */
    masterXfer.txData = buffer;
    masterXfer.rxData = NULL;
    masterXfer.dataSize = 1 + FLASH_ADDRESS_BYTES;
    masterXfer.configFlags = kDSPI_MasterCtar0 | EXAMPLE_DSPI_MASTER_PCS_FOR_TRANSFER | kDSPI_MasterPcsContinuous;
    if( kStatus_Success != DSPI_RTOS_Transfer(spifd, &masterXfer)){ 
        PRINTF("norflash_block_erase: failed\n");
        return false;
    }

    while (norflash_read_status (spifd) & 1)
    {
        //_time_delay (5);
    }

    return true;
}

bool norflash_sector_erase_ll (dspi_rtos_handle_t *spifd, uint32_t addr)
{
    dspi_transfer_t masterXfer;
    uint8_t         buffer[1 + FLASH_ADDRESS_BYTES];

    addr += FLASH_FS_START_OFFSET;

    /* This operation must be write-enabled */
    norflash_set_write_latch (spifd, true);

    norflash_read_status (spifd);

    buffer[0] = FLASH_SECTOR_ERASE_CMD;
    norflash_addr_to_buffer(addr, &(buffer[1]));

    /* Start master transfer */
    masterXfer.txData = buffer;
    masterXfer.rxData = NULL;
    masterXfer.dataSize = 1 + FLASH_ADDRESS_BYTES;
    masterXfer.configFlags = kDSPI_MasterCtar0 | EXAMPLE_DSPI_MASTER_PCS_FOR_TRANSFER | kDSPI_MasterPcsContinuous;
    if( kStatus_Success != DSPI_RTOS_Transfer(spifd, &masterXfer)){ 
        PRINTF("norflash_sector_erase: failed\n");
        return false;
    }

    return true;
}

bool norflash_read_data_ll (dspi_rtos_handle_t *spifd, uint32_t addr, uint32_t size, uint8_t *data)
{
    dspi_transfer_t masterXfer;
    uint8_t         buffer[5];

    addr += FLASH_FS_START_OFFSET;

    /* Read instruction, address */
    buffer[0] = FLASH_READ_DATA_CMD;
    norflash_addr_to_buffer(addr, &(buffer[1]));

    /* Start master transfer */
    masterXfer.txData = buffer;
    masterXfer.rxData = NULL;
    masterXfer.dataSize = 1 + FLASH_ADDRESS_BYTES;
    masterXfer.configFlags = kDSPI_MasterCtar0 | EXAMPLE_DSPI_MASTER_PCS_FOR_TRANSFER | kDSPI_MasterPcsContinuous | kDSPI_MasterActiveAfterTransfer;
    if( kStatus_Success != DSPI_RTOS_Transfer(spifd, &masterXfer)){ 
        PRINTF("norflash_read_data: tx_addr failed\n");
        return false;
    }

    /* Start master transfer */
    masterXfer.txData = NULL;
    masterXfer.rxData = data;
    masterXfer.dataSize = size;
    masterXfer.configFlags = kDSPI_MasterCtar0 | EXAMPLE_DSPI_MASTER_PCS_FOR_TRANSFER | kDSPI_MasterPcsContinuous;
    if( kStatus_Success != DSPI_RTOS_Transfer(spifd, &masterXfer)) {
        PRINTF("norflash_read_data: rx_data failed\n");
        return false;
    }

    return true;
}

bool norflash_write_data_ll (dspi_rtos_handle_t *spifd, uint32_t addr, uint32_t size, uint8_t *data)
{
    dspi_transfer_t masterXfer;
    uint32_t        len;
    uint32_t        count = size;
    uint8_t         buffer[1 + FLASH_ADDRESS_BYTES];

    addr += FLASH_FS_START_OFFSET;

    while (count > 0)
    {
       /* Each write operation must be enabled in memory */
       norflash_set_write_latch (spifd, true);

       while((norflash_read_status (spifd) & 0x02)==0){}

       len = count;
       if (len > FLASH_PHYSICAL_PAGE_SIZE - (addr & (FLASH_PHYSICAL_PAGE_SIZE - 1)))
       	len = FLASH_PHYSICAL_PAGE_SIZE - (addr & (FLASH_PHYSICAL_PAGE_SIZE - 1));
       count -= len;

       /* Write instruction, address */
       buffer[0] = FLASH_WRITE_DATA_CMD;
       norflash_addr_to_buffer(addr, &(buffer[1]));

        masterXfer.txData = buffer;
        masterXfer.rxData = NULL;
        masterXfer.dataSize = 1 + FLASH_ADDRESS_BYTES;
        masterXfer.configFlags = kDSPI_MasterCtar0 | EXAMPLE_DSPI_MASTER_PCS_FOR_TRANSFER | kDSPI_MasterPcsContinuous | kDSPI_MasterActiveAfterTransfer;
        if( kStatus_Success != DSPI_RTOS_Transfer(spifd, &masterXfer)){ 
            PRINTF("norflash_write_data: tx_addr failed\n");
            return false;
        }

        masterXfer.txData = data;
        masterXfer.rxData = NULL;
        masterXfer.dataSize = (long)len;
        masterXfer.configFlags = kDSPI_MasterCtar0 | EXAMPLE_DSPI_MASTER_PCS_FOR_TRANSFER | kDSPI_MasterPcsContinuous;
        if( kStatus_Success != DSPI_RTOS_Transfer(spifd, &masterXfer)){ 
            PRINTF("norflash_write_data: tx_data failed\n");
            return false;
        }
        
		while(norflash_read_status (spifd)&0x1)
		{
			//_time_delay (1);
		}
		
       /* Move to next block */
        addr += len;
        data += len;
    }
    return true;
}

bool norflash_set_protection_ll (dspi_rtos_handle_t *spifd, bool protect)
{
    dspi_transfer_t masterXfer;
    uint8_t         protection;
    uint8_t         buffer[2];

    /* Must do it twice to ensure right transitions in protection status register */
    for (uint8_t i = 0; i < 2; i++)
    {
        /* Each write operation must be enabled in memory */
        norflash_set_write_latch (spifd, true);

        norflash_read_status (spifd);

        if (protect)
        {
            protection = 0xFF;
        } else {
            protection = 0x00;
        }

        buffer[0] = FLASH_WRITE_STATUS_CMD;
        buffer[1] = protection;

        masterXfer.txData = buffer;
        masterXfer.rxData = NULL;
        masterXfer.dataSize = 1 + FLASH_ADDRESS_BYTES;
        masterXfer.configFlags = kDSPI_MasterCtar0 | EXAMPLE_DSPI_MASTER_PCS_FOR_TRANSFER | kDSPI_MasterPcsContinuous;
        if( kStatus_Success != DSPI_RTOS_Transfer(spifd, &masterXfer)){ 
            PRINTF("norflash_set_protection:falied\n");
            return false;
        }
    }    
    return true;
}


//int vmain(void)
//{
//    BOARD_InitPins();
//    BOARD_BootClockRUN();
//    BOARD_InitDebugConsole();
//
//    PRINTF("DSPI edma example start.\r\n");
//    PRINTF("This example use one dspi instance as master and another as slave on one board.\r\n");
//    PRINTF("Master use edma way , slave uses interrupt.\r\n");
//    PRINTF("Please make sure you make the correct line connection. Basically, the connection is: \r\n");
//    PRINTF("DSPI_master -- DSPI_slave   \r\n");
//    PRINTF("   CLK      --    CLK  \r\n");
//    PRINTF("   PCS      --    PCS  \r\n");
//    PRINTF("   SOUT     --    SIN  \r\n");
//    PRINTF("   SIN      --    SOUT \r\n");
//
//    /* DMA Mux setting and EDMA init */
//    uint32_t masterRxChannel, masterIntermediaryChannel, masterTxChannel;
//    edma_config_t userConfig;
//
//    masterRxChannel = 0U;
//    masterIntermediaryChannel = 1U;
//    masterTxChannel = 2U;
//
//    /* DMA MUX init */
//    DMAMUX_Init(EXAMPLE_DSPI_MASTER_DMA_MUX_BASEADDR);
//
//    DMAMUX_SetSource(EXAMPLE_DSPI_MASTER_DMA_MUX_BASEADDR, masterRxChannel, EXAMPLE_DSPI_MASTER_DMA_RX_REQUEST_SOURCE);
//    DMAMUX_EnableChannel(EXAMPLE_DSPI_MASTER_DMA_MUX_BASEADDR, masterRxChannel);
//
//    DMAMUX_SetSource(EXAMPLE_DSPI_MASTER_DMA_MUX_BASEADDR, masterTxChannel, EXAMPLE_DSPI_MASTER_DMA_TX_REQUEST_SOURCE);
//    DMAMUX_EnableChannel(EXAMPLE_DSPI_MASTER_DMA_MUX_BASEADDR, masterTxChannel);
//
//    /* EDMA init*/
//    /*
//     * userConfig.enableRoundRobinArbitration = false;
//     * userConfig.enableHaltOnError = true;
//     * userConfig.enableContinuousLinkMode = false;
//     * userConfig.enableDebugMode = false;
//     */
//    EDMA_GetDefaultConfig(&userConfig);
//
//    EDMA_Init(EXAMPLE_DSPI_MASTER_DMA_BASEADDR, &userConfig);
//
//    /*DSPI init*/
//    uint32_t srcClock_Hz;
//    dspi_master_config_t masterConfig;
//    dspi_transfer_t masterXfer;
//
//    /*Master config*/
//    masterConfig.whichCtar = kDSPI_Ctar0;
//    masterConfig.ctarConfig.baudRate = TRANSFER_BAUDRATE;
//    masterConfig.ctarConfig.bitsPerFrame = 8U;
//    masterConfig.ctarConfig.cpol = kDSPI_ClockPolarityActiveHigh;
//    masterConfig.ctarConfig.cpha = kDSPI_ClockPhaseFirstEdge;
//    masterConfig.ctarConfig.direction = kDSPI_MsbFirst;
//    masterConfig.ctarConfig.pcsToSckDelayInNanoSec = 1000000000U / TRANSFER_BAUDRATE;
//    masterConfig.ctarConfig.lastSckToPcsDelayInNanoSec = 1000000000U / TRANSFER_BAUDRATE;
//    masterConfig.ctarConfig.betweenTransferDelayInNanoSec = 1000000000U / TRANSFER_BAUDRATE;
//
//    masterConfig.whichPcs = EXAMPLE_DSPI_MASTER_PCS_FOR_INIT;
//    masterConfig.pcsActiveHighOrLow = kDSPI_PcsActiveLow;
//
//    masterConfig.enableContinuousSCK = false;
//    masterConfig.enableRxFifoOverWrite = false;
//    masterConfig.enableModifiedTimingFormat = false;
//    masterConfig.samplePoint = kDSPI_SckToSin0Clock;
//
//    srcClock_Hz = CLOCK_GetFreq(DSPI_MASTER_CLK_SRC);
//    DSPI_MasterInit(EXAMPLE_DSPI_MASTER_BASEADDR, &masterConfig, srcClock_Hz);
//    
//    /* Set up dspi master */
//    memset(&(dspiEdmaMasterRxRegToRxDataHandle), 0, sizeof(dspiEdmaMasterRxRegToRxDataHandle));
//    memset(&(dspiEdmaMasterTxDataToIntermediaryHandle), 0, sizeof(dspiEdmaMasterTxDataToIntermediaryHandle));
//    memset(&(dspiEdmaMasterIntermediaryToTxRegHandle), 0, sizeof(dspiEdmaMasterIntermediaryToTxRegHandle));
//
//    EDMA_CreateHandle(&(dspiEdmaMasterRxRegToRxDataHandle), EXAMPLE_DSPI_MASTER_DMA_BASEADDR, masterRxChannel);
//    EDMA_CreateHandle(&(dspiEdmaMasterTxDataToIntermediaryHandle), EXAMPLE_DSPI_MASTER_DMA_BASEADDR, masterIntermediaryChannel);
//    EDMA_CreateHandle(&(dspiEdmaMasterIntermediaryToTxRegHandle), EXAMPLE_DSPI_MASTER_DMA_BASEADDR, masterTxChannel);
//
//    DSPI_MasterTransferCreateHandleEDMA(EXAMPLE_DSPI_MASTER_BASEADDR, &g_dspi_edma_m_handle, DSPI_MasterUserCallback,
//                                        NULL, &dspiEdmaMasterRxRegToRxDataHandle,
//                                        &dspiEdmaMasterTxDataToIntermediaryHandle,
//                                        &dspiEdmaMasterIntermediaryToTxRegHandle);
//    
//    uint32_t    id;
//    read_ID(g_dspi_edma_m_handle, &id);
//    
//    
//    
//    
//    
//    
//    DSPI_Deinit(EXAMPLE_DSPI_MASTER_BASEADDR);
//}