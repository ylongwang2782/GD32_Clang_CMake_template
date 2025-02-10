#include <cstdio>

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

void usartDMATask(void *pvParameters);
void ledBlinkTask(void *pvParameters);
void logTask(void *pvParameters);

// 全局信号量
extern SemaphoreHandle_t dmaCompleteSemaphore;

LED led0(GPIOC, GPIO_PIN_6);

// 创建 USART_DMA_Handler 实例
extern UasrtConfig usart1_info;
USART_DMA_Handler usartDMA = USART_DMA_Handler(usart1_info);

int main(void) {
    nvic_priority_group_set(NVIC_PRIGROUP_PRE4_SUB0);

    UIDReader &uid = UIDReader::getInstance();
    xTaskCreate(usartDMATask, "usartDMATask", 256, NULL, 1, NULL);
    xTaskCreate(ledBlinkTask, "ledBlinkTask", 256, NULL, 2, NULL);
    xTaskCreate(logTask, "logTask", 1024, NULL, 3, NULL);

    vTaskStartScheduler();
    for (;;);
}

void usartDMATask(void *pvParameters) {
    Logger &log = Logger::getInstance();
    // 创建信号量
    dmaCompleteSemaphore = xSemaphoreCreateBinary();
    // Log.d("Usart DMA task start.");
    USART_DMA_Handler *uartDMA = static_cast<USART_DMA_Handler *>(pvParameters);
    for (;;) {
        // 等待 DMA 完成信号
        if (xSemaphoreTake(dmaCompleteSemaphore, portMAX_DELAY) == pdPASS) {
            log.d("Usart recv.");
        }
    }
}

void ledBlinkTask(void *pvParameters) {
    Logger &log = Logger::getInstance();
    for (;;) {
        log.d("ledBlinkTask!");
        led0.toggle();
        vTaskDelay(500);
    }
}

void logTask(void *pvParameters) {
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