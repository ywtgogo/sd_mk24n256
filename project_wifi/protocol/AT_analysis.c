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

#include <stdlib.h>
#include <string.h>

#include "AT_execute.h"


#define AT_LINE_MAX		(4096+2048)

uart_rtos_handle_t at_handle;
struct _uart_handle at_t_handle;

uint8_t at_background_buffer[4096+2048];
uint8_t	recv_buffer[AT_LINE_MAX];

struct rtos_uart_config at_config = {
    .baudrate = BOARD_DEBUG_UART_BAUDRATE,
    .parity = kUART_ParityDisabled,
    .stopbits = kUART_OneStopBit,
    .buffer = at_background_buffer,
    .buffer_size = sizeof(at_background_buffer),
};


int cmd_nordata_analyze(char **p_data)
{
	char		*p = *p_data;
	char 		*p_run=p;
	char		c;
	int			col;
	uint32_t	addr;

	for(;;)
	{
		c = *p++;
		if (c == '\0') {
			PRINTF("%s\r\n", p_run);
			*p_data = p_run;
			//return p_data-p;
			return addr;
		}else if (c == ','){
			col = p-p_run;
			p_run[col-1] = '\0';
			addr = atoi(p_run);
			PRINTF("%d ", addr);
			p_run = p;
		}
	}
}

int cmd_data_analyze(char *p_data)
{
	char		*p = p_data;
	char		c;
	int			col;
	uint32_t	addr;

	for(;;)
	{
		c = *p_data++;
		if (c == '\0') {
			PRINTF("%s\r\n", p);
			return p_data-p;
		}
	}
}

int cmd_analyze(char *p_buf)
{
	char		*p = p_buf;
	char		c;
	int			col;
	for(;;)
	{
		c = *p_buf++;
		if (c == '+') {
			if (0 == strncmp(p, "AT+", p_buf-p))
				p = p_buf;
			else
				return -1;
		}else if (c == '=') {
			if (0 == strncmp(p, "HWTEST=", p_buf-p)){
				p = p_buf;
				col = cmd_data_analyze(p_buf);
				HWTEST(p_buf, col);
			}else if (0 == strncmp(p, "HWQUERY=", p_buf-p)){
				p = p_buf;
				col = cmd_data_analyze(p_buf);
				HWQUERY(p_buf, col);
			}else if (0 == strncmp(p, "SETNORDATA=", p_buf-p)){
				p = p_buf;
				col = cmd_nordata_analyze(&p_buf);
				SETNORDATA(p_buf, col);
			}else
				return -1;
		}else if (c == '\0')
			return -1;

	}
}

int get_info_pc(char *buffer)
{
	char		*p;
	char 		*delim = " ";
	int			n=1;
	uint16_t	blk_pc;

	//printf("%s ", strtok(buffer, delim));
	p = strtok(buffer, delim);
	if (0==strcmp("PRODUCEINFO", p)) {
		//PRINTF("PRODUCEINFO");
		if (p=strtok(NULL, delim)) {
			//PRINTF("%s\r\n", p);
			AtRunMsg.cmd = HW_PRODUCEINFO;
			AtRunMsg.data= p;
			xQueueSend(at_run_handle, &AtRunMsg, portMAX_DELAY);
		}
	}else if (0==strcmp("NORDATAIMG", p)) {
		//PRINTF("NORDATAIMG\r\n");
		if (p=strtok(NULL, delim)) {
			//PRINTF("%d\r\n", blk_pc=atoi(p));
			AtRunMsg.blk_num = atoi(p);
			//PRINTF("%d\r\n", AtRunMsg.blk_num);
			p=strtok(NULL, delim);
			//PRINTF("%s\r\n", p);
			AtRunMsg.b64_len = strlen(p);
			AtRunMsg.cmd = HW_NORDATAIMG;
			AtRunMsg.data= p;
			xQueueSend(at_run_handle, &AtRunMsg, portMAX_DELAY);
		}
	}else if (0==strcmp("NORDATAAUDIO", p)) {
		//PRINTF("NORDATAAUDIO\r\n");
		if (p=strtok(NULL, delim)) {
			//PRINTF("%d\r\n", atoi(p));
			AtRunMsg.blk_num = atoi(p);
			p=strtok(NULL, delim);
			//PRINTF("%s\r\n", p);
			AtRunMsg.b64_len = strlen(p);
			AtRunMsg.cmd = HW_NORDATAAUDIO;
			AtRunMsg.data= p;
			xQueueSend(at_run_handle, &AtRunMsg, portMAX_DELAY);
		}
	}else if (0==strcmp("LONGID", p)) {
		if (p=strtok(NULL, delim)) {
			AtRunMsg.cmd = HW_LONGID;
			AtRunMsg.data= p;
			xQueueSend(at_run_handle, &AtRunMsg, portMAX_DELAY);
		}
	}else if (0==strcmp("FREQ", p)) {
		if (p=strtok(NULL, delim)) {
			//PRINTF("%s\r\n", p);
			AtRunMsg.cmd = HW_FREQ;
			AtRunMsg.data= p;
			xQueueSend(at_run_handle, &AtRunMsg, portMAX_DELAY);
		}
	}else if (0==strcmp("START", p)) {
		//PRINTF("START\r\n");
		AtRunMsg.cmd = HW_START;
		xQueueSend(at_run_handle, &AtRunMsg, portMAX_DELAY);
	}else if (0==strcmp("RESET", p)) {
		//PRINTF("START\r\n");
		AtRunMsg.cmd = HW_RESET;
		xQueueSend(at_run_handle, &AtRunMsg, portMAX_DELAY);
	}else{
		PRINTF("ERRCMD\r\n");
		return -1;
	}

	return 1;
//    while((p = strtok(NULL, delim)))
//    {
//    	printf("%s ", p);
//    }
//
//        printf("\n");

//	for(;;)
//	{
//		c = *p_buf++;
//		if (c == '+') {
//			if (0 == strncmp(p, "AT+", p_buf-p))
//				p = p_buf;
//			else
//				return -1;
//		}else if (c == '=') {
//			if (0 == strncmp(p, "HWTEST=", p_buf-p)){
//				p = p_buf;
//				col = cmd_data_analyze(p_buf);
//				HWTEST(p_buf, col);
//			}else if (0 == strncmp(p, "HWQUERY=", p_buf-p)){
//				p = p_buf;
//				col = cmd_data_analyze(p_buf);
//				HWQUERY(p_buf, col);
//			}else if (0 == strncmp(p, "SETNORDATA=", p_buf-p)){
//				p = p_buf;
//				col = cmd_nordata_analyze(&p_buf);
//				SETNORDATA(p_buf, col);
//			}else
//				return -1;
//		}else if (c == '\0')
//			return -1;
//
//	}
}

int at_get_line(char *buffer)
{
	char 	*p = buffer;
	char 	*p_buf = p;
    int     	n = 0;                          /* buffer index         */
    //int     	col;                            /* output column cnt    */
    char    	c;
    int 	error;

    for (;;) {

    	error = UART_RTOS_Receive(&at_handle, (uint8_t*)&c, 1, NULL);
        if (error == kStatus_UART_RxHardwareOverrun)
        {
        	PRINTF("HWOVERRUN\r\n");
        }
        if (error == kStatus_UART_RxRingBufferOverrun)
        {
        	PRINTF("RBOVERRUN\r\n");;
        }

		/*
		 * Special character handling
		 */
		switch (c) {

		case '\r':                      /* Enter                */
		case '\n':
				if (n==0) continue;
				*p = '\0';
				return p - p_buf;

		default:
			/*
			 * Must be a normal character then
			 */
			if (n < AT_LINE_MAX) {
				   *p++ = c;
					++n;
			}
		}
    }
}

void at_uart_task(void *pvParameters)
{
    int n;
    at_config.srcclk = CLOCK_GetFreq(CONSOLE_UART_CLKSRC);
    at_config.base = CONSOLE_UART;
    NVIC_SetPriority(CONSOLE_UART_RX_TX_IRQn, 5);

    if (0 > UART_RTOS_Init(&at_handle, &at_t_handle, &at_config))
    {
        PRINTF("Error during at UART initialization.\r\n");
        vTaskSuspend(NULL);
    }

    while(1)
    {
		n = at_get_line((char*)recv_buffer);
		if (n > 0)
		{
			//PRINTF("%s\r\n", recv_buffer);
			get_info_pc((char*)recv_buffer);
			//cmd_analyze((char*)recv_buffer);
		}
    }
}
