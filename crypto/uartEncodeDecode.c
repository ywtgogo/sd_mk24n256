#include "uartEncodeDecode.h"


/*
	iBuf:input buf
	iLen:input buf len
	oBuf:ouput buf  
	NOTE:oBuf'len = (iLen*2)
*/
int uartEncode(const void *iBuf, int iLen, unsigned char * oBuf)
{
	unsigned char * ip, *op;
	int oLen, i;
	ip = (unsigned char *)iBuf;
	op = oBuf;
	oLen = iLen;

	for (i = 0; i < iLen; i++)
	{
		if ((UART_FRAME_FLAG == *ip) || (UART_FRAME_ESC == *ip) || (UART_FRAME_ENC > *ip))
		{
			*op = UART_FRAME_ESC;
			*op++;
			oLen++;
			*op = *ip ^ UART_FRAME_ENC;
		}
		else
		{
			*op = *ip;
		}

		*ip++;
		*op++;
	}
	return oLen;

}

/*
iBuf:input buf
iLen:input buf len
oBuf:ouput buf
NOTE:oBuf'len = iLen
*/
int uartDecode(const void *iBuf, int iLen, unsigned char * oBuf)
{
	unsigned char * ip, *op;
	int oLen, i;
	ip = (unsigned char*)iBuf;
	op = oBuf;
	oLen = iLen;

	for (i = 0; i < iLen; i++)
	{
		if (UART_FRAME_ESC == *ip)
		{
			ip++;
			i++;
			oLen--;
			*op = *ip ^ UART_FRAME_ENC;

		}
		else
		{
			*op = *ip;
		}
		*op++;
		*ip++;
	}
	return oLen;
}