#ifndef _MODEM_AT_H_
#define _MODEM_AT_H_


void At_UART_EnableInterrupts();
void At_UART_DisableInterrupts();
status_t  At_ReadData_from_Modem( uint8_t *data, size_t length);
status_t  At_ReadData_from_Uart( void *fd, uint8_t *data, size_t length);
void  At_SendCmd_to_Modem(const uint8_t *data, size_t length);
int  At_SendData_to_Modem(void *device, const uint8_t *data, size_t length);


#endif
