#include "bsp_uart.h"

#include <cstdio>

UasrtConfig usart1_info = {.baudrate = 115200,
                              .gpio_port = GPIOD,
                              .tx_pin = GPIO_PIN_5,
                              .rx_pin = GPIO_PIN_6,
                              .usart_periph = USART1,
                              .usart_clk = RCU_USART1,
                              .usart_port_clk = RCU_GPIOD,
                              .gpio_af = GPIO_AF_7,
                              .rcu_dma_periph = RCU_DMA0,
                              .dma_periph = DMA0,
                              .dma_tx_channel = DMA_CH6,
                              .dma_rx_channel = DMA_CH5,
                              .nvic_irq = USART1_IRQn,
                              .nvic_irq_pre_priority = 6,
                              .nvic_irq_sub_priority = 0,
                              .rx_count = 0};

UasrtConfig usart2_config = {.baudrate = 115200,
                              .gpio_port = GPIOB,
                              .tx_pin = GPIO_PIN_10,
                              .rx_pin = GPIO_PIN_11,
                              .usart_periph = USART2,
                              .usart_clk = RCU_USART2,
                              .usart_port_clk = RCU_GPIOB,
                              .gpio_af = GPIO_AF_7,
                              .rcu_dma_periph = RCU_DMA0,
                              .dma_periph = DMA0,
                              .dma_tx_channel = DMA_CH3,
                              .dma_rx_channel = DMA_CH1,
                              .nvic_irq = USART2_IRQn,
                              .nvic_irq_pre_priority = 1,
                              .nvic_irq_sub_priority = 2,
                              .rx_count = 0};

// 全局信号量
SemaphoreHandle_t dmaCompleteSemaphore;
void handle_usart_interrupt(UasrtConfig *config) {
    if (RESET !=
        usart_interrupt_flag_get(config->usart_periph, USART_INT_FLAG_IDLE)) {
        /* clear IDLE flag */
        usart_data_receive(config->usart_periph);
        /* number of data received */
        config->rx_count = DMA_RX_BUFFER_SIZE -
                           (dma_transfer_number_get(config->dma_periph,
                                                    config->dma_rx_channel));
        dma_channel_disable(config->dma_periph, config->dma_rx_channel);
        dma_flag_clear(config->dma_periph, config->dma_rx_channel,
                       DMA_FLAG_FTF);
        // 通知任务 DMA 接收完成
        BaseType_t xHigherPriorityTaskWoken = pdFALSE;
        xSemaphoreGiveFromISR(dmaCompleteSemaphore, &xHigherPriorityTaskWoken);
        portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
        dma_transfer_number_config(config->dma_periph, config->dma_rx_channel,
                                   DMA_RX_BUFFER_SIZE);
        dma_channel_enable(config->dma_periph, config->dma_rx_channel);
    }
}

extern "C" {
void USART1_IRQHandler(void) { handle_usart_interrupt(&usart1_info); }
// void USART2_IRQHandler(void) { handle_usart_interrupt(&usart2_config); }
}