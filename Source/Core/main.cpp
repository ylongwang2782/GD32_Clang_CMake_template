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

void vTask1(void *pvParameters);
void led_task(void *pvParameters);
extern SerialConfig usart1_config;

Logger log;

LED led0(RCU_GPIOC, GPIOC, GPIO_PIN_6);
void LogTask(void *pvParameters);

int main(void) {
    Serial usart1(usart1_config);
    UIDReader &uid = UIDReader::getInstance();
    xTaskCreate(vTask1, "Task 1", 128, NULL, 1, NULL);
    xTaskCreate(led_task, "Task 2", 128, NULL, 2, NULL);

    // 最大存储10条日志，每条最多300字节
    size_t freeHeap = xPortGetFreeHeapSize();
    printf("Free heap size: %u bytes\n", freeHeap);
    // FIXME 修复队列大小不能扩大的问题
    log.logQueue = xQueueCreate(10, 64);
    xTaskCreate(LogTask, "LogTask", 1024, nullptr, 3, nullptr);

    vTaskStartScheduler();
    for (;;);
}

void vTask1(void *pvParameters) {
    for (;;) {
        vTaskDelay(500);
    }
}

void led_task(void *pvParameters) {
    for (;;) {
        // log.d( "led_task!");
        log.log(LogLevel::DEBUGL, "led_task!");
        log.i( "led_task!");
        log.w( "led_task!");
        log.e( "led_task!");
        led0.toggle();
        vTaskDelay(500);
    }
}

void LogTask(void *pvParameters) {
    char buffer[30];
    for (;;) {
        if (xQueueReceive(log.logQueue, buffer, portMAX_DELAY)) {
            // TODO 更换为dma发送
            for (const char *p = buffer; *p; ++p) {
                while (RESET == usart_flag_get(USART1, USART_FLAG_TBE));
                usart_data_transmit(USART1, (uint8_t)*p);
            }
        }
    }
}