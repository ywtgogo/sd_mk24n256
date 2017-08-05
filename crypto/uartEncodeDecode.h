#ifndef __UART_ENCODE_DECODE_H__
#define __UART_ENCODE_DECODE_H__  

#define UART_FRAME_FLAG 0x7e /* 标志字符 */
#define UART_FRAME_ESC  0x7d /* 转义字符 */
#define UART_FRAME_ENC  0x20 /* 编码字符 */

int uartEncode(const void *iBuf, int iLen, unsigned char * oBuf);
int uartDecode(const void *iBuf, int iLen, unsigned char * oBuf);

#endif