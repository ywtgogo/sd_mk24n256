/*
 * FreeRTOS_Sockets.h
 *
 *  Created on: 2016Äê11ÔÂ10ÈÕ
 *      Author: yiqiu.yang
 */

#ifndef MQTT_FREERTOS_SOCKETS_H_
#define MQTT_FREERTOS_SOCKETS_H_
#include "sockets.h"
#include "api.h"
#include "netdb.h"
#define FreeRTOS_setsockopt  			setsockopt
#define FreeRTOS_getsockopt				getsockopt
#define FreeRTOS_recv 		 			recv
#define FreeRTOS_closesocket 			close
#define FreeRTOS_socket					socket
#define FreeRTOS_connect				connect
#define	FreeRTOS_gethostbyname 			netconn_gethostbyname
#define FreeRTOS_htons					htons
#define FreeRTOS_send					send

#define FREERTOS_SO_RCVTIMEO 			SO_RCVTIMEO
#define FRERRTOS_SO_SNDTIMEO			SO_SNDTIMEO
#define FRERRTOS_SO_ERROR				SO_ERROR
#define FREERTOS_AF_INET				AF_INET
#define FREERTOS_SOCK_STREAM			SOCK_STREAM
#define FREERTOS_IPPROTO_TCP			IPPROTO_TCP
#define FREERTOS_SOL_SOCKET				SOL_SOCKET

#define freertos_sockaddr 				sockaddr_in



typedef int xSocket_t;

#endif /* MQTT_FREERTOS_SOCKETS_H_ */
