#define PR_MODULE "gprs\1"

#include "fsl_uart_freertos.h"
#include "fsl_debug_console.h"
#include "board.h"
#include "uart_ppp.h"
#include "modem_socket.h"
#include "pppif.h"
#include "sockets.h"

extern uart_rtos_handle_t ppp_handle;
extern const char *send_ring_overrun;
extern const char *send_hardware_overrun;
extern ppp_gprs_info_conf ppp_gprs_info;

extern CHAR at_cmd_mipcall[32];
extern CHAR at_cmd_mipcall_down[32];
extern CHAR at_cmd_mipopen[96];
extern CHAR at_cmd_mipclose[32];
extern CHAR at_cmd_mipdns[64];
extern CHAR at_cmd_mipdns_result[64];
extern CHAR at_cmd_mipsend[2048];
extern CHAR at_cmd_mipread[32];
extern CHAR at_cmd_mipread_result[24];
extern CHAR at_cmd_mippush[32];
extern CHAR at_cmd_mippush_result[16];

#define in_range(c, lo, up)  ((u8_t)c >= lo && (u8_t)c <= up)
#define isprint(c)           in_range(c, 0x20, 0x7f)
#define isdigit(c)           in_range(c, '0', '9')
#define isxdigit(c)          (isdigit(c) || in_range(c, 'a', 'f') || in_range(c, 'A', 'F'))
#define islower(c)           in_range(c, 'a', 'z')
#define ishigher(c)           in_range(c, 'A', 'Z')
#define isspace(c)           (c == ' ' || c == '\f' || c == '\n' || c == '\r' || c == '\t' || c == '\v')

u32_t curr_modem_socket_index = 0xff;
CHAR  curr_socket_url[96];
struct netif netif_default;
CHAR dns_ip_addr[16];
//CHAR socket_recv_data[2080];

MODEM_SOCKET modem_socket_buf = {
	.modem_socket_cache[0].socket_index = 0,
	.modem_socket_cache[0].socket_index_flag = MODEM_SOCKET_NOT_USED,
	.modem_socket_cache[0].socket_proto_type = MODEM_SOCKET_NONE,
	.modem_socket_cache[1].socket_index = 1,
	.modem_socket_cache[1].socket_index_flag = MODEM_SOCKET_NOT_USED,
	.modem_socket_cache[1].socket_proto_type = MODEM_SOCKET_NONE,
	.modem_socket_cache[2].socket_index = 2,
	.modem_socket_cache[2].socket_index_flag = MODEM_SOCKET_NOT_USED,
	.modem_socket_cache[2].socket_proto_type = MODEM_SOCKET_NONE,
	.modem_socket_cache[3].socket_index = 3,
	.modem_socket_cache[3].socket_index_flag = MODEM_SOCKET_NOT_USED,
	.modem_socket_cache[3].socket_proto_type = MODEM_SOCKET_NONE,
};

/**
 *hao.yunran
 *2017.3.23
 * Convert an u16_t from host- to network byte order.
 *
 * @param n u16_t in host byte order
 * @return n in network byte order
 */
u16_t modem_htons(u16_t n)
{
  return ((n & 0xff) << 8) | ((n & 0xff00) >> 8);
}

/**
 *hao.yunran
 *2017.3.23
 * Convert an u16_t from network- to host byte order.
 *
 * @param n u16_t in network byte order
 * @return n in host byte order
 */
u16_t modem_ntohs(u16_t n)
{
  return modem_htons(n);
}

/**
 *hao.yunran
 *2017.3.23
 * Convert an u32_t from host- to network byte order.
 *
 * @param n u32_t in host byte order
 * @return n in network byte order
 */
u32_t modem_htonl(u32_t n)
{
  return ((n & 0xff) << 24) |
    ((n & 0xff00) << 8) |
    ((n & 0xff0000UL) >> 8) |
    ((n & 0xff000000UL) >> 24);
}

/**
 *hao.yunran
 *2017.3.23
 * Convert an u32_t from network- to host byte order.
 *
 * @param n u32_t in network byte order
 * @return n in host byte order
 */
u32_t modem_ntohl(u32_t n)
{
  return modem_htonl(n);
}

/**
 * Ascii internet address interpretation routine.
 * The value returned is in network order.
 *
 * @param cp IP address in ascii represenation (e.g. "127.0.0.1")
 * @return ip address in network order
 */
u32_t modem_ipaddr_addr(const char *cp)
{
  ip_addr_t val;

  if (modem_ipaddr_aton(cp, &val)) {
    return ip4_addr_get_u32(&val);
  }
  return (IPADDR_NONE);
}

/**
 * Check whether "cp" is a valid ascii representation
 * of an Internet address and convert to a binary address.
 * Returns 1 if the address is valid, 0 if not.
 * This replaces inet_addr, the return value from which
 * cannot distinguish between failure and a local broadcast address.
 *
 * @param cp IP address in ascii represenation (e.g. "127.0.0.1")
 * @param addr pointer to which to save the ip address in network order
 * @return 1 if cp could be converted to addr, 0 on failure
 */
int modem_ipaddr_aton(const char *cp, ip_addr_t *addr)
{
	u32_t val;
	u8_t base;
	char c;
	u32_t parts[4];
	u32_t *pp = parts;

	c = *cp;
	for (;;) 
	{
		/*
		 * Collect number up to ``.''.
		 * Values are specified as for C:
		 * 0x=hex, 0=octal, 1-9=decimal.
		 */
		if (!isdigit(c))
		{
			return (0);
		}
		val = 0;
		base = 10;
		if (c == '0') 
		{
			c = *++cp;
			if (c == 'x' || c == 'X') 
			{
				base = 16;
				c = *++cp;
			} 
			else
			{
				base = 8;
			}
		}
		for (;;) 
		{
			if (isdigit(c)) 
			{
				val = (val * base) + (int)(c - '0');
				c = *++cp;
			} 
			else if (base == 16 && isxdigit(c)) 
			{
				val = (val << 4) | (int)(c + 10 - (islower(c) ? 'a' : 'A'));
				c = *++cp;
			} 
			else
			{
				break;
			}
		}
		if (c == '.') 
		{
			/*
			* Internet format:
			*  a.b.c.d
			*  a.b.c   (with c treated as 16 bits)
			*  a.b (with b treated as 24 bits)
			*/
			if (pp >= parts + 3) 
			{
				return (0);
			}
			*pp++ = val;
			c = *++cp;
		} 
		else
		{
			break;
		}
	}
	/*
	* Check for trailing characters.
	*/
	if (c != '\0' && !isspace(c)) 
	{
		return (0);
	}
	/*
	* Concoct the address according to
	* the number of parts specified.
	*/
	switch (pp - parts + 1) 
	{

		case 0:
			return (0);       /* initial nondigit */

		case 1:             /* a -- 32 bits */
			break;

		case 2:             /* a.b -- 8.24 bits */
			if (val > 0xffffffUL) {
			  return (0);
			}
			val |= parts[0] << 24;
			break;

		case 3:             /* a.b.c -- 8.8.16 bits */
			if (val > 0xffff) {
			  return (0);
			}
			val |= (parts[0] << 24) | (parts[1] << 16);
			break;

		case 4:             /* a.b.c.d -- 8.8.8.8 bits */
			if (val > 0xff) {
			  return (0);
			}
			val |= (parts[0] << 24) | (parts[1] << 16) | (parts[2] << 8);
			break;
		default:
			//LWIP_ASSERT("unhandled", 0);
			break;
	}
	if (addr) 
	{
		ip4_addr_set_u32(addr, modem_htonl(val));
	}
	return (1);
}

/**
 * Convert numeric IP address into decimal dotted ASCII representation.
 * returns ptr to static buffer; not reentrant!
 *
 * @param addr ip address in network order to convert
 * @return pointer to a global static (!) buffer that holds the ASCII
 *         represenation of addr
 */
char * modem_ipaddr_ntoa(const ip_addr_t *addr)
{
  static char str[16];
  return modem_ipaddr_ntoa_r(addr, str, 16);
}

/*
*hao.yunran
*2017.3.23
*
*/
char *modem_ipaddr_ntoa_r(const ip_addr_t *addr, char *buf, int buflen)
{
  u32_t s_addr;
  char inv[3];
  char *rp;
  u8_t *ap;
  u8_t rem;
  u8_t n;
  u8_t i;
  int len = 0;

  s_addr = ip4_addr_get_u32(addr);

  rp = buf;
  ap = (u8_t *)&s_addr;
  for(n = 0; n < 4; n++) {
    i = 0;
    do {
      rem = *ap % (u8_t)10;
      *ap /= (u8_t)10;
      inv[i++] = '0' + rem;
    } while(*ap);
    while(i--) {
      if (len++ >= buflen) {
        return NULL;
      }
      *rp++ = inv[i];
    }
    if (len++ >= buflen) {
      return NULL;
    }
    *rp++ = '.';
    ap++;
  }
  *--rp = 0;
  return buf;
}

/*
*hao.yunran
*2017.3.23
*
*/
s32_t modem_gethostbyname(const char *name, ip_addr_t *addr)
{
	s32_t ret = RET_SOCKET_DNS_FAIL;
	u32_t i=0;
	char *url = name;
	char mipdns_format[] = "AT+MIPDNS=\"%s\"\r";
	char mipdns_format_result[] = "+MIPDNS: \"%s\",";
	
	PRINTF("..modem_gethostbyname...%s..\r\n",name);
	if(pdFALSE == xSemaphoreTake(call_http_mqtt_sem, 1000U))
	{
		PRINTF("..modem_gethostbyname..take sem fail..\r\n");
		return RET_SOCKET_DNS_FAIL;
	}
	if(name == NULL)
	{
		xSemaphoreGive(call_http_mqtt_sem);
		return RET_SOCKET_DNS_FAIL;
	}
	memset(&curr_socket_url, 0, sizeof(curr_socket_url));
	strncpy(curr_socket_url, url, strlen(url));
	sprintf(at_cmd_mipdns, mipdns_format, url);
	sprintf(at_cmd_mipdns_result, mipdns_format_result, url);
	
	//PRINTF("..,,,,,,,,,,,...0x%x.......\r\n",&netif_default.ip_addr);
	
	//PRINTF("..,,,,,,,,,,,...0x%x.......\r\n",netif_default);
	ret = modem_at_cmd_mipdns(&ppp_handle, AT_REPEAT_MIN, AT_TIMEOUT_MIPDNS_MAX, WAIT_RETURN);
	if(ret == AT_RET_OK)
	{
		PRINTF("..dns_ip_addr.%s..\r\n",dns_ip_addr);
		for(i=15;i>0;i--)
		{
			if(!isdigit(dns_ip_addr[i]))
			{
				dns_ip_addr[i]='\n';
			}
			else
			{
				break;
			}
		}
		
		modem_ipaddr_aton(dns_ip_addr, &netif_default.ip_addr);
		modem_ipaddr_aton(dns_ip_addr, addr);
		//PRINTF("..,,,,,,,,,,,,,,,,,,ip_addr.0x%x..\r\n",netif_default.ip_addr);
		ret = RET_SOCKET_DNS_OK;
	}
	else
	{
		ret = RET_SOCKET_DNS_FAIL;
	}
	PRINTF("..modem_gethostbyname..ret=%d..\r\n",ret);
	xSemaphoreGive(call_http_mqtt_sem);
	return ret;
}

/*
*hao.yunran
*2017.3.23
*
*/
s32_t modem_socket(int domain, int type, int protocol)
{
	s32_t ret = RET_SOCKET_FAIL;
	u32_t modem_local_port = MODEM_LOCAL_PORT_OFFSET;
	u32_t i = 0;
	u32_t proto_type = 0;
	char mipopen_format[] = "AT+MIPOPEN=%d,%d,\"%s\",15590,%d\r";
	
	PRINTF("..modem_socket..\r\n");
	if(pdFALSE == xSemaphoreTake(call_http_mqtt_sem, 1000U))
	{
		PRINTF("..modem_socket..take sem fail..\r\n");
		return RET_SOCKET_FAIL;
	}
	
	if(MQTT_SOCKET_TCP == protocol)
	{
		proto_type = MODEM_SOCKET_TCP;
	}
	else if(MQTT_SOCKET_UDP == protocol)
	{
		proto_type = MODEM_SOCKET_UDP;
	}
	
	for(i=0;i<4;i++)
	{
		if(modem_socket_buf.modem_socket_cache[i].socket_index_flag == MODEM_SOCKET_NOT_USED)
		{
			//PRINTF("..modem_socket.domain=%d.type=%d..protocol=%d...index=%d...i=%d.\r\n",domain,type,protocol,modem_socket_buf.modem_socket_cache[i].socket_index,i);
			modem_socket_buf.modem_socket_cache[i].socket_index_flag = MODEM_SOCKET_USED;
			curr_modem_socket_index = modem_socket_buf.modem_socket_cache[i].socket_index;
			modem_socket_buf.modem_socket_cache[i].socket_proto_type = proto_type;
			ret = RET_SOCKET_OK;
			break;
		}
		else
		{
			ret = RET_SOCKET_FAIL;
		}
	}
	if(ret == RET_SOCKET_FAIL)
	{
		PRINTF("..modem_socket..fail..\r\n");
		xSemaphoreGive(call_http_mqtt_sem);
		return RET_SOCKET_FAIL;
	}
	else
	{
		ret = curr_modem_socket_index;
	}
	PRINTF("..modem_socket..ok..index=%d..\r\n",ret);
	xSemaphoreGive(call_http_mqtt_sem);
	return ret;
}

/*
*hao.yunran
*2017.3.23
*
*/
s32_t modem_closesocket(int s)
{
	s32_t ret = RET_SOCKET_FAIL;
	u32_t socket_index = MODEM_SOCKET_NONE;
	char mipclose_format[] = "AT+MIPCLOSE=%d\r";
	PRINTF("..modem_closesocket..%d..\r\n",s);

	if(pdFALSE == xSemaphoreTake(call_http_mqtt_sem, 1000U))
	{
		PRINTF("..modem_closesocket..take sem fail..\r\n");
		return RET_SOCKET_FAIL;
	}
	if(s < 0)
	{
		PRINTF("..modem_closesocket..socket index err..\r\n");
		xSemaphoreGive(call_http_mqtt_sem);
		return RET_SOCKET_FAIL;
	}
	socket_index = s+1;
	sprintf(at_cmd_mipclose, mipclose_format, socket_index);
	ret = modem_at_cmd_mipclose(&ppp_handle, AT_REPEAT_NOR, AT_TIMEOUT_MIPCLOSE_MAX, WAIT_RETURN);
	if(ret == AT_RET_OK)
	{
		modem_socket_buf.modem_socket_cache[(socket_index-1)].socket_index_flag = MODEM_SOCKET_NOT_USED;
		
		PRINTF("..modem_closesocket..ok..index=%d..\r\n",s);
		xSemaphoreGive(call_http_mqtt_sem);		
		return RET_SOCKET_OK;
	}
	else
	{
		ret = RET_SOCKET_FAIL;
	}
	PRINTF("..modem_closesocket..fail..\r\n");
	xSemaphoreGive(call_http_mqtt_sem);
	return ret;
}

/*
*hao.yunran
*2017.3.23
*
*/
s32_t modem_connect(int s, const struct sockaddr *name, socklen_t namelen)
{
	s32_t ret = RET_SOCKET_FAIL;
	u32_t modem_local_port = MODEM_LOCAL_PORT_OFFSET;
	u32_t socket_index = s;
	u32_t proto_type = 0;
	u32_t socket_port = 0;
	const struct sockaddr_in *sAddr = (const struct sockaddr_in *)(void*)name;
	char mipopen_format[] = "AT+MIPOPEN=%d,%d,\"%s\",%d,%d\r";
	
	PRINTF("..modem_connect..\r\n");
	if(pdFALSE == xSemaphoreTake(call_http_mqtt_sem, 1000U))
	{
		PRINTF("..modem_connect..take sem fail..\r\n");
		return RET_SOCKET_FAIL;
	}
	if(s < 0)
	{
		PRINTF("..modem_connect..socket index err..\r\n");
		xSemaphoreGive(call_http_mqtt_sem);
		return RET_SOCKET_FAIL;
	}
	modem_local_port = modem_socket_buf.modem_socket_cache[socket_index].socket_index + MODEM_LOCAL_PORT_OFFSET;
	socket_port = modem_ntohs(sAddr->sin_port);
	proto_type = modem_socket_buf.modem_socket_cache[socket_index].socket_proto_type;

	sprintf(at_cmd_mipopen, mipopen_format, (socket_index+1), modem_local_port, curr_socket_url, socket_port, proto_type);

	ret = modem_at_cmd_mipopen(&ppp_handle, AT_REPEAT_MIN, AT_TIMEOUT_MIPOPEN_MAX, WAIT_RETURN);
	if(ret == AT_RET_OK)
	{		
		PRINTF("..modem_connect ok....socket_index=%d..\r\n",socket_index);
		xSemaphoreGive(call_http_mqtt_sem);
		return RET_SOCKET_OK;
	}
	else
	{
		PRINTF("..modem_connect fail..socket_index=%d.\r\n",socket_index);
		ret = RET_SOCKET_FAIL;
	}
	xSemaphoreGive(call_http_mqtt_sem);
	return ret;
}


/*
*hao.yunran
*2017.3.23
*
*/
s32_t modem_disconnect(int socket_n)
{
	s32_t ret = RET_SOCKET_FAIL;
	PRINTF("..modem_disconnect..index=%d..\r\n",socket_n);
	if(socket_n < 0)
	{
		PRINTF("..modem_disconnect..index err..\r\n");
		return RET_SOCKET_FAIL;
	}
	ret = modem_closesocket(socket_n);
	return ret;
}

/*
*hao.yunran
*2017.3.23
*
*/
//u8_t modem_send_buf[1048];
s32_t modem_write(int socket_n, unsigned char* buffer, int len, int timeout_ms)
{
	s32_t ret = RET_SOCKET_FAIL;
	unsigned char *temp_buf = buffer;
	char mipsend_format[] = "AT+MIPSEND=%d,\"88\"\r";
	char mippush_format[] = "AT+MIPPUSH=%d\r";
	char mippush_format_ret[] = "+MIPPUSH: %d,0";
	int len_send=0;

	PRINTF("..modem_write..index=%d..len=%d..timeout=%d..\r\n",socket_n,len,timeout_ms);

	if(pdFALSE == xSemaphoreTake(call_http_mqtt_sem, 1000U))
	{
		PRINTF("..modem_write..take sem fail..\r\n");
		return RET_SOCKET_FAIL;
	}
	if(socket_n < 0)
	{
		PRINTF("..modem_write..socket index err..\r\n");
		xSemaphoreGive(call_http_mqtt_sem);
		return RET_SOCKET_FAIL;
	}
	if(modem_socket_buf.modem_socket_cache[(socket_n)].socket_index_flag == MODEM_SOCKET_NOT_USED)
	{
		PRINTF("..modem_write..socket index not used..\r\n");
		xSemaphoreGive(call_http_mqtt_sem);
		return RET_SOCKET_FAIL;
	}
	
	if((buffer == NULL)||(len>1024))
	{
		xSemaphoreGive(call_http_mqtt_sem);
		return RET_SOCKET_FAIL;
	}
	
	memset(&at_cmd_mipsend, 0, sizeof(at_cmd_mipsend));
	sprintf(at_cmd_mipsend, mipsend_format, socket_n+1);

	for(len_send=0;len_send<len;len_send++)
	{
		at_cmd_mipsend[14+len_send] = buffer[len_send];
	}
	
	for(len_send=(len-1);len_send>=0;len_send--)
	{
		if((at_cmd_mipsend[14+len_send]&0xf)<0xa)
		{
			at_cmd_mipsend[(14+len_send*2)+1] = (at_cmd_mipsend[14+len_send]&0xf)+'0';
		}
		else
		{
			at_cmd_mipsend[(14+len_send*2)+1] = ((at_cmd_mipsend[14+len_send]&0xf)-0xa)+'a';
		}
		
		if(((at_cmd_mipsend[14+len_send]&0xf0)>>4)<0xa)
		{
			at_cmd_mipsend[(14+len_send*2)] = ((at_cmd_mipsend[14+len_send]&0xf0)>>4)+'0';
		}
		else 
		{
			at_cmd_mipsend[(14+len_send*2)] = (((at_cmd_mipsend[14+len_send]&0xf0)>>4)-0xa)+'a';
		}
	}
	at_cmd_mipsend[14+len*2] = '"';
	at_cmd_mipsend[15+len*2] = '\r';
	
	//PRINTF("..modem_write..%d..\r\n",(len*2+16));
	//PRINTF("..modem_write....%s....\r\n",at_cmd_mipsend);
	ret = modem_at_cmd_mipsend(&ppp_handle, AT_REPEAT_MIN, (len*2+16), timeout_ms, WAIT_RETURN);
	if(ret == AT_RET_OK)
	{
		//PRINTF("..modem_send..ok..\r\n");
		memset(&at_cmd_mippush, 0, sizeof(at_cmd_mippush));
		memset(&at_cmd_mippush_result, 0, sizeof(at_cmd_mippush_result));
		sprintf(at_cmd_mippush, mippush_format, socket_n+1);
		sprintf(at_cmd_mippush_result, mippush_format_ret, socket_n+1);
		
		//PRINTF("..mippush_result...%s..\r\n",at_cmd_mippush_result);
		ret = modem_at_cmd_mippush(&ppp_handle, AT_REPEAT_MIN, timeout_ms, WAIT_RETURN);
		if(ret == AT_RET_OK)
		{
			PRINTF("..send ok......\r\n");
			ret = len;
		}
		else
		{
			PRINTF("..push fail......\r\n");
			ret = RET_SOCKET_FAIL;
		}
	}
	else 
	{
		PRINTF("..send fail......\r\n");
		ret = RET_SOCKET_FAIL;
	}
	xSemaphoreGive(call_http_mqtt_sem);
	return ret;
}

/*
*hao.yunran
*2017.3.23
*
*/
s32_t modem_read(int socket_n, unsigned char* buffer, int len, int timeout_ms)
{
	s32_t ret = RET_SOCKET_FAIL;
	u32_t read_time = xTaskGetTickCount();
	u32_t i=0;
	u32_t len_ret=0;
	unsigned char *temp_buf = buffer;
	char mipread_format[] = "AT+MIPREAD=%d,%d\r";
	char mipread_format_ret[] = "+MIP";

	PRINTF("..modem_read..index=%d..len=%d..timeout=%d..\r\n",socket_n,len,timeout_ms);
	if(pdFALSE == xSemaphoreTake(call_http_mqtt_sem, 1000U))
	{
		PRINTF("..modem_read..take sem fail..\r\n");
		return RET_SOCKET_FAIL;
	}
	if(socket_n < 0)
	{
		PRINTF("..modem_read..socket index err..\r\n");
		xSemaphoreGive(call_http_mqtt_sem);
		return RET_SOCKET_FAIL;
	}
	if(modem_socket_buf.modem_socket_cache[(socket_n)].socket_index_flag == MODEM_SOCKET_NOT_USED)
	{
		PRINTF("..modem_read..socket index not used..\r\n");
		xSemaphoreGive(call_http_mqtt_sem);
		return RET_SOCKET_FAIL;
	}
	if(buffer == NULL)
	{
		xSemaphoreGive(call_http_mqtt_sem);
		return RET_SOCKET_FAIL;
	}
	memset(&at_cmd_mipread, 0, sizeof(at_cmd_mipread));
	sprintf(at_cmd_mipread, mipread_format, socket_n+1, len);
	
	//memset(&at_cmd_mipread_result, 0, sizeof(at_cmd_mipread_result));
	//sprintf(at_cmd_mipread_result, mipread_format_ret, socket_n+1);
	//memset(&socket_recv_data, 0, sizeof(socket_recv_data));
	read_time = xTaskGetTickCount();
	while((xTaskGetTickCount() - read_time) < timeout_ms)
	{
		//PRINTF("..modem_read..\r\n");
		//ret = modem_at_cmd_mipread(&ppp_handle, AT_REPEAT_MIN, timeout_ms, WAIT_RETURN, sizeof(socket_recv_data), socket_recv_data, &len_ret);
		ret = modem_at_cmd_mipread(&ppp_handle, AT_REPEAT_MIN, AT_TIMEOUT_NOR, WAIT_RETURN, len, buffer, &len_ret);
		if(ret == AT_RET_OK)
		{
			PRINTF("..modem_read..ok..len=%d..\r\n",len_ret);
			ret = len_ret;
			break;
		}
		else
		{
			//PRINTF("..modem_read..fail..\r\n");
			ret = RET_SOCKET_FAIL;
		}
		vTaskDelay(3000);
	}
	
	xSemaphoreGive(call_http_mqtt_sem);
	return ret;
}







