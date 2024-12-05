#include <string>
#include "FreeRTOS.h"
#include "gd32f4xx.h"
#include "task.h"
#include "bsp_uart.h"

void vTask1(void *pvParameters);
void vTask2(void *pvParameters);
extern SerialConfig usart1_config;
Serial usart1(usart1_config);

int main(void) {
    xTaskCreate(vTask1, "Task 1", 128, NULL, 1, NULL);
    xTaskCreate(vTask2, "Task 2", 128, NULL, 2, NULL);

    vTaskStartScheduler();

    for (;;);
}

void vTask1(void *pvParameters) {
    for (;;) {
        printf("Task 1 running\r\n");
        std::string str = "hello world";
        usart1.dma_tx((uint8_t*)str.c_str(), str.length() );
        vTaskDelay(500);
    }
}

void vTask2(void *pvParameters) {
    rcu_periph_clock_enable(RCU_SYSCFG);
    rcu_periph_clock_enable(RCU_GPIOC);
    gpio_mode_set(GPIOC, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, GPIO_PIN_6);
    gpio_output_options_set(GPIOC, GPIO_OTYPE_PP, GPIO_OSPEED_50MHZ,
                            GPIO_PIN_6);
    gpio_bit_set(GPIOC, GPIO_PIN_6);

    for (;;) {
        // vTaskDelayUntil(&xLastWakeTime, 1000);
        vTaskDelay(500);
        gpio_bit_toggle(GPIOC, GPIO_PIN_6);
    }
}