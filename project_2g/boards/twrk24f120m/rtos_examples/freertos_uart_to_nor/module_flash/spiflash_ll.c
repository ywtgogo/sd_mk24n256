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
#define FLASH_DSPI_MASTER_BASEADDR SPI1
#define FLASH_DSPI_CLK_SRC DSPI1_CLK_SRC
#define FLASH_DSPI_MASTER_PCS_FOR_INIT kDSPI_Pcs0
#define FLASH_DSPI_MASTER_PCS_FOR_TRANSFER kDSPI_MasterPcs0
#define FLASH_TRANSFER_BAUDRATE 10000000U /*! Transfer baudrate - 500k */
    
dspi_rtos_handle_t      flash_master_rtos_handle;

SemaphoreHandle_t flash_dspi_sem;
#define master_task_PRIORITY (configMAX_PRIORITIES - 3)


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
    masterXfer.configFlags = kDSPI_MasterCtar0 | FLASH_DSPI_MASTER_PCS_FOR_TRANSFER | kDSPI_MasterPcsContinuous;
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
    masterXfer.configFlags = kDSPI_MasterCtar0 | FLASH_DSPI_MASTER_PCS_FOR_TRANSFER | kDSPI_MasterPcsContinuous | kDSPI_MasterActiveAfterTransfer;
    if( kStatus_Success != DSPI_RTOS_Transfer(spifd, &masterXfer)){ 
        PRINTF("norflash_read_status:write buffer failed\n");
        return false;
    }
    /* Start master transfer */
    masterXfer.txData = NULL;
    masterXfer.rxData = &state;
    masterXfer.dataSize = 1;
    masterXfer.configFlags = kDSPI_MasterCtar0 | FLASH_DSPI_MASTER_PCS_FOR_TRANSFER | kDSPI_MasterPcsContinuous;
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

    //PRINTF("read id: ");
  
    buffer[0] = FLASH_READ_ID_CMD;
 
    norflash_addr_to_buffer(0x00000000, &(buffer[1]));

    len = 1 + FLASH_ADDRESS_BYTES;

    /* Start master transfer */
    masterXfer.txData = buffer;
    masterXfer.rxData = NULL;
    masterXfer.dataSize = len;
    masterXfer.configFlags = kDSPI_MasterCtar0 | FLASH_DSPI_MASTER_PCS_FOR_TRANSFER | kDSPI_MasterPcsContinuous | kDSPI_MasterActiveAfterTransfer;
    DSPI_RTOS_Transfer(spifd, &masterXfer);
    
    len = 2;

    /* Start master transfer */
    masterXfer.txData = NULL;
    masterXfer.rxData = buffer;
    masterXfer.dataSize = len;
    masterXfer.configFlags = kDSPI_MasterCtar0 | FLASH_DSPI_MASTER_PCS_FOR_TRANSFER | kDSPI_MasterPcsContinuous;
    DSPI_RTOS_Transfer(spifd, &masterXfer);
    PRINTF("\r\nNorFlashID: ");
    for (uint8_t j=0; j<len; j++) 
    {
        PRINTF("%02x", buffer[j]);
    }
    PRINTF("\r\n");

    return true;
}

bool norflash_read_ID_ll (dspi_rtos_handle_t *spifd, uint32_t*id)
{
    //while(1)
    //    read_ID(spifd,id);
    return 	read_ID(spifd, id);
}

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
    masterXfer.configFlags = kDSPI_MasterCtar0 | FLASH_DSPI_MASTER_PCS_FOR_TRANSFER | kDSPI_MasterPcsContinuous;
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
    masterXfer.configFlags = kDSPI_MasterCtar0 | FLASH_DSPI_MASTER_PCS_FOR_TRANSFER | kDSPI_MasterPcsContinuous;
    if( kStatus_Success != DSPI_RTOS_Transfer(spifd, &masterXfer)){ 
        PRINTF("norflash_block_erase: failed\n");
        return false;
    }

    while (norflash_read_status (spifd) & 1)
    {
        //vTaskDelay(5);
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
    masterXfer.configFlags = kDSPI_MasterCtar0 | FLASH_DSPI_MASTER_PCS_FOR_TRANSFER | kDSPI_MasterPcsContinuous;
    if( kStatus_Success != DSPI_RTOS_Transfer(spifd, &masterXfer)){ 
        PRINTF("norflash_sector_erase: failed\n");
        return false;
    }
    while (norflash_read_status (spifd) & 1)
    {
        //_time_delay (5);
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
    masterXfer.configFlags = kDSPI_MasterCtar0 | FLASH_DSPI_MASTER_PCS_FOR_TRANSFER | kDSPI_MasterPcsContinuous | kDSPI_MasterActiveAfterTransfer;
    if( kStatus_Success != DSPI_RTOS_Transfer(spifd, &masterXfer)){ 
        PRINTF("norflash_read_data: tx_addr failed\n");
        return false;
    }

    /* Start master transfer */
    masterXfer.txData = NULL;
    masterXfer.rxData = data;
    masterXfer.dataSize = size;
    masterXfer.configFlags = kDSPI_MasterCtar0 | FLASH_DSPI_MASTER_PCS_FOR_TRANSFER | kDSPI_MasterPcsContinuous;
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
        masterXfer.configFlags = kDSPI_MasterCtar0 | FLASH_DSPI_MASTER_PCS_FOR_TRANSFER | kDSPI_MasterPcsContinuous | kDSPI_MasterActiveAfterTransfer;
        if( kStatus_Success != DSPI_RTOS_Transfer(spifd, &masterXfer)){ 
            PRINTF("norflash_write_data: tx_addr failed\n");
            return false;
        }

        masterXfer.txData = data;
        masterXfer.rxData = NULL;
        masterXfer.dataSize = (long)len;
        masterXfer.configFlags = kDSPI_MasterCtar0 | FLASH_DSPI_MASTER_PCS_FOR_TRANSFER | kDSPI_MasterPcsContinuous;
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
        masterXfer.configFlags = kDSPI_MasterCtar0 | FLASH_DSPI_MASTER_PCS_FOR_TRANSFER | kDSPI_MasterPcsContinuous;
        if( kStatus_Success != DSPI_RTOS_Transfer(spifd, &masterXfer)){ 
            PRINTF("norflash_set_protection:falied\n");
            return false;
        }
    }    
    return true;
}

//bool norflash_cachewrite_data_ll()
//{
//
//}

bool image_storage(uint32_t addr, uint32_t size, uint8_t *data)
{
	if(norflash_sector_erase_ll(&flash_master_rtos_handle,addr) != true){
		PRINTF("\r\n norflash erase error\r\n");
		return false;
	}
	PRINTF("\r\naddr:%d\r\n",addr);
    if(norflash_write_data_ll(&flash_master_rtos_handle,addr,size,data) != true){
    	PRINTF("\r\n norflash write error\r\n");
    	return false;
    }

    return true;
}

bool image_read(uint32_t addr, uint32_t size, uint8_t *data)
{
	if(norflash_read_data_ll(&flash_master_rtos_handle,addr,size,data) != true){
		PRINTF("\r\n norflash read error\r\n");
		return false;
	}
	return true;
}

void spi_flash_test(void)
{
    uint32_t    id;
    uint8_t     data[4096]={0};
    
    read_ID(&flash_master_rtos_handle, &id);

    for (uint32_t j=0; j<(sizeof(data)); j++)
    {
        data[j] = j;
        //PRINTF("%02x", data[j]);
    }
    PRINTF("erase start\r\n");
    norflash_block_erase_ll(&flash_master_rtos_handle, 0x00);
    norflash_block_erase_ll(&flash_master_rtos_handle, 0x10000);
    norflash_block_erase_ll(&flash_master_rtos_handle, 0x20000);
    norflash_block_erase_ll(&flash_master_rtos_handle, 0x30000);
    norflash_sector_erase_ll(&flash_master_rtos_handle,0x40000);
    PRINTF("erase done!\r\n");
    PRINTF("write start\r\n");
    norflash_write_data_ll(&flash_master_rtos_handle, 0x30, sizeof(data), data);
    norflash_write_data_ll(&flash_master_rtos_handle, 0x30+sizeof(data)*1, sizeof(data), data);
    norflash_write_data_ll(&flash_master_rtos_handle, 0x30+sizeof(data)*2, sizeof(data), data);
    norflash_write_data_ll(&flash_master_rtos_handle, 0x30+sizeof(data)*3, sizeof(data), data);
    PRINTF("write done!\r\n");
    memset(data, 0, 4096);
    norflash_read_data_ll (&flash_master_rtos_handle, 0x10, sizeof(data), data);
    for (uint32_t j=0; j<sizeof(data); j++)
    {
        PRINTF("%02x", data[j]);
    }
    PRINTF("\r\n");
}

void spi_flash_deinit(void)
{
    if (&flash_master_rtos_handle != NULL)
    {
        DSPI_RTOS_Deinit(&flash_master_rtos_handle);
    }
}

/*!
 * @brief Task responsible for master SPI communication.
 */
void spi_flash_init(void)
{
    dspi_master_config_t    flash_masterConfig;
    uint32_t                flash_sourceClock;
    status_t                status;

    /*Master config*/
    flash_masterConfig.whichCtar = kDSPI_Ctar0;
    flash_masterConfig.ctarConfig.baudRate = FLASH_TRANSFER_BAUDRATE;
    flash_masterConfig.ctarConfig.bitsPerFrame = 8;
    flash_masterConfig.ctarConfig.cpol = kDSPI_ClockPolarityActiveHigh;
    flash_masterConfig.ctarConfig.cpha = kDSPI_ClockPhaseFirstEdge;
    flash_masterConfig.ctarConfig.direction = kDSPI_MsbFirst;
    flash_masterConfig.ctarConfig.pcsToSckDelayInNanoSec = 20;
    flash_masterConfig.ctarConfig.lastSckToPcsDelayInNanoSec = 20;
    flash_masterConfig.ctarConfig.betweenTransferDelayInNanoSec = 20;

    flash_masterConfig.whichPcs = kDSPI_Pcs0;
    flash_masterConfig.pcsActiveHighOrLow = kDSPI_PcsActiveLow;

    flash_masterConfig.enableContinuousSCK = false;
    flash_masterConfig.enableRxFifoOverWrite = false;
    flash_masterConfig.enableModifiedTimingFormat = false;
    flash_masterConfig.samplePoint = kDSPI_SckToSin0Clock;

    NVIC_SetPriority(SPI1_IRQn, 7);

    flash_sourceClock = CLOCK_GetFreq(FLASH_DSPI_CLK_SRC);
    status = DSPI_RTOS_Init(&flash_master_rtos_handle, FLASH_DSPI_MASTER_BASEADDR, &flash_masterConfig, flash_sourceClock);

    if (status != kStatus_Success)
    {
        PRINTF("DSPI master: error during initialization. \r\n");
    }
    PRINTF("DSPI master: initialized. \r\n");
}

