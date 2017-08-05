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
#include "board.h"

#include "fsl_uart_freertos.h"
#include "fsl_uart.h"
#include "fsl_shell.h"

#include "pin_mux.h"
#include "clock_config.h"
#include "fsl_common.h"
#include "SPI_A7139.h"
#include "hb_protocol.h"
#include "board.h"

extern EventGroupHandle_t FIFO_event;
EventGroupHandle_t FOTA_event = NULL;


static bool get_response(otah_t img,RF_MSG token)
{
  
  int r = 0;
  rf_send_then_receive(&token);
  xEventGroupClearBits(FIFO_event,B_FIFO_event);
  for (r=0;r<3;r++)
   {

     if(xEventGroupWaitBits(FIFO_event,    	/* The event group handle. */
                                        B_FIFO_event,			/* The bit pattern the event group is waiting for. */
                                        pdTRUE,         		/* BIT will be cleared automatically. */
                                        pdTRUE,         		/* Don't wait for both bits, either bit unblock task. */
                                        2000) == pdTRUE)
     {

        if((recv_packet.crc == crc_cal((uint8_t*)&recv_packet.sn_dest, PACKET_CRC_LENGTH)) && recv_packet.cmd == 2 && recv_packet.sn_dest == token.sn_mitt && recv_packet.sn_mitt == token.sn_dest)
        {
          if(img.fwv == recv_packet.fw_major)
          {
            return 0;
          }
          else
          {
            PRINTF("fw %d2\b\b",img.fwv);
            return 1;
          }

        }
        else
        {
          rf_send_then_receive(&token);
        }
     }
      else
      {
        rf_send_then_receive(&token);
      }
   }
  return 1;
}


static bool do_fota_process(otah_t img,RF_MSG token)
{
  token.cmd = RF_CMD_UPDATE_IMAGE;
  rf_send_then_receive(&token);
  vTaskDelay(10);
  rf_send_image(img);
  vTaskDelay(1000);
  token.cmd = RF_CMD_GENERIC_REQUEST;
  return get_response(img,token);
}

uint8_t do_fota(uint32_t id)
{
  
  
  otah_t img_info[2] =
  {
    {
      .lc_addr = 0x32000,
      .rm_addr = 0x4400,
      .len = 0x3470,
      .fwv = 3,
    },
    {
      .lc_addr = 0x39800,
      .rm_addr = 0x0800,
      .len = 0x3470,
      .fwv = 2,
    }
  };
  RF_MSG fota_token = { 0x00, 0x00, 0x00, 0x14};
  fota_token.sn_dest = id;
  fota_token.sn_mitt = 0x00ABEDEF;
  
  for(int j=0;j<2;j++)
  {
    if(do_fota_process(img_info[j],fota_token) == 1)
    {
      return 1;
    }
    vTaskDelay(100);
  }
  
  return 0;
  


  

}