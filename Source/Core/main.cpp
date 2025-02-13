#include <cstdio>

#include "TaskCPP.h"
#include "TaskCpp.h"
#include "bsp_led.h"
#include "bsp_log.h"
#include "bsp_uid.h"

#ifdef __cplusplus
extern "C" {
#endif
#include "FreeRTOS.h"
#include "task.h"

#ifdef __cplusplus
}
#endif

// 全局信号量
extern SemaphoreHandle_t dmaCompleteSemaphore;

// 创建 USART_DMA_Handler 实例
extern UasrtConfig usart1_info;
USART_DMA_Handler usartDMA = USART_DMA_Handler(usart1_info);

class LedBlinkTask : public TaskClassS<1024> {
   public:
    LedBlinkTask() : TaskClassS<1024>("LedBlinkTask", TaskPrio_Mid) {}

    void task() override {
        Logger &log = Logger::getInstance();
        LED led0(GPIOC, GPIO_PIN_6);

        for (;;) {
            log.d("ledBlinkTask!");
            led0.toggle();
            TaskBase::delay(500);
        }
    }
};

class UsartDMATask : public TaskClassS<1024> {
   public:
    UsartDMATask() : TaskClassS<1024>("LedBlinkTask", TaskPrio_Mid) {}

    void task() override {
        Logger &log = Logger::getInstance();
        // 创建信号量
        dmaCompleteSemaphore = xSemaphoreCreateBinary();
        log.d("Usart DMA task start.");
        for (;;) {
            // 等待 DMA 完成信号
            if (xSemaphoreTake(dmaCompleteSemaphore, portMAX_DELAY) == pdPASS) {
                log.d("Usart recv.");
            }
        }
    }
};

class LogTask : public TaskClassS<1024> {
   public:
    LogTask() : TaskClassS<1024>("LedBlinkTask", TaskPrio_Mid) {}

    void task() override {
        char buffer[LOG_QUEUE_SIZE + 8];
        Logger &log = Logger::getInstance();
        for (;;) {
            if (xQueueReceive(log.logQueue, buffer, portMAX_DELAY)) {
                // TODO 更换为dma发送
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
