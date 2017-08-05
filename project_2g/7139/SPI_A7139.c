#include "board.h"
#include "SPI_A7139.h"
#include <fsl_port.h>
#include <stdbool.h>
#include "fsl_dspi_freertos.h"
#include "fsl_dspi.h"
#include "fsl_debug_console.h"
#include "event_groups.h"
#ifndef DRIVER_RF_PROJECT
//#include "uart_ppp.h"
#endif
#define TRANSFER_BAUDRATE 10000000U /*! baudrate - 10M */
#define A7139_SPI_MASTER_BASE (SPI0_BASE)
#define A7139_SPI_MASTER_BASEADDR ((SPI_Type *)A7139_SPI_MASTER_BASE)

dspi_rtos_handle_t A7139_SPI_Handle;
EventGroupHandle_t A7139EventGroup = NULL;

uint32_t   GIO2_FLAG;
uint32_t   GIO2S = RX;

#ifndef DRIVER_RF_PROJECT
//extern ppp_gprs_info_conf ppp_gprs_info;
#endif
#ifndef CPU_MK24FN256VDC12
void PORTB_IRQHandler(void)
{
	BaseType_t xHigherPriorityTaskWoken, xResult;
	xHigherPriorityTaskWoken = pdFALSE;
    GPIO_ClearPinsInterruptFlags(GPIOB, 1U << 2);
    if (GIO2S == RX) {
    	GIO2_FLAG  = 1;
    	xResult = xEventGroupSetBitsFromISR(A7139EventGroup,  B_GIO2_FYNC, &xHigherPriorityTaskWoken );
    	if( xResult != pdFAIL )
    	{
    		portYIELD_FROM_ISR( xHigherPriorityTaskWoken );
    	}
    }
    if (GIO2S == TXIN) {
    	GIO2S = TXEND;
    }	
}

void EInt_GIO2_Init(void)
{
    gpio_pin_config_t gio2_config = {
        kGPIO_DigitalInput, 0,
    };
    PORT_SetPinInterruptConfig(PORTB, 2, kPORT_InterruptFallingEdge);
    NVIC_SetPriority(PORTB_IRQn, 7);
    EnableIRQ(PORTB_IRQn);
    GPIO_PinInit(GPIOB, 2, &gio2_config);
}

void EInt_GIO2_Disable()
{
    DisableIRQ(PORTB_IRQn);
}

void EInt_GIO2_Enable()
{
	EnableIRQ(PORTB_IRQn);
}

#else
#ifndef DRIVER_RF_PROJECT

#endif 
#ifndef DRIVER_RF_PROJECT

#endif
void PORTC_IRQHandler(void)
{
	BaseType_t xHigherPriorityTaskWoken, xResult;
	xHigherPriorityTaskWoken = pdFALSE;
    GPIO_ClearPinsInterruptFlags(A7139_GIO2_GPIO, 1U << A7139_GIO2_GPIO_PIN);
    if (GIO2S == RX) {
    	GIO2_FLAG  = 1;
    	xResult = xEventGroupSetBitsFromISR(A7139EventGroup,  B_GIO2_FYNC, &xHigherPriorityTaskWoken );
    	if( xResult != pdFAIL )
    	{
    		portYIELD_FROM_ISR( xHigherPriorityTaskWoken );
    	}
    }
    if (GIO2S == TXIN) {
    	GIO2S = TXEND;
    }
}

void EInt_GIO2_Init(void)
{
    PORT_SetPinInterruptConfig(A7139_GIO2_PORT, A7139_GIO2_GPIO_PIN, kPORT_InterruptFallingEdge);
    NVIC_SetPriority(A7139_GIO2_IRQ, 7);
    EnableIRQ(A7139_GIO2_IRQ);
    A7139_GIO2_INIT();
}
    
void EInt_GIO2_Disable()
{
    DisableIRQ(A7139_GIO2_IRQ);
}

void EInt_GIO2_Enable()
{
    EnableIRQ(A7139_GIO2_IRQ);
}
#endif

void SPI_A7139_INIT(void)
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

    NVIC_SetPriority(SPI0_IRQn, 5);

    sourceClock = CLOCK_GetFreq(DSPI0_CLK_SRC);
    status = DSPI_RTOS_Init(&A7139_SPI_Handle, A7139_SPI_MASTER_BASEADDR, &masterConfig, sourceClock);

    if (status != kStatus_Success)
    {
        PRINTF("DSPI master: error during initialization. \r\n");
    }    

    A7139EventGroup = xEventGroupCreate();
	xEventGroupSetBits(A7139EventGroup, B_GIO2_FYNC);
    EInt_GIO2_Init();
}
