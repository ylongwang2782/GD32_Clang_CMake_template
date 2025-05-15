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

#include "TaskCPP.h"
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
    int rmt_port = 8080;
    int bod_port = 8080;
    struct sockaddr_in rmt_addr, bod_addr;
    char buf[100];
    u32_t len;
    ip_addr_t ipaddr;
    std::vector<uint8_t> tx_buffer;

    IP4_ADDR(&ipaddr, IP_S_ADDR0, IP_S_ADDR1, IP_S_ADDR2, IP_S_ADDR3);

    rmt_addr.sin_family = AF_INET;
    rmt_addr.sin_port = htons(rmt_port);
    rmt_addr.sin_addr.s_addr = ipaddr.addr;

    bod_addr.sin_family = AF_INET;
    bod_addr.sin_port = htons(bod_port);
    bod_addr.sin_addr.s_addr = htons(INADDR_ANY);

    Log.v("UDP", "UDP server start");

    send_buf.resize(100);
    for (int i = 0; i < 100; i++) {
        send_buf[i] = i;
    }

    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    
    if (sockfd < 0) {
        vTaskDelete(nullptr);
        return;
    }

    if (bind(sockfd, (struct sockaddr *)&bod_addr, sizeof(bod_addr)) < 0) {
        lwip_close(sockfd);
        vTaskDelete(nullptr);
        return;
    }

    len = sizeof(rmt_addr);
    while (1) {
        // if (recvnum > 0) {
        //     /* send packets to rmt_addr */
        //     sendto(sockfd, buf, recvnum, 0, (struct sockaddr *)&rmt_addr,
        //            sizeof(rmt_addr));
        // }
        // recvnum = recvfrom(sockfd, buf, MAX_BUF_SIZE, 0,
        //                    (struct sockaddr *)&rmt_addr, &len);

        sendto(sockfd, send_buf.data(), send_buf.size(), 0,
               (struct sockaddr *)&rmt_addr, sizeof(rmt_addr));

        TaskBase::delay(500);
    }

    lwip_close(sockfd);
}
