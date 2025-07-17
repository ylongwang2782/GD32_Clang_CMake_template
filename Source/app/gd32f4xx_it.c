/*!
    \file    gd32f4xx_it.c
    \brief   interrupt service routines

    \version 2024-01-17, V2.6.4, demo for GD32F4xx
*/

/*
    Copyright (c) 2024, GigaDevice Semiconductor Inc.

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

#include "gd32f4xx_it.h"

#include <stdio.h>

#include "gd32f4xx.h"

#ifdef MASTER
#include "lwip/sys.h"
#include "netcfg.h"
#include "queue.h"
#include "semphr.h"

extern xSemaphoreHandle g_rx_semaphore;
#endif
/*!
    \brief      this function handles NMI exception
    \param[in]  none
    \param[out] none
    \retval     none
*/
void NMI_Handler(void) {
    /* if NMI exception occurs, go to infinite loop */
    while (1) {
    }
}

/*!
    \brief      this function handles HardFault exception
    \param[in]  none
    \param[out] none
    \retval     none
*/
extern uint32_t __StackTop;      // 栈顶地址（链接脚本定义）
extern uint32_t __StackLimit;    // 栈底地址
void HardFault_Handler(void) {
    __asm volatile(
        "TST lr, #4 \n"
        "ITE EQ \n"
        "MRSEQ r0, MSP \n"
        "MRSNE r0, PSP \n"
        "B HardFault_Handler_C \n");
}

void HardFault_Handler_C(uint32_t *fault_stack_address) {
    uint32_t r0 = fault_stack_address[0];
    uint32_t r1 = fault_stack_address[1];
    uint32_t r2 = fault_stack_address[2];
    uint32_t r3 = fault_stack_address[3];
    uint32_t r12 = fault_stack_address[4];
    uint32_t lr = fault_stack_address[5];
    uint32_t pc = fault_stack_address[6];
    uint32_t psr = fault_stack_address[7];

    printf("HardFault_Handler_C\r\n");
    printf("R0  = 0x%08lX\r\n", r0);
    printf("R1  = 0x%08lX\r\n", r1);
    printf("R2  = 0x%08lX\r\n", r2);
    printf("R3  = 0x%08lX\r\n", r3);
    printf("R12 = 0x%08lX\r\n", r12);
    printf("LR  = 0x%08lX\r\n", lr);
    printf("PC  = 0x%08lX\r\n", pc);
    printf("PSR = 0x%08lX\r\n", psr);
    printf("SCB->HFSR = 0x%08lX\r\n", SCB->HFSR);
    printf("SCB->CFSR = 0x%08lX\r\n", SCB->CFSR);
    printf("SCB->MMFAR= 0x%08lX\r\n", SCB->MMFAR);
    printf("SCB->BFAR = 0x%08lX\r\n", SCB->BFAR);

    uint32_t msp = __get_MSP();
    uint32_t psp = __get_PSP();

    printf("MSP=0x%08lX(limit:0x%08lX), PSP=0x%08lX\n", (unsigned long)msp,
           (unsigned long)&__StackLimit, (unsigned long)psp);
    if (msp < (uint32_t)&__StackLimit) {
        printf("MSP Stack Overflow! Used: %lu bytes over limit\n",
               (unsigned long)((uint32_t)&__StackLimit - msp));
    }

    while (1);
}
/*!
    \brief      this function handles MemManage exception
    \param[in]  none
    \param[out] none
    \retval     none
*/
void MemManage_Handler(void) {
    /* if Memory Manage exception occurs, go to infinite loop */
    while (1) {
    }
}

/*!
    \brief      this function handles BusFault exception
    \param[in]  none
    \param[out] none
    \retval     none
*/
void BusFault_Handler(void) {
    /* if Bus Fault exception occurs, go to infinite loop */
    while (1) {
    }
}

/*!
    \brief      this function handles UsageFault exception
    \param[in]  none
    \param[out] none
    \retval     none
*/
void UsageFault_Handler(void) {
    /* if Usage Fault exception occurs, go to infinite loop */
    while (1) {
    }
}

/*!
    \brief      this function handles DebugMon exception
    \param[in]  none
    \param[out] none
    \retval     none
*/
void DebugMon_Handler(void) {
    /* if DebugMon exception occurs, go to infinite loop */
    while (1) {
    }
}

#ifdef MASTER
/*!
    \brief      this function handles ethernet interrupt request
    \param[in]  none
    \param[out] none
    \retval     none
*/
void ENET_IRQHandler(void) {
    portBASE_TYPE xHigherPriorityTaskWoken = pdFALSE;

    /* frame received */
    if (SET == enet_interrupt_flag_get(ENET_DMA_INT_FLAG_RS)) {
        /* give the semaphore to wakeup LwIP task */
        xSemaphoreGiveFromISR(g_rx_semaphore, &xHigherPriorityTaskWoken);
    }

    /* clear the enet DMA Rx interrupt pending bits */
    enet_interrupt_flag_clear(ENET_DMA_INT_FLAG_RS_CLR);
    enet_interrupt_flag_clear(ENET_DMA_INT_FLAG_NI_CLR);

    /* switch tasks if necessary */
    if (pdFALSE != xHigherPriorityTaskWoken) {
        portEND_SWITCHING_ISR(xHigherPriorityTaskWoken);
    }
}
#endif