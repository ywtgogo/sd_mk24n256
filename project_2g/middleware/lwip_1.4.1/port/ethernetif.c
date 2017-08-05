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

#include <string.h>
#include "lwip/mem.h"
#include "lwip/pbuf.h"
#include <lwip/stats.h>
#include <lwip/snmp.h>
#include "netif/etharp.h"
#include "netif/ppp_oe.h"
#include "ethernetif.h"

#include "fsl_enet.h"
#include "fsl_phy.h"

///////////////////////////////////////////////////////////////////////////////
// Definitions
///////////////////////////////////////////////////////////////////////////////
/* Define those to better describe your network interface. */
#define IFNAME0 'e'
#define IFNAME1 'n'

/*******************************************************************************
 * Variables
 ******************************************************************************/
enet_handle_t g_handle;

#if defined(__ICCARM__)
#pragma data_alignment = ENET_BUFF_ALIGNMENT
#endif
__ALIGN_BEGIN enet_rx_bd_struct_t RxBuffDescrip[ENET_RXBD_NUM] __ALIGN_END;
#if defined(__ICCARM__)
#pragma data_alignment = ENET_BUFF_ALIGNMENT
#endif
__ALIGN_BEGIN enet_tx_bd_struct_t TxBuffDescrip[ENET_TXBD_NUM] __ALIGN_END;

#if defined(__ICCARM__)
#pragma data_alignment = ENET_BUFF_ALIGNMENT
#endif
__ALIGN_BEGIN uint8_t RxDataBuff[ENET_RXBD_NUM][ENET_BuffSizeAlign(ENET_RXBUFF_SIZE)] __ALIGN_END;

#if defined(__ICCARM__)
#pragma data_alignment = ENET_BUFF_ALIGNMENT
#endif
__ALIGN_BEGIN uint8_t TxDataBuff[ENET_TXBD_NUM][ENET_BuffSizeAlign(ENET_TXBUFF_SIZE)] __ALIGN_END;

/**
 * In this function, the hardware should be initialized.
 * Called from ethernetif_init().
 *
 * @param netif the already initialized lwip network interface structure
 *        for this ethernetif
 */
void ethernetif_input(enet_handle_t *handle, void *param);

void ethernet_callback(ENET_Type *base, enet_handle_t *handle, enet_event_t event, void *param)
{
    switch (event)
    {
        case kENET_RxEvent:
        ethernetif_input(handle, param);
        break;
        default:
        break;
    }
}

err_t low_level_init(struct netif *netif)
{
    /* set MAC hardware address length */
    netif->hwaddr_len = ETHARP_HWADDR_LEN;

    /* set MAC hardware address */
    netif->hwaddr[0] = configMAC_ADDR0;
    netif->hwaddr[1] = configMAC_ADDR1;
    netif->hwaddr[2] = configMAC_ADDR2;
    netif->hwaddr[3] = configMAC_ADDR3;
    netif->hwaddr[4] = configMAC_ADDR4;
    netif->hwaddr[5] = configMAC_ADDR5;

    /* maximum transfer unit */
    netif->mtu = 1500; // TODO: define a config

    /* don't set NETIF_FLAG_ETHARP if this device is not an ethernet one */
    netif->flags = NETIF_FLAG_BROADCAST | NETIF_FLAG_ETHARP | NETIF_FLAG_LINK_UP;

    enet_config_t config;
    uint32_t sysClock;
    uint32_t phyAddr = 0;
    bool link = false;
    phy_speed_t speed;
    phy_duplex_t duplex;
    uint32_t count = 0;
    /* prepare the buffer configuration. */
    enet_buffer_config_t buffCfg = {
        ENET_RXBD_NUM,
        ENET_TXBD_NUM,
        ENET_BuffSizeAlign(ENET_RXBUFF_SIZE),
        ENET_BuffSizeAlign(ENET_TXBUFF_SIZE),
        &RxBuffDescrip[0],
        &TxBuffDescrip[0],
        &RxDataBuff[0][0],
        &TxDataBuff[0][0],
    };

    sysClock = CLOCK_GetFreq(kCLOCK_CoreSysClk);

    ENET_GetDefaultConfig(&config);
    PHY_Init(ENET, 0, sysClock);

    while ((count < ENET_PHY_TIMEOUT) && (!link))
    {
        PHY_GetLinkStatus(ENET, phyAddr, &link);
        if (link)
        {
            /* Get the actual PHY link speed. */
            PHY_GetLinkSpeedDuplex(ENET, phyAddr, &speed, &duplex);
            /* Change the MII speed and duplex for actual link status. */
            config.miiSpeed = (enet_mii_speed_t)speed;
            config.miiDuplex = (enet_mii_duplex_t)duplex;
            config.interrupt = kENET_RxFrameInterrupt;
        }

        count++;
    }

    if (count == ENET_PHY_TIMEOUT)
    {
        LWIP_ASSERT("\r\nPHY Link down, please check the cable connection.\r\n", 0);
    }
    ENET_Init(ENET, &g_handle, &config, &buffCfg, netif->hwaddr, sysClock);
    ENET_SetCallback(&g_handle, ethernet_callback, netif);
    ENET_ActiveRead(ENET);
    return ENET_OK;
}

/**
 * This function should do the actual transmission of the packet. The packet is
 * contained in the pbuf that is passed to the function. This pbuf
 * might be chained.
 *
 * @param netif the lwip network interface structure for this ethernetif
 * @param p the MAC packet to send (e.g. IP packet including MAC addresses and type)
 * @return ERR_OK if the packet could be sent
 *         an err_t value if the packet couldn't be sent
 *
 * @note Returning ERR_MEM here if a DMA queue of your MAC is full can lead to
 *       strange results. You might consider waiting for space in the DMA queue
 *       to become availale since the stack doesn't retry to send a packet
 *       dropped because of memory failure (except for the TCP timers).
 */

static err_t low_level_output(struct netif *netif, struct pbuf *packetBuffer)
{
    struct pbuf *packetTempBuffer;
    bool link = false;
    static unsigned char ucBuffer[ENET_FRAME_MAX_FRAMELEN];
    unsigned char *pucBuffer = ucBuffer;
    unsigned char *pucChar;
    uint32_t phyAddr = 0;

    LWIP_ASSERT("Output packet buffer empty", packetBuffer);

    /* Initiate transfer. */
    if (packetBuffer->len == packetBuffer->tot_len)
    {
        /* No pbuf chain, don't have to copy -> faster. */
        pucBuffer = &((unsigned char *)packetBuffer->payload)[ETH_PAD_SIZE];
    }
    else
    {
        /* pbuf chain, copy into contiguous ucBuffer. */
        if (packetBuffer->tot_len >= ENET_FRAME_MAX_FRAMELEN)
        {
            return ERR_BUF;
        }
        else
        {
            pucChar = ucBuffer;

            for (packetTempBuffer = packetBuffer; packetTempBuffer != NULL; packetTempBuffer = packetTempBuffer->next)
            {
                /* Send the data from the pbuf to the interface, one pbuf at a
                time. The size of the data in each pbuf is kept in the ->len
                variable. */
                /* send data from(packetTempBuffer->payload, packetTempBuffer->len); */
                if (packetTempBuffer == packetBuffer)
                {
                    memcpy(pucChar, &((char *)packetTempBuffer->payload)[ETH_PAD_SIZE],
                           packetTempBuffer->len - ETH_PAD_SIZE);
                    pucChar += packetTempBuffer->len - ETH_PAD_SIZE;
                }
                else
                {
                    memcpy(pucChar, packetTempBuffer->payload, packetTempBuffer->len);
                    pucChar += packetTempBuffer->len;
                }
            }
        }
    }

    /* Send a multicast frame when the PHY is link up. */
    if (kStatus_Success == PHY_GetLinkStatus(ENET, phyAddr, &link))
    {
        if (link)
        {
            if (kStatus_Success == ENET_SendFrame(ENET, &g_handle, pucBuffer, packetBuffer->tot_len - ETH_PAD_SIZE))
            {
                return ERR_OK;
            }
        }
    }
    return ERR_IF;
}

/**
 * This function should be called when a packet is ready to be read
 * from the interface. It uses the function low_level_input() that
 * should handle the actual reception of bytes from the network
 * interface. Then the type of the received packet is determined and
 * the appropriate input function is called.
 *
 * @param netif the lwip network interface structure for this ethernetif
 */
void ethernetif_input(enet_handle_t *handle, void *param)
{
    struct eth_hdr *ethhdr;
    status_t status;
    uint32_t length = 0;
    struct netif *netif = param;
    enet_data_error_stats_t eErrStatic;

    /* Check input parameter */
    if (netif == NULL)
    {
        LWIP_ASSERT("Input param is NULL", 0);
        /* Should not reach this statement if error occurs */
        return;
    }
    /* Read all data from ring buffer and send to uper layer */
    do
    {
        /* Get the Frame size */
        status = ENET_GetRxFrameSize(handle, &length);

        /* Call ENET_ReadFrame when there is a received frame. */
        if (length != 0)
        {
            uint8_t *receivedDataBuffer;
            struct pbuf *packetBuffer;
            packetBuffer = pbuf_alloc(PBUF_RAW, length, PBUF_POOL);
            /* Received valid frame. Deliver the rx buffer with the size equal to length. */
            if ((packetBuffer == NULL) || (packetBuffer->payload == NULL))
            {
                LWIP_ASSERT("Fail to allocate new memory space", 0);
                /* Should not reach this statement if error occurs */
                return;
            }
            receivedDataBuffer = packetBuffer->payload;
            packetBuffer->len = (u16_t)length;
            ENET_ReadFrame(ENET, handle, receivedDataBuffer, packetBuffer->len);

            /* points to packet payload, which starts with an Ethernet header */
            ethhdr = packetBuffer->payload;
            switch (htons(ethhdr->type))
            {
                /* IP or ARP packet? */
                case ETHTYPE_IP:
                case ETHTYPE_ARP:
#if PPPOE_SUPPORT
                /* PPPoE packet? */
                case ETHTYPE_PPPOEDISC:
                case ETHTYPE_PPPOE:
#endif /* PPPOE_SUPPORT */
                    /* full packet send to tcpip_thread to process */
                    if (netif->input(packetBuffer, netif) != ERR_OK)
                    {
                        LWIP_DEBUGF(NETIF_DEBUG, ("ethernetif_input: IP input error\n"));
                        pbuf_free(packetBuffer);
                        packetBuffer = NULL;
                    }
                    break;

                default:
                    pbuf_free(packetBuffer);
                    packetBuffer = NULL;
                    break;
            }
        }
        else
        {
            /* Update the received buffer when error happened. */
            if (status != kStatus_Success)
            {
                /* Get the error information of the received g_frame. */
                ENET_GetRxErrBeforeReadFrame(handle, &eErrStatic);
                /* update the receive buffer. */
                ENET_ReadFrame(ENET, handle, NULL, 0);
            }
        }
    } while (kStatus_ENET_RxFrameEmpty != status);
}

/**
 * Should be called at the beginning of the program to set up the
 * network interface. It calls the function low_level_init() to do the
 * actual setup of the hardware.
 *
 * This function should be passed as a parameter to netif_add().
 *
 * @param netif the lwip network interface structure for this ethernetif
 * @return ERR_OK if the loopif is initialized
 *         ERR_MEM if private data couldn't be allocated
 *         any other err_t on error
 */
err_t ethernetif_init(struct netif *netif)
{
    err_t result;
    LWIP_ASSERT("netif != NULL", (netif != NULL));

#if LWIP_NETIF_HOSTNAME
    /* Initialize interface hostname */
    netif->hostname = "lwip";
#endif /* LWIP_NETIF_HOSTNAME */

    /*
    * Initialize the snmp variables and counters inside the struct netif.
    * The last argument should be replaced with your link speed, in units
    * of bits per second.
    */
    NETIF_INIT_SNMP(netif, snmp_ifType_ethernet_csmacd, LINK_SPEED_OF_YOUR_NETIF_IN_BPS);

    netif->name[0] = IFNAME0;
    netif->name[1] = IFNAME1;
    /* We directly use etharp_output() here to save a function call.
    * You can instead declare your own function an call etharp_output()
    * from it if you have to do some checks before sending (e.g. if link
    * is available...) */
    netif->output = etharp_output;
    netif->linkoutput = low_level_output;
    /* initialize the hardware */
    result = low_level_init(netif);
    NVIC_SetPriority(ENET_Receive_IRQn, 6U);
#ifdef ENET_ENHANCEDBUFFERDESCRIPTOR_MODE
    NVIC_SetPriority(ENET_Transmit_IRQn, 6U);
    NVIC_SetPriority(ENET_1588_Timer_IRQn, 6U);
#endif
    return result;
}
