#ifndef __SPIFLASH_CONFIG_H__
#define __SPIFLASH_CONFIG_H__

#define MERGE_BLOCK_LIMIT	5

#define FLASH_W25Q64    1
#define FLASH_S25FL064  2 
#define AT26DF081       3

#define BSP_FLASH_DEVICE    FLASH_W25Q64

//////////////////////////////////////////////////
//     W25Q64 FLASH INFO CONFIGURATION          //
//////////////////////////////////////////////////

#define W25Q64_ADDRESS_BYTES             3

#define W25Q64_BLOCKCOUNT                128
#define W25Q32_BLOCKCOUNT                64
#define W25Q64_BLOCKSIZE		         65536
#define W25Q64_BLOCK_SHIFT_NUMBER        16
#define W25Q64_ERASE_SECTORSIZE          4096
#define W25Q64_ERASE_SECTOR_SHIFT_NUMBER 12
#define W25Q64_PAGE_SIZE                 256
#define W25Q64_ID			             0xef16
#define W25Q32_ID			             0xef15

#define W25Q64_WRITE_STATUS_CMD          0x01
#define W25Q64_WRITE_DATA_CMD            0x02
#define W25Q64_READ_DATA_CMD             0x03
#define W25Q64_WRITE_LATCH_DISABLE_CMD   0x04
#define W25Q64_READ_STATUS_CMD           0x05
#define W25Q64_WRITE_LATCH_ENABLE_CMD    0x06
#define W25Q64_CHIP_ERASE_CMD            0xC7
#define W25Q64_SECTOR_ERASE_CMD	         0x20
#define W25Q64_READ_ID_CMD               0x90
#define W25Q64_BLOCK_ERASE_CMD	    	 0xd8


//////////////////////////////////////////////////
//     S25FL064 FLASH INFO CONFIGURATION          //
//////////////////////////////////////////////////

#define S25FL064_ADDRESS_BYTES             3

#define S25FL064_BLOCKCOUNT                64
#define S25FL064_BLOCKSIZE				   65536
#define S25FL064_BLOCK_SHIFT_NUMBER        16
#define S25FL064_ERASE_SECTORSIZE          4096
#define S25FL064_ERASE_SECTOR_SHIFT_NUMBER 12
#define S25FL064_PAGE_SIZE                 256
#define S25FL064_ID						   0x0116

#define S25FL064_WRITE_STATUS_CMD          0x01
#define S25FL064_WRITE_DATA_CMD            0x02
#define S25FL064_READ_DATA_CMD             0x03
#define S25FL064_WRITE_LATCH_DISABLE_CMD   0x04
#define S25FL064_READ_STATUS_CMD           0x05
#define S25FL064_WRITE_LATCH_ENABLE_CMD    0x06
#define S25FL064_CHIP_ERASE_CMD            0xC7
#define S25FL064_SECTOR_ERASE_CMD	       0x20
#define S25FL064_READ_ID_CMD               0x90
#define S25FL064_BLOCK_ERASE_CMD	       0xd8

//////////////////////////////////////////////////
//     AT26DF081A FLASH INFO CONFIGURATION          //
//////////////////////////////////////////////////

#define AT26DF081_ADDRESS_BYTES             3

#define AT26DF081_BLOCKCOUNT                16
#define AT26DF081_BLOCKSIZE		    65536
#define AT26DF081_BLOCK_SHIFT_NUMBER        16
#define AT26DF081_ERASE_SECTORSIZE          4096
#define AT26DF081_ERASE_SECTOR_SHIFT_NUMBER 12
#define AT26DF081_PAGE_SIZE                 256
#define AT26DF081_ID			    0x1f45

#define AT26DF081_WRITE_STATUS_CMD          0x01
#define AT26DF081_WRITE_DATA_CMD            0x02
#define AT26DF081_READ_DATA_CMD             0x03
#define AT26DF081_WRITE_LATCH_DISABLE_CMD   0x04
#define AT26DF081_READ_STATUS_CMD           0x05
#define AT26DF081_WRITE_LATCH_ENABLE_CMD    0x06
#define AT26DF081_CHIP_ERASE_CMD            0xC7
#define AT26DF081_SECTOR_ERASE_CMD	    0x20
#define AT26DF081_READ_ID_CMD               0x9f
#define AT26DF081_BLOCK_ERASE_CMD	    0xd8


////////////////////////////////////////////////////////////////////////////
#define FLASH_FS_START_BLOCK                (16)

#if (BSP_FLASH_DEVICE == FLASH_W25Q64)

    #define FLASH_ADDRESS_BYTES              W25Q64_ADDRESS_BYTES

    #define FLASH_BLOCKCOUNT                 W25Q64_BLOCKCOUNT
    #define FLASH_BLOCKSIZE                  W25Q64_BLOCKSIZE
    #define FLASH_BLOCK_SHIFT_NUMBER         W25Q64_BLOCK_SHIFT_NUMBER
    #define FLASH_ERASE_SECTORSIZE           W25Q64_ERASE_SECTORSIZE
    #define FLASH_ERASE_SECTOR_SHIFT_NUMBER  W25Q64_ERASE_SECTOR_SHIFT_NUMBER
    #define FLASH_PHYSICAL_PAGE_SIZE         W25Q64_PAGE_SIZE
    #define FLASH_ID			     W25Q64_ID

    #define FLASH_WRITE_STATUS_CMD           W25Q64_WRITE_STATUS_CMD
    #define FLASH_WRITE_DATA_CMD             W25Q64_WRITE_DATA_CMD
    #define FLASH_READ_DATA_CMD              W25Q64_READ_DATA_CMD
    #define FLASH_WRITE_LATCH_DISABLE_CMD    W25Q64_WRITE_LATCH_DISABLE_CMD
    #define FLASH_READ_STATUS_CMD            W25Q64_READ_STATUS_CMD
    #define FLASH_WRITE_LATCH_ENABLE_CMD     W25Q64_WRITE_LATCH_ENABLE_CMD
    #define FLASH_CHIP_ERASE_CMD             W25Q64_CHIP_ERASE_CMD
    #define FLASH_SECTOR_ERASE_CMD	     W25Q64_SECTOR_ERASE_CMD
    #define FLASH_READ_ID_CMD                W25Q64_READ_ID_CMD
    #define FLASH_BLOCK_ERASE_CMD	     W25Q64_BLOCK_ERASE_CMD

#elif(BSP_FLASH_DEVICE == FLASH_S25FL064)

    #define FLASH_ADDRESS_BYTES              S25FL064_ADDRESS_BYTES

    #define FLASH_BLOCKCOUNT                 S25FL064_BLOCKCOUNT
    #define FLASH_BLOCKSIZE                  S25FL064_BLOCKSIZE
    #define FLASH_BLOCK_SHIFT_NUMBER         S25FL064_BLOCK_SHIFT_NUMBER
    #define FLASH_ERASE_SECTORSIZE           S25FL064_ERASE_SECTORSIZE
    #define FLASH_ERASE_SECTOR_SHIFT_NUMBER  S25FL064_ERASE_SECTOR_SHIFT_NUMBER
    #define FLASH_PHYSICAL_PAGE_SIZE         S25FL064_PAGE_SIZE
    #define FLASH_ID						 S25FL064_ID

    #define FLASH_WRITE_STATUS_CMD           S25FL064_WRITE_STATUS_CMD
    #define FLASH_WRITE_DATA_CMD             S25FL064_WRITE_DATA_CMD
    #define FLASH_READ_DATA_CMD              S25FL064_READ_DATA_CMD
    #define FLASH_WRITE_LATCH_DISABLE_CMD    S25FL064_WRITE_LATCH_DISABLE_CMD
    #define FLASH_READ_STATUS_CMD            S25FL064_READ_STATUS_CMD
    #define FLASH_WRITE_LATCH_ENABLE_CMD     S25FL064_WRITE_LATCH_ENABLE_CMD
    #define FLASH_CHIP_ERASE_CMD             S25FL064_CHIP_ERASE_CMD
    #define FLASH_SECTOR_ERASE_CMD	         S25FL064_SECTOR_ERASE_CMD
    #define FLASH_READ_ID_CMD                S25FL064_READ_ID_CMD
    #define FLASH_BLOCK_ERASE_CMD			 S25FL064_BLOCK_ERASE_CMD

#elif(BSP_FLASH_DEVICE == AT26DF081)

    #define FLASH_ADDRESS_BYTES              AT26DF081_ADDRESS_BYTES

    #define FLASH_BLOCKCOUNT                 AT26DF081_BLOCKCOUNT
    #define FLASH_BLOCKSIZE                  AT26DF081_BLOCKSIZE
    #define FLASH_BLOCK_SHIFT_NUMBER         AT26DF081_BLOCK_SHIFT_NUMBER
    #define FLASH_ERASE_SECTORSIZE           AT26DF081_ERASE_SECTORSIZE
    #define FLASH_ERASE_SECTOR_SHIFT_NUMBER  AT26DF081_ERASE_SECTOR_SHIFT_NUMBER
    #define FLASH_PHYSICAL_PAGE_SIZE         AT26DF081_PAGE_SIZE
    #define FLASH_ID						 AT26DF081_ID

    #define FLASH_WRITE_STATUS_CMD           AT26DF081_WRITE_STATUS_CMD
    #define FLASH_WRITE_DATA_CMD             AT26DF081_WRITE_DATA_CMD
    #define FLASH_READ_DATA_CMD              AT26DF081_READ_DATA_CMD
    #define FLASH_WRITE_LATCH_DISABLE_CMD    AT26DF081_WRITE_LATCH_DISABLE_CMD
    #define FLASH_READ_STATUS_CMD            AT26DF081_READ_STATUS_CMD
    #define FLASH_WRITE_LATCH_ENABLE_CMD     AT26DF081_WRITE_LATCH_ENABLE_CMD
    #define FLASH_CHIP_ERASE_CMD             AT26DF081_CHIP_ERASE_CMD
    #define FLASH_SECTOR_ERASE_CMD	         AT26DF081_SECTOR_ERASE_CMD
    #define FLASH_READ_ID_CMD                AT26DF081_READ_ID_CMD
    #define FLASH_BLOCK_ERASE_CMD			 AT26DF081_BLOCK_ERASE_CMD

#else
	#error please config your SPI-FLASH device in spiflash_config.h
#endif

#define FLASH_FS_START_OFFSET            (FLASH_BLOCKSIZE * FLASH_FS_START_BLOCK)


#include "fsl_dspi.h"
bool norflash_read_ID_ll (dspi_master_handle_t *spifd, uint32_t*id);
bool norflash_chip_erase_ll (dspi_master_handle_t *spifd);
bool norflash_block_erase_ll (dspi_master_handle_t *spifd, uint32_t addr);
bool norflash_sector_erase_ll (dspi_master_handle_t *spifd, uint32_t addr);
bool norflash_read_data_ll (dspi_master_handle_t *spifd, uint32_t addr, uint32_t size, uint8_t *data);
bool norflash_write_data_ll (dspi_master_handle_t *spifd, uint32_t addr, uint32_t size, uint8_t *data);
bool norflash_set_protection_ll (dspi_master_handle_t *spifd, bool protect);
void check_spi_flash_image(void);

bool image_read(uint32_t addr, uint32_t size, uint8_t *data);
#endif 
