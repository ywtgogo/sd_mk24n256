#include <stdbool.h>
#include <stdint.h>
#include "fsl_dspi_edma.h"
#include "fsl_edma.h"
#include "fsl_dmamux.h"
#include "fsl_device_registers.h"
#include "fsl_debug_console.h"
#include "fsl_flash.h"

#include "board.h"
#include "pin_mux.h"
#include "clock_config.h"
#include "sha1.h"
#include "spiflash_config.h"
#include "common.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/
#define FLASH_DSPI_MASTER_BASEADDR SPI1
#define FLASH_DSPI_MASTER_CLK_SRC DSPI1_CLK_SRC
#define FLASH_DSPI_MASTER_PCS_FOR_INIT kDSPI_Pcs0
#define FLASH_DSPI_MASTER_PCS_FOR_TRANSFER kDSPI_MasterPcs0
#define TRANSFER_BAUDRATE 50000000U /*! Transfer baudrate - 500k */

dspi_master_handle_t g_m_handle;

void DSPI_MasterUserCallback(SPI_Type *base, dspi_master_handle_t *handle, status_t status, void *userData)
{
    if (status == kStatus_Success)
    {
        __NOP();
        //PRINTF("This is DSPI master transfer completed callback. \r\n\r\n");
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

bool norflash_set_write_latch (dspi_master_handle_t *spifd, bool enable)
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

    if (kStatus_Success != DSPI_MasterTransferNonBlocking(FLASH_DSPI_MASTER_BASEADDR, spifd, &masterXfer))
    {
        PRINTF("There is error when start DSPI_MasterTransferEDMA \r\n ");
        return false;
    }
    
    return true;
}

uint8_t norflash_read_status (dspi_master_handle_t *spifd)
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
    if( kStatus_Success != DSPI_MasterTransferNonBlocking(FLASH_DSPI_MASTER_BASEADDR, spifd, &masterXfer)){ 
        PRINTF("norflash_read_status:write buffer failed\n");
        return false;
    }
    /* Start master transfer */
    masterXfer.txData = NULL;
    masterXfer.rxData = &state;
    masterXfer.dataSize = 1;
    masterXfer.configFlags = kDSPI_MasterCtar0 | FLASH_DSPI_MASTER_PCS_FOR_TRANSFER | kDSPI_MasterPcsContinuous;
    if( kStatus_Success != DSPI_MasterTransferNonBlocking(FLASH_DSPI_MASTER_BASEADDR, spifd, &masterXfer)) {
        PRINTF("norflash_read_status:read buffer failed\n");
        return false;
    }
    
    return state;
}

bool read_ID(dspi_master_handle_t *spifd, uint32_t*id)
{
    uint8_t         buffer[5];
    dspi_transfer_t masterXfer;
    int len;

    PRINTF("read id: ");
  
    buffer[0] = FLASH_READ_ID_CMD;
 
    norflash_addr_to_buffer(0x00000000, &(buffer[1]));

    len = 1 + FLASH_ADDRESS_BYTES;
   
    /*                        */
    //result = fwrite (buffer, 1, len, spifd);
    /* Start master transfer */
    masterXfer.txData = buffer;
    masterXfer.rxData = NULL;
    masterXfer.dataSize = len;
    masterXfer.configFlags = kDSPI_MasterCtar0 | FLASH_DSPI_MASTER_PCS_FOR_TRANSFER | kDSPI_MasterPcsContinuous | kDSPI_MasterActiveAfterTransfer;

    if (kStatus_Success != DSPI_MasterTransferNonBlocking(FLASH_DSPI_MASTER_BASEADDR, spifd, &masterXfer))
    {
        //PRINTF("There is error when start DSPI_MasterTransferEDMA \r\n ");
        PRINTF("norflash_read_ID: tx failed\n");
        return false;
    }
       
    buffer[0] = buffer[1] = buffer[2] = buffer[3] = buffer[4] = 0xFF;

    len = 2;

    //result = fread (buffer, 1, len, spifd);
    /* Start master transfer */
    masterXfer.txData = NULL;
    masterXfer.rxData = buffer;
    masterXfer.dataSize = len;
    masterXfer.configFlags = kDSPI_MasterCtar0 | FLASH_DSPI_MASTER_PCS_FOR_TRANSFER | kDSPI_MasterPcsContinuous;

    if (kStatus_Success != DSPI_MasterTransferNonBlocking(FLASH_DSPI_MASTER_BASEADDR, &g_m_handle, &masterXfer))
    {
        //PRINTF("There is error when start DSPI_MasterTransferEDMA \r\n ");
        PRINTF("norflash_read_ID: rx failed\n");
        return false;
    }

    *id = buffer[1] | (buffer [0] << 8);

    PRINTF("%x\r\n", *id);
    return true;
}

bool norflash_read_ID_ll (dspi_master_handle_t *spifd, uint32_t*id)
{
    //while(1)
    //    read_ID(spifd,id);
    return 	read_ID(spifd, id);
}

bool norflash_chip_erase_ll (dspi_master_handle_t *spifd)
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
    if( kStatus_Success != DSPI_MasterTransferNonBlocking(FLASH_DSPI_MASTER_BASEADDR, spifd, &masterXfer)){ 
        PRINTF("norflash_chip_erase: tx failed\n");
        return false;
    }

	//vTaskDelay (10);

    while (norflash_read_status (spifd) & 1){}

    norflash_set_write_latch (spifd, false);
    
    return true;
}

bool norflash_block_erase_ll (dspi_master_handle_t *spifd, uint32_t addr)
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
    if( kStatus_Success != DSPI_MasterTransferNonBlocking(FLASH_DSPI_MASTER_BASEADDR, spifd, &masterXfer)){ 
        PRINTF("norflash_block_erase: failed\n");
        return false;
    }

    while (norflash_read_status (spifd) & 1)
    {
        //vTaskDelay(5);
    }

    return true;
}

bool norflash_sector_erase_ll (dspi_master_handle_t *spifd, uint32_t addr)
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
    if( kStatus_Success != DSPI_MasterTransferNonBlocking(FLASH_DSPI_MASTER_BASEADDR, spifd, &masterXfer)){ 
        PRINTF("norflash_sector_erase: failed\n");
        return false;
    }
    while (norflash_read_status (spifd) & 1)
    {
        //_time_delay (5);
    }
    return true;
}

bool norflash_read_data_ll (dspi_master_handle_t *spifd, uint32_t addr, uint32_t size, uint8_t *data)
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
    if( kStatus_Success != DSPI_MasterTransferNonBlocking(FLASH_DSPI_MASTER_BASEADDR, spifd, &masterXfer)){ 
        PRINTF("norflash_read_data: tx_addr failed\n");
        return false;
    }

    /* Start master transfer */
    masterXfer.txData = NULL;
    masterXfer.rxData = data;
    masterXfer.dataSize = size;
    masterXfer.configFlags = kDSPI_MasterCtar0 | FLASH_DSPI_MASTER_PCS_FOR_TRANSFER | kDSPI_MasterPcsContinuous;
    if( kStatus_Success != DSPI_MasterTransferNonBlocking(FLASH_DSPI_MASTER_BASEADDR, spifd, &masterXfer)) {
        PRINTF("norflash_read_data: rx_data failed\n");
        return false;
    }

    return true;
}

bool norflash_write_data_ll (dspi_master_handle_t *spifd, uint32_t addr, uint32_t size, uint8_t *data)
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
        if( kStatus_Success != DSPI_MasterTransferNonBlocking(FLASH_DSPI_MASTER_BASEADDR, spifd, &masterXfer)){ 
            PRINTF("norflash_write_data: tx_addr failed\n");
            return false;
        }

        masterXfer.txData = data;
        masterXfer.rxData = NULL;
        masterXfer.dataSize = (long)len;
        masterXfer.configFlags = kDSPI_MasterCtar0 | FLASH_DSPI_MASTER_PCS_FOR_TRANSFER | kDSPI_MasterPcsContinuous;
        if( kStatus_Success != DSPI_MasterTransferNonBlocking(FLASH_DSPI_MASTER_BASEADDR, spifd, &masterXfer)){ 
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

bool norflash_set_protection_ll (dspi_master_handle_t *spifd, bool protect)
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
        if( kStatus_Success != DSPI_MasterTransferNonBlocking(FLASH_DSPI_MASTER_BASEADDR, spifd, &masterXfer)){
            PRINTF("norflash_set_protection:falied\n");
            return false;
        }
    }
    return true;
}

bool img_read(uint32_t addr, uint32_t size, uint8_t *data)
{
	if(norflash_read_data_ll(&g_m_handle,addr,size,data) != true){
		PRINTF("\r\n norflash read error\r\n");
		return false;
	}
	return true;
}

uint32_t srcClock_Hz;
dspi_master_config_t masterConfig;
dspi_transfer_t masterXfer;
void spi_flash_init()
{
	 /* Master config */
	    masterConfig.whichCtar = kDSPI_Ctar0;
	    masterConfig.ctarConfig.baudRate = TRANSFER_BAUDRATE;
	    masterConfig.ctarConfig.bitsPerFrame = 8;
	    masterConfig.ctarConfig.cpol = kDSPI_ClockPolarityActiveHigh;
	    masterConfig.ctarConfig.cpha = kDSPI_ClockPhaseFirstEdge;
	    masterConfig.ctarConfig.direction = kDSPI_MsbFirst;
	    masterConfig.ctarConfig.pcsToSckDelayInNanoSec = 20;
	    masterConfig.ctarConfig.lastSckToPcsDelayInNanoSec = 20;
	    masterConfig.ctarConfig.betweenTransferDelayInNanoSec = 20;

	    masterConfig.whichPcs = FLASH_DSPI_MASTER_PCS_FOR_INIT;
	    masterConfig.pcsActiveHighOrLow = kDSPI_PcsActiveLow;

	    masterConfig.enableContinuousSCK = false;
	    masterConfig.enableRxFifoOverWrite = false;
	    masterConfig.enableModifiedTimingFormat = false;
	    masterConfig.samplePoint = kDSPI_SckToSin0Clock;

	    srcClock_Hz = CLOCK_GetFreq(FLASH_DSPI_MASTER_CLK_SRC);
	    DSPI_MasterInit(FLASH_DSPI_MASTER_BASEADDR, &masterConfig, srcClock_Hz);
    
	    NVIC_SetPriority(SPI1_IRQn, 5U);

	       /* Set up master transfer */
	    DSPI_MasterTransferCreateHandle(FLASH_DSPI_MASTER_BASEADDR, &g_m_handle, DSPI_MasterUserCallback, NULL);
}
/*
//for test
void write_img_into_spi_flash()
{
uint32_t testaddr= 0x32000;
#define testlen 0x2C00

	uint8_t hb_img_write[testlen];
	uint8_t *img_point=hb_img_write;
	uint8_t hb_img_encrypt[testlen]={0};
	uint8_t *encrypt_img_point=hb_img_encrypt;
	flash_config_t flash_config;
    memset(&flash_config, 0, sizeof(flash_config));
	FLASH_Init(&flash_config);
	FLASH_Read(&flash_config,testaddr,hb_img_write,testlen);
	//need encrypt
	unsigned char aesivv[20]="SandlacusData#@1";
	unsigned char aeskey[40]="SandlacusData#@1SandlacusData#@1";
	uint8_t ivv[SHA1_SIZE] = { '\0' };
	AES_KEY enc_key;
	AES_set_encrypt_key(aeskey, 256, &enc_key);
	int num = (testlen+ 4096 - 1) >> 12;
	for(int i = 0;i<num;i++)
	{
		memcpy(ivv, aesivv, 20);
		AES_cbc_encrypt(img_point, encrypt_img_point, BLOCK_SIZE, &enc_key, ivv, 1,1);
		img_point+=BLOCK_SIZE;
		encrypt_img_point+=BLOCK_SIZE;
	}

	spi_flash_init();
	norflash_chip_erase_ll(&g_m_handle);
	norflash_write_data_ll(&g_m_handle, 0x141000, testlen, hb_img_encrypt);
}
*/
