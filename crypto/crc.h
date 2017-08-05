#ifndef __CRC_H__
#define __CRC_H__  
unsigned short Crc16_Calc(const void  * pbuf, unsigned int len);
unsigned int Crc32_Calc(const void *pbuf, unsigned int len);
#endif 