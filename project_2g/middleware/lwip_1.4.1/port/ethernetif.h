/*
 * Copyright (c) 2001-2003 Swedish Institute of Computer Science.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 * 3. The name of the author may not be used to endorse or promote products
 *    derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT
 * SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT
 * OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING
 * IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY
 * OF SUCH DAMAGE.
 *
 * This file is part of the lwIP TCP/IP stack.
 *
 * Author: Adam Dunkels <adam@sics.se>
 *
 */

#ifndef ETHERNETIF_H
#define ETHERNETIF_H

#include "lwip/err.h"
#include "fsl_enet.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/
#define ENET_RXBD_NUM (6)
#define ENET_TXBD_NUM (5)
#define ENET_RXBUFF_SIZE (ENET_FRAME_MAX_FRAMELEN)
#define ENET_TXBUFF_SIZE (ENET_FRAME_MAX_FRAMELEN)
#define ENET_BuffSizeAlign(n) ENET_ALIGN(n, ENET_BUFF_ALIGNMENT)
#define ENET_DATA_LENGTH (1000)
#define ENET_TRANSMIT_DATA_NUM (20)
#define ENET_ALIGN(x, align) ((unsigned int)((x) + ((align)-1)) & (unsigned int)(~(unsigned int)((align)-1)))

#if defined(__GNUC__)
#ifndef __ALIGN_END
#define __ALIGN_END __attribute__((aligned(ENET_BUFF_ALIGNMENT)))
#endif
#ifndef __ALIGN_BEGIN
#define __ALIGN_BEGIN
#endif
#else
#ifndef __ALIGN_END
#define __ALIGN_END
#endif
#ifndef __ALIGN_BEGIN
#if defined(__CC_ARM)
#define __ALIGN_BEGIN __align(ENET_BUFF_ALIGNMENT)
#elif defined(__ICCARM__)
#define __ALIGN_BEGIN
#endif
#endif
#endif

#if FSL_FEATURE_ENET_SUPPORT_PTP
#define ENET_PTP_TXTS_RING_LEN (25)
#define ENET_PTP_RXTS_RING_LEN (25)
#define ENET_PTP_L2RING_LEN (15)
#define ENET_PTP_SYNC_MSG (0)
#define ENET_PTP_DELAYREQ_MSG (1)
#define ENET_PTP_ETHERNET_LAYER2_TEST (1)
#define ENET_IPV4_VERSION (4)
#endif
/* MAC address configuration. */
#define configMAC_ADDR0 0x00
#define configMAC_ADDR1 0x12
#define configMAC_ADDR2 0x13
#define configMAC_ADDR3 0x10
#define configMAC_ADDR4 0x15
#define configMAC_ADDR5 0x11

#define ENET_OK (0U)
#define ENET_ERROR (0xffU)
#define ENET_PHY_TIMEOUT (0xFFFFU)

err_t ethernetif_init(struct netif *netif);

#endif
