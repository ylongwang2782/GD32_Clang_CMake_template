#include <stdint.h>

#include <array>
#include <cstdint>
#include <cstdio>
#include <string>

#include "FreeRTOS.h"
#include "bsp_led.h"
#include "bsp_log.h"
#include "bsp_uid.h"
#include "task.h"

#ifdef __cplusplus
extern "C" {
#endif

#include "gd32f4xx.h"
#ifdef __cplusplus
}
#endif

void vTask1(void *pvParameters);
void vTask2(void *pvParameters);
extern SerialConfig usart1_config;

LED led0(RCU_GPIOC, GPIOC, GPIO_PIN_6);

int main(void) {
    Serial usart1(usart1_config);
    UIDReader &uid = UIDReader::getInstance();
    printf("Hello World!\n");

    xTaskCreate(vTask1, "Task 1", 128, NULL, 1, NULL);
    xTaskCreate(vTask2, "Task 2", 128, NULL, 2, NULL);
    vTaskStartScheduler();
    for (;;);
}

void vTask1(void *pvParameters) {
    for (;;) {
        vTaskDelay(500);
    }
}

void vTask2(void *pvParameters) {
    for (;;) {
        led0.toggle();
        vTaskDelay(500);
    }
}