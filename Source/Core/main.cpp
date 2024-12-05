#include <array>
#include <cstdint>
#include <string>

#include "FreeRTOS.h"
#include "bsp_uart.h"
#include "gd32f4xx.h"
#include "task.h"

void vTask1(void *pvParameters);
void vTask2(void *pvParameters);
extern SerialConfig usart1_config;
Serial usart1(usart1_config);
static StaticTask_t xTaskBuffer;
static StaticTask_t xTaskBuffer1;

int main(void) {

    xTaskCreate(vTask1, "Task 1", 128, NULL, 1, NULL);
    xTaskCreate(vTask2, "Task 2", 128, NULL, 2, NULL);
    vTaskStartScheduler();
    for (;;);
}

void vTask1(void *pvParameters) {
    for (;;) {
        std::array<uint8_t, 8> arr = {1, 2, 3, 4, 5, 6, 7, 8};
        usart1.dma_tx(arr.data(), 8);
        // usart1.data_send(arr.data(), 8);
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