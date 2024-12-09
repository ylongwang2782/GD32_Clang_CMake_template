#include <cstdio>

#include "FreeRTOS.h"
#include "bsp_led.h"
#include "bsp_log.h"
#include "bsp_uid.h"
#include "task.h"

#ifdef __cplusplus
extern "C" {
#endif

#ifdef __cplusplus
}
#endif

void vTask1(void *pvParameters);
void led_task(void *pvParameters);
extern SerialConfig usart1_config;

LED led0(RCU_GPIOC, GPIOC, GPIO_PIN_6);

int main(void) {
    Serial usart1(usart1_config);
    UIDReader &uid = UIDReader::getInstance();
    LOGF("Read uid success!\n");
    xTaskCreate(vTask1, "Task 1", 128, NULL, 1, NULL);
    xTaskCreate(led_task, "Task 2", 128, NULL, 2, NULL);
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
        led0.toggle();
        vTaskDelay(500);
    }
}