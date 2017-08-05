#include <stdio.h>
#include <string.h>
#include <stdlib.h>

/* FreeRTOS kernel includes. */
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "timers.h"

/* Freescale includes. */
#include "fsl_device_registers.h"
#include "fsl_debug_console.h"
#include "fsl_dspi_freertos.h"
#include "fsl_dspi.h"
#include "fsl_shell.h"

#include "pin_mux.h"
#include "clock_config.h"
#include "board.h"

#include "A7139.h"
#include "SPI_A7139.h"
#include "spiflash_config.h"


static int32_t SpiControl(p_shell_context_t context, int32_t argc, char **argv);

const shell_command_context_t   xSpiCommand = {"spi",
                                "\r\n\"spi arg1\":\r\n"
                                " Usage:\r\n"
                                "    arg1: 0|1                                  Spi index\r\n"
                                /*"    arg2: on|off                               Spi do\r\n"*/,
                                SpiControl, 1};


static int32_t SpiControl(p_shell_context_t context, int32_t argc, char **argv)
{
    int32_t kSpiIndex = ((int32_t)atoi(argv[1]));
    //char *kUartCommand = argv[2];

    /* Check second argument to control led */
    switch (kSpiIndex)
    {
        case 0:
            PRINTF("Test A7139\r\n");
            {
                
                uint32_t ID;
                SPI_A7139_INIT();
                A7139_StrobeCmd(0x55);
                A7139_Init(865.001);
                A7139_SetCID(0x78877887);
                ID = A7139_GetCID();
                PRINTF("0x%x \r\n", ID);    
                /* send premble*/
                uint8_t vpal[64];
                for(int i=0;i<64;i++)
                vpal[i]=0x55;
                A7139_INFS_Mode(1);				//set INFS
                A7139_StrobeCmd(CMD_TFR);
                A7139_WriteFIFO(vpal, 64);
                A7139_StrobeCmd(CMD_TX);
            }
            break;
        case 1:
            PRINTF("Test NorFlash\r\n");           
            spi_flash_init();
            spi_flash_test();
            spi_flash_deinit();
            
            break;
        default:
            SHELL_Printf("Spi index is wrong\r\n");
            break;
    }
    return 0;
}