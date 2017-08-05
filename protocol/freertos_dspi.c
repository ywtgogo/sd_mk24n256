/*
* Copyright (c) 2013 - 2015, Freescale Semiconductor, Inc.
* All rights reserved.
*
* Redistribution and use in source and binary forms, with or without modification,
* are permitted provided that the following conditions are met:
*
* o Redistributions of source code must retain the above copyright notice, this list
*   of conditions and the following disclaimer.
*
* o Redistributions in binary form must reproduce the above copyright notice, this
*   list of conditions and the following disclaimer in the documentation and/or
*   other materials provided with the distribution.
*
* o Neither the name of Freescale Semiconductor, Inc. nor the names of its
*   contributors may be used to endorse or promote products derived from this
*   software without specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
* ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
* WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
* DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR
* ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
* (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
* LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
* ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
* (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
* SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/
#define PR_MODULE "rf\1"

/* FreeRTOS kernel includes. */
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "timers.h"

/* Freescale includes. */
#include "fsl_device_registers.h"
#include "fsl_debug_console.h"
#include "fsl_common.h"
#include "A7139.h"
#include "hb_protocol.h"
#include "SPI_A7139.h"
#include "message.h"
#include "event_groups.h"
#include "fsl_gpio.h"

#include "board.h"

/*******************************************************************************
* Definitions
******************************************************************************/
/*******************************************************************************
* Variables
******************************************************************************/
extern void SPI_A7139_INIT(void);
extern EventGroupHandle_t A7139EventGroup;
#ifdef PRODUCE_TEST
extern RF_MSG HwQueryMsg;
#endif
/*******************************************************************************
 * Prototypes
 ******************************************************************************/
static void DelayBlinkRfCallback(TimerHandle_t xTimer)
{
	//RF_LED_ON();
    vTaskDelay(50);
    //RF_LED_OFF();
}

void rf_message_send_msqg(uint8_t *buf)
{
	xQueueSend(rx_msg_queue, buf, portMAX_DELAY);
}

void rf_get_message_task(void *pvParameters)
{
	TimerHandle_t 	DelayBlinkRfHandle = NULL;
	RF_MSG msg;

	init_A7139();

	RF_LED_INIT();
	//PPP_LED_INIT();
    vTaskDelay(200);
    RF_LED_OFF();
    //PPP_LED_OFF();


    DelayBlinkRfHandle = xTimerCreate("delay_blink_rf",     /* Text name. */
                                 ((TickType_t)50U), 			/* Timer period. */
							   	 pdFALSE,                   /* Enable auto reload. */
                                 0,                         /* ID is not used. */
								 DelayBlinkRfCallback);    /* The callback function. */

#if 0	/* set test packet */
	send_packet.fw_major = 2;
	send_packet.fw_minor = 0;
	send_packet.header = 0x14;
	send_packet.sn_dest = (0x0a<<28)|0x0AAAAAAA;
	send_packet.sn_mitt = 0x11111111;
	send_packet.cmd = 0x01;
	send_packet.enable_cmd = 0;
	send_packet.sequence = 32;

	//rf_send_then_receive(&send_packet);
#endif

#if 0 	/* test send packet */
	while(1) {
		send_packet.sn_dest = (0x0a<<28)|0x0AAAbbbb;
		rf_send_then_receive(&send_packet);//_no_preamble
		//vTaskDelay(100);
	}
#endif
//    aes256_self_test();
//    spi_flash_init();
    while(1)
    {

    	xEventGroupWaitBits(A7139EventGroup,    	/* The event group handle. */
    						B_GIO2_FYNC,			/* The bit pattern the event group is waiting for. */
							pdTRUE,         		/* BIT will be cleared automatically. */
							pdTRUE,         		/* Don't wait for both bits, either bit unblock task. */
							portMAX_DELAY); 		/* Block indefinitely to wait for the condition to be met. */

        if ((GIO2S == RX) && GIO2_FLAG)
        {
        	xTimerStart(DelayBlinkRfHandle, 0);
        	xSemaphoreTake(rf_message_sem, portMAX_DELAY);

            GIO2_FLAG  = 0;
            A7139_StrobeCmd(CMD_RFR);
            A7139_ReadFIFO((uint8_t *)&msg.header, PACKET_CONTENT_LENGTH);
            msg.rssi = RSSI_measurement();
            A7139_StrobeCmd(CMD_RX);
        	xSemaphoreGive(rf_message_sem);
#ifdef PRODUCE_TEST
        	memcpy(&HwQueryMsg, &msg, sizeof(RF_MSG));
#endif
//    		PRINTF("encrypt !\r\n");
//            for(uint32_t j=0;j<PACKET_CONTENT_LENGTH;j++)
//    		{
//    			PRINTF("%02x ", (uint8_t)(*((uint8_t *)&msg.header+j)));
//    		}
        	//RF_LED_TOGGLE();
#ifndef PRODUCE_TEST
        	if (msg.header == 0x16)
        	{
				if (Crc16_Calc((uint8_t*)&msg.sn_dest, PACKET_CRC_LENGTH + 2) != msg.crc2) {

					PRINTF("\r\nRF_RECV CRC2 ERROR! %x\r\n", Crc16_Calc((uint8_t*)&msg.sn_dest, PACKET_CRC_LENGTH + 2));
				}else{
						rf_message_send_msqg((uint8_t *)&msg);
				}
        	}
        	else if (msg.header == 0x14)
        	{
        		if (Crc16_Calc((uint8_t*)&msg.sn_dest, PACKET_CRC_LENGTH) != msg.crc) {

					PRINTF("\r\nRF_RECV CRC ERROR!\r\n");
				}else{

					rf_message_send_msqg((uint8_t *)&msg);
				}
        	}
        	else
        		PRINTF("RF_RECV error protocol\r\n");
//    		PRINTF("decrypt !\r\n");
//
//            for(uint32_t j=0;j<16;j++)
//    		{
//    			PRINTF("%02x ", out[j]);
//    		}
#endif
#if 0
            static	uint8_t k = 0;
            for(uint32_t j=0;j<21;j++)
    		{
    			PRINTF("%02x ", (uint8_t)(*((uint8_t *)&msg.header+j)));
    		}
    		PRINTF("\r\n");
    		if (k == 5) {
    			send_packet.sn_dest = (0x0a<<28)|0x0AAAbbbb;
    			rf_send_then_receive_no_preamble(&send_packet);
    			k = 0;
    		}
    		k++;
            A7139_StrobeCmd(CMD_RX);
#endif

        }
    }
}

/*!
 * @brief Application entry point.  configMAX_PRIORITIES + 1
 */
//int rf_spi_init(void)
//{
//    PRINTF("FreeRTOS DSPI example start.\r\n");
//    if (xTaskCreate(rf_get_message_task, "Rf_get_message_task", configMINIMAL_STACK_SIZE + 64, NULL, configMAX_PRIORITIES - 1, NULL) != pdPASS)
//    {
//        PRINTF("Failed to create Rf_get_message_task");
//        vTaskSuspend(NULL);
//    }
//    return 1;
//}



