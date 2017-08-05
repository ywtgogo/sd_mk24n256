#ifndef _MODEM_AT_H_
#define _MODEM_AT_H_
#include "MQTTFreeRTOS.h"
#include "ip_addr.h"
#include "netbuf.h"
#include "err.h"
#include "netif.h"

typedef unsigned   char    u8_t;
typedef signed     char    s8_t;
typedef unsigned   short   u16_t;
typedef signed     short   s16_t;
typedef unsigned   int    u32_t;
typedef signed     int    s32_t;
typedef char          CHAR;

#define MODEM_LOCAL_PORT_MIN             2000
#define MODEM_LOCAL_PORT_OFFSET          8964
#define MODEM_LOCAL_PORT_MAX             65535

#define MODEM_SOCKET_NOT_USED            0
#define MODEM_SOCKET_USED                1
#define MODEM_SOCKET_OFFSET              1024

#define MQTT_SOCKET_TCP                  6
#define MQTT_SOCKET_UDP                  17
#define MODEM_SOCKET_TCP                 0
#define MODEM_SOCKET_UDP                 1
#define MODEM_SOCKET_NONE                0xff

#define RET_SOCKET_OK               	 0
#define RET_SOCKET_FAIL                  -1

#define RET_SOCKET_DNS_FAIL              -6
#define RET_SOCKET_DNS_OK                0

typedef struct modem_socket_type_t
{
    u32_t	socket_index;
    u32_t	socket_index_flag;
    u32_t	socket_proto_type;
	CHAR 	socket_url[96];
}MODEM_SOCKET_TYPE;

typedef struct modem_socket_t
{
    MODEM_SOCKET_TYPE      modem_socket_cache[4];
}MODEM_SOCKET;

u16_t modem_htons(u16_t n);
char *modem_ipaddr_ntoa_r(const ip_addr_t *addr, char *buf, int buflen);
s32_t modem_gethostbyname(const char *name, ip_addr_t *addr);
s32_t modem_socket(int domain, int type, int protocol);
s32_t modem_closesocket(int s);
s32_t modem_connect(int s, const struct sockaddr *name, socklen_t namelen);
s32_t modem_disconnect(int socket_n);
s32_t modem_write(int socket_n, unsigned char* buffer, int len, int timeout_ms);
s32_t modem_read(int socket_n, unsigned char* buffer, int len, int timeout_ms);


#endif
