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

void uartDMATask(void *pvParameters);
void led_task(void *pvParameters);
void LogTask(void *pvParameters);

extern SerialConfig usart1_config;

Logger Log;

LED led0(RCU_GPIOC, GPIOC, GPIO_PIN_6);

int main(void) {
    nvic_priority_group_set(NVIC_PRIGROUP_PRE4_SUB0);

    Serial usart1(usart1_config);

    UIDReader &uid = UIDReader::getInstance();

    // FIXME 修复队列大小不能扩大的问题
    Log.logQueue = xQueueCreate(10, LOG_QUEUE_SIZE);

    xTaskCreate(uartDMATask, "Task 1", 1024, NULL, 1, NULL);
    xTaskCreate(led_task, "led task", 256, NULL, 2, NULL);
    xTaskCreate(LogTask, "LogTask", 1024, NULL, 3, NULL);

    vTaskStartScheduler();
    for (;;);
}

void uartDMATask(void *pvParameters) {
    // Log.v("task1!");
    for (;;) {
        vTaskDelay(500);
    }
}

void led_task(void *pvParameters) {
    for (;;) {
        Log.d("led_task!");
        led0.toggle();
        vTaskDelay(500);
    }
}

void LogTask(void *pvParameters) {
    char buffer[LOG_QUEUE_SIZE + 8];
    for (;;) {
        if (xQueueReceive(Log.logQueue, buffer, portMAX_DELAY)) {
            // TODO 更换为dma发送
            for (const char *p = buffer; *p; ++p) {
                while (RESET == usart_flag_get(USART1, USART_FLAG_TBE));
                usart_data_transmit(USART1, (uint8_t)*p);
            }
        }
    }
}