/*!
    \file    netconf.c
    \brief   network connection configuration

    \version 2024-01-17, V2.6.4, demo for GD32F4xx
*/

/*
    Copyright (c) 2024, GigaDevice Semiconductor Inc

    Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

    1. Redistributions of source code must retain the above copyright notice,
this list of conditions and the following disclaimer.
    2. Redistributions in binary form must reproduce the above copyright notice,
       this list of conditions and the following disclaimer in the documentation
       and/or other materials provided with the distribution.
    3. Neither the name of the copyright holder nor the names of its
contributors may be used to endorse or promote products derived from this
software without specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR
ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "netconf.h"

#include <stdio.h>

#include "Logger.h"
#include "enet_hal.h"
#include "ethernetif.h"
#include "lwip/dhcp.h"
#include "lwip/errno.h"
#include "lwip/mem.h"
#include "lwip/memp.h"
#include "netcfg.h"
#include "queue.h"
#include "tcpip.h"

// 使用新的静态Logger接口

int EthDevice::init() {
    int ret;
    if (!initialized) {
        /* configure ethernet (GPIOs, clocks, MAC, DMA) */
        ret = Enet::enet_system_setup();
        if (0 != ret) {
            Log::e("ETH", "enet_system_setup failed, exiting eth_device_init");
            return 1;
        }
        Log::t("ETH", "ethernet initialized");

        /* initilaize the LwIP stack */
        lwip_stack_init();
        Log::t("ETH", "lwip stack initialized");
        initialized = true;
    }
    return 0;
}

void EthDevice::lwip_netif_status_callback(struct netif *netif) {
    // Log::t("LWIP", "netif status changed: %d", netif->flags);
    // // logd addr
    // Log::t("LWIP", "netif addr: %d.%d.%d.%d", ip4_addr1_16(&netif->ip_addr),
    //       ip4_addr2_16(&netif->ip_addr), ip4_addr3_16(&netif->ip_addr),
    //       ip4_addr4_16(&netif->ip_addr));
    // if (((netif->flags & NETIF_FLAG_UP) != 0) && (0 != netif->ip_addr.addr))
    // {
    //     /* initilaize the udp: echo 1025 */
    //     Log::t("LWIP", "udp echo initialized");
    // }
}

/*!
    \brief      initializes the LwIP stack
    \param[in]  none
    \param[out] none
    \retval     none
*/
void EthDevice::lwip_stack_init(void) {
    ip_addr_t ipaddr;
    ip_addr_t netmask;
    ip_addr_t gw;

    /* create tcp_ip stack thread */
    tcpip_init(NULL, NULL);
    Log::t("LWIP", "tcpip_init initialized");

    /* IP address setting */
    IP4_ADDR(&ipaddr, IP_ADDR0, IP_ADDR1, IP_ADDR2, IP_ADDR3);
    Log::t("LWIP", "static ip address set to %d.%d.%d.%d", ip4_addr1_16(&ipaddr),
          ip4_addr2_16(&ipaddr), ip4_addr3_16(&ipaddr), ip4_addr4_16(&ipaddr));
    IP4_ADDR(&netmask, NETMASK_ADDR0, NETMASK_ADDR1, NETMASK_ADDR2,
             NETMASK_ADDR3);
    Log::t("LWIP", "netmask set to %d.%d.%d.%d", ip4_addr1_16(&netmask),
          ip4_addr2_16(&netmask), ip4_addr3_16(&netmask),
          ip4_addr4_16(&netmask));
    IP4_ADDR(&gw, GW_ADDR0, GW_ADDR1, GW_ADDR2, GW_ADDR3);
    Log::t("LWIP", "gateway set to %d.%d.%d.%d", ip4_addr1_16(&gw),
          ip4_addr2_16(&gw), ip4_addr3_16(&gw), ip4_addr4_16(&gw));

    netif_add(&g_mynetif, &ipaddr, &netmask, &gw, NULL, &ethernetif_init,
              &tcpip_input);
    Log::t("LWIP", "ethernet interface added");

    /* registers the default network interface */
    netif_set_default(&g_mynetif);
    Log::t("LWIP", "default network interface set");
    netif_set_status_callback(&g_mynetif, lwip_netif_status_callback);
    Log::t("LWIP", "network interface status callback set");

    /* when the netif is fully configured this function must be called */
    netif_set_up(&g_mynetif);
    Log::t("LWIP", "network interface set up");
}
