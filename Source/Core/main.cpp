#include <cstdint>
#include <cstdio>

#include "TaskCPP.h"
#include "TaskCpp.h"
#include "bsp_led.h"
#include "bsp_log.hpp"
#include "bsp_uid.h"

#ifdef __cplusplus
extern "C" {
#endif
#include "FreeRTOS.h"
#include "task.h"

#ifdef __cplusplus
}
#endif

extern UasrtInfo usart1_info;
UartConfig uart1Conf(usart1_info);
Uart uart1(uart1Conf);

class LedBlinkTask : public TaskClassS<1024> {
   public:
    LedBlinkTask() : TaskClassS<1024>("LedBlinkTask", TaskPrio_Mid) {}

    void task() override {
        Logger &log = Logger::getInstance();
        LED led0(GPIOC, GPIO_PIN_6);

        for (;;) {
            led0.toggle();
            TaskBase::delay(500);
        }
    }
};

class UsartDMATask : public TaskClassS<1024> {
   public:
    UsartDMATask() : TaskClassS<1024>("UsartDMATask", TaskPrio_Mid) {}

    void task() override {
        Logger &log = Logger::getInstance();
        for (;;) {
            // 等待 DMA 完成信号
            if (xSemaphoreTake(usart1_info.dmaRxDoneSema, portMAX_DELAY) == pdPASS) {
                log.d("Usart recv.");
                uint8_t buffer[DMA_RX_BUFFER_SIZE];
                uint16_t len =
                    uart1.getReceivedData(buffer, DMA_RX_BUFFER_SIZE);
                if (len > 0) {
                    // dma tx the data recv
                    uart1.send(buffer, len);
                }
            }
        }
    }
};

class LogTask : public TaskClassS<1024> {
   public:
    LogTask() : TaskClassS<1024>("LogTask", TaskPrio_Mid) {}

    void task() override {
        char buffer[LOG_QUEUE_SIZE + 8];
        Logger &log = Logger::getInstance();
        for (;;) {
            if (xQueueReceive(log.logQueue, buffer, portMAX_DELAY)) {
                for (const char *p = buffer; *p; ++p) {
                    while (RESET == usart_flag_get(USART_LOG, USART_FLAG_TBE));
                    usart_data_transmit(USART_LOG, (uint8_t)*p);
                }
            }
        }
    }
};

LedBlinkTask ledBlinkTask;
UsartDMATask usartDMATask;
LogTask logTask;

int main(void) {
    nvic_priority_group_set(NVIC_PRIGROUP_PRE4_SUB0);
    vTaskStartScheduler();
    for (;;);
}
