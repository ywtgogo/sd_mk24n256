#ifndef  __KL02_SPI_A7139_H__
#define  __KL02_SPI_A7139_H__

#include "fsl_dspi.h"
#include "fsl_dspi_freertos.h"
#include "fsl_gpio.h"
#include "event_groups.h"

#define  LOW  0
#define  HIGH 1

extern dspi_master_config_t A7139MasterConfig;

void SPI_A7139_INIT(void);
void EInt_GIO2_Disable();
void EInt_GIO2_Enable();

#define RX 		3
#define TXIN 	1
#define TXEND 	2
extern uint32_t   GIO2_FLAG;
extern uint32_t   GIO2S;

extern EventGroupHandle_t A7139EventGroup;

#define B_GIO2_FYNC 	(1 << 0)
#define B_FIFO_event 	(1 << 0)


#endif

