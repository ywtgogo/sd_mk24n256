#define PR_MODULE "gprs\1"

#include "fsl_uart_freertos.h"
#include "fsl_debug_console.h"
#include "board.h"
#include "uart_ppp.h"

extern uart_rtos_handle_t ppp_handle;
extern const char *send_ring_overrun;
extern const char *send_hardware_overrun;
extern ppp_gprs_info_conf ppp_gprs_info;

status_t  At_ReadData_from_Modem( uint8_t *data, size_t length)
{
	size_t n;
	uint32_t error;
	error = UART_RTOS_Receive(&ppp_handle, data, length, &n);
    if (error == kStatus_UART_RxHardwareOverrun)
    {
        /* Notify about hardware buffer overrun */
        if (kStatus_Success != UART_RTOS_Send(&ppp_handle, (uint8_t *)send_hardware_overrun, strlen(send_hardware_overrun)))
        {
            PRINTF("SendError Error\r\n");
        }
    }
    if (error == kStatus_UART_RxRingBufferOverrun)
    {
        /* Notify about ring buffer overrun */
        if (kStatus_Success != UART_RTOS_Send(&ppp_handle, (uint8_t *)send_ring_overrun, strlen(send_ring_overrun)))
        {
            PRINTF("SendError Error\r\n");
        }
    }      
	return error;
}

status_t  At_ReadData_from_Uart( void *fd, uint8_t *data, size_t length)
{
	size_t n;
	uint32_t error;
	error = UART_RTOS_Receive(&ppp_handle, data, length, &n);
    if (error == kStatus_UART_RxHardwareOverrun)
    {
        /* Notify about hardware buffer overrun */
        if (kStatus_Success != UART_RTOS_Send(&ppp_handle, (uint8_t *)send_hardware_overrun, strlen(send_hardware_overrun)))
        {
            PRINTF("SendError Error\r\n");
        }
    }
    if (error == kStatus_UART_RxRingBufferOverrun)
    {
        /* Notify about ring buffer overrun */
        if (kStatus_Success != UART_RTOS_Send(&ppp_handle, (uint8_t *)send_ring_overrun, strlen(send_ring_overrun)))
        {
            PRINTF("SendError Error\r\n");
        }
    }
	return error;
}

int  At_SendData_to_Modem(void *device, const uint8_t *data, size_t length)
{
	int rc;
	//PRINTF("SendData start\r\n");
	if ((rc = UART_RTOS_Send(&ppp_handle, data, length)) == kStatus_Success){
		//PPP_GREEN_LED_TOGGLE();
		//PRINTF("SendData end\r\n");
		if(ppp_gprs_info.ppp_uart_current_counter == K24_UART_CURRENT_COUNT_MAX)
		{
			ppp_gprs_info.ppp_uart_current_counter = 0;
		}
		else
		{
			ppp_gprs_info.ppp_uart_current_counter++;
		}
		return length;
	}else{
		PPP_GREEN_LED_OFF();
		PPP_RED_LED_TOGGLE();
		//PRINTF("..At_SendData_to_Modem.....end1....\r\n ");
        PRINTF("SendData Error %d\r\n", rc);
		if(ppp_gprs_info.ppp_uart_current_counter == K24_UART_CURRENT_COUNT_MAX)
		{
			ppp_gprs_info.ppp_uart_current_counter = 0;
		}
		else
		{
			ppp_gprs_info.ppp_uart_current_counter++;
		}
		return 0;
	}
}


