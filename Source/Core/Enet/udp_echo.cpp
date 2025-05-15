/*!
    \file    udp_echo.c
    \brief   UDP demo program

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

#include "udp_echo.h"

#include <stdio.h>
#include <string.h>

#include "gd32f4xx.h"
#include "lwip/api.h"
#include "lwip/memp.h"
#include "lwip/opt.h"
#include "lwip/sys.h"
#include "lwip/tcp.h"
#include "main.h"

#define UDP_TASK_PRIO (tskIDLE_PRIORITY + 5)
#define MAX_BUF_SIZE  50

#include "bsp_log.hpp"
extern Logger Log;
#include "lwip/sockets.h"

UdpTask::UdpTask() : TaskClassS<UDP_TASK_DEPTH>("UdpTask", TaskPrio_Mid) {}

void UdpTask::task() {
    Log.v("UDP", "UDP task start");
    int ret, recvnum, sockfd = -1;
    int udp_port = 1025;
    struct sockaddr_in rmt_addr, bod_addr;
    char buf[100];
    u32_t len;
    ip_addr_t ipaddr;

    IP4_ADDR(&ipaddr, IP_S_ADDR0, IP_S_ADDR1, IP_S_ADDR2, IP_S_ADDR3);
    Log.v("UDP", "IP address set: %d.%d.%d.%d", ip4_addr1_16(&ipaddr),
          ip4_addr2_16(&ipaddr), ip4_addr3_16(&ipaddr), ip4_addr4_16(&ipaddr));

    /* set up address to connect to */
    rmt_addr.sin_family = AF_INET;
    Log.v("UDP", "Remote address family set to AF_INET");

    rmt_addr.sin_port = htons(udp_port);
    Log.v("UDP", "Remote port set to %d", udp_port);

    rmt_addr.sin_addr.s_addr = ipaddr.addr;
    Log.v("UDP", "Remote IP address set");

    bod_addr.sin_family = AF_INET;
    Log.v("UDP", "Bind address family set to AF_INET");

    bod_addr.sin_port = htons(udp_port);
    Log.v("UDP", "Bind port set to %d", udp_port);

    bod_addr.sin_addr.s_addr = htons(INADDR_ANY);
    Log.v("UDP", "Bind IP address set to INADDR_ANY");

    Log.v("UDP", "UDP server start");

    while (1) {
        /* create the socket */
        sockfd = socket(AF_INET, SOCK_DGRAM, 0);
        if (sockfd < 0) {
            continue;
        }

        ret = bind(sockfd, (struct sockaddr *)&bod_addr, sizeof(bod_addr));

        if (ret < 0) {
            lwip_close(sockfd);
            sockfd = -1;
            continue;
        }

        len = sizeof(rmt_addr);
        /* reveive packets from rmt_addr, and limit a reception to MAX_BUF_SIZE
         * bytes */
        recvnum = recvfrom(sockfd, buf, MAX_BUF_SIZE, 0,
                           (struct sockaddr *)&rmt_addr, &len);

        while (-1 != sockfd) {
            if (recvnum > 0) {
                /* send packets to rmt_addr */
                sendto(sockfd, buf, recvnum, 0, (struct sockaddr *)&rmt_addr,
                       sizeof(rmt_addr));
            }
            recvnum = recvfrom(sockfd, buf, MAX_BUF_SIZE, 0,
                               (struct sockaddr *)&rmt_addr, &len);
        }

        lwip_close(sockfd);
    }
}
