// #include "cstdio.h"
#ifndef BSP_UART_HPP
#define BSP_UART_HPP

#include <cstdint>
#include <cstring>
#include <string>

extern "C" {
#include "FreeRTOS.h"
#include "gd32f4xx.h"
#include "gd32f4xx_dma.h"
#include "queue.h"
#include "semphr.h"
#include "task.h"
}

#define ARRAYNUM(arr_name) (uint32_t)(sizeof(arr_name) / sizeof(*(arr_name)))
#define DMA_RX_BUFFER_SIZE 1024

typedef struct {
    uint32_t baudrate;                  // 波特率
    uint32_t gpio_port;                 // GPIO端口
    uint32_t tx_pin;                    // 发送引脚
    uint32_t rx_pin;                    // 接收引脚
    uint32_t usart_periph;              // USART外设
    rcu_periph_enum usart_clk;          // USART时钟
    rcu_periph_enum usart_port_clk;     // USART时钟
    uint8_t gpio_af;                    // GPIO复用功能
    rcu_periph_enum rcu_dma_periph;     // DMA发送通道
    uint32_t dma_periph;                // DMA发送通道
    dma_channel_enum dma_tx_channel;    // DMA发送通道
    dma_channel_enum dma_rx_channel;    // DMA接收通道
    uint8_t nvic_irq;                   // NVIC中断号
    uint8_t nvic_irq_pre_priority;      // NVIC中断优先级
    uint8_t nvic_irq_sub_priority;      // NVIC中断子优先级
    uint16_t rx_count;
    SemaphoreHandle_t dmaRxDoneSema;
} UasrtInfo;

class UartConfig {
   public:
    uint32_t baudrate;                  // 波特率
    uint32_t gpio_port;                 // GPIO端口
    uint32_t tx_pin;                    // 发送引脚
    uint32_t rx_pin;                    // 接收引脚
    uint32_t usart_periph;              // USART外设
    rcu_periph_enum usart_clk;          // USART时钟
    rcu_periph_enum usart_port_clk;     // USART端口时钟
    uint8_t gpio_af;                    // GPIO复用功能
    rcu_periph_enum rcu_dma_periph;     // DMA时钟
    uint32_t dma_periph;                // DMA外设
    dma_channel_enum dma_tx_channel;    // DMA发送通道
    dma_channel_enum dma_rx_channel;    // DMA接收通道
    uint8_t nvic_irq;                   // NVIC中断号
    uint8_t nvic_irq_pre_priority;      // NVIC中断优先级
    uint8_t nvic_irq_sub_priority;      // NVIC中断子优先级
    uint16_t *rx_count;                 // 接收计数

    UartConfig(UasrtInfo &info)
        : baudrate(info.baudrate),
          gpio_port(info.gpio_port),
          tx_pin(info.tx_pin),
          rx_pin(info.rx_pin),
          usart_periph(info.usart_periph),
          usart_clk(info.usart_clk),
          usart_port_clk(info.usart_port_clk),
          gpio_af(info.gpio_af),
          rcu_dma_periph(info.rcu_dma_periph),
          dma_periph(info.dma_periph),
          dma_tx_channel(info.dma_tx_channel),
          dma_rx_channel(info.dma_rx_channel),
          nvic_irq(info.nvic_irq),
          nvic_irq_pre_priority(info.nvic_irq_pre_priority),
          nvic_irq_sub_priority(info.nvic_irq_sub_priority),
          rx_count(&info.rx_count) {}    // 传递 rx_count 指针
};

extern UasrtInfo usart0_info;
extern UasrtInfo usart1_info;
extern UasrtInfo usart2_info;
extern UasrtInfo uart3_info;

class Uart {
   public:
    Uart(UartConfig &config) : config(config) {
        initGpio();
        initUsart();
        initDmaTx();
        initDmaRx();
    }

    void send(const uint8_t *data, uint16_t len) {
        dma_channel_disable(config.dma_periph, config.dma_tx_channel);
        dma_flag_clear(config.dma_periph, config.dma_tx_channel, DMA_FLAG_FTF);
        dma_memory_address_config(config.dma_periph, config.dma_tx_channel,
                                  DMA_MEMORY_0, (uintptr_t)data);
        dma_transfer_number_config(config.dma_periph, config.dma_tx_channel,
                                   len);
        dma_channel_enable(config.dma_periph, config.dma_tx_channel);
        while (RESET == usart_flag_get(config.usart_periph, USART_FLAG_TC));
    }

    uint16_t getReceivedData(uint8_t *buffer, uint16_t bufferSize) {
        if (bufferSize < *config.rx_count) {
            return 0;    // 缓冲区不足
        }
        memcpy(buffer, dmaRxBuffer, *config.rx_count);
        return *config.rx_count;
    }

   private:
    UartConfig &config;
    uint8_t dmaRxBuffer[DMA_RX_BUFFER_SIZE];

    void initGpio() {
        rcu_periph_clock_enable(config.usart_port_clk);
        gpio_af_set(config.gpio_port, config.gpio_af, config.tx_pin);
        gpio_af_set(config.gpio_port, config.gpio_af, config.rx_pin);
        gpio_mode_set(config.gpio_port, GPIO_MODE_AF, GPIO_PUPD_PULLUP,
                      config.tx_pin);
        gpio_output_options_set(config.gpio_port, GPIO_OTYPE_PP,
                                GPIO_OSPEED_50MHZ, config.tx_pin);
        gpio_mode_set(config.gpio_port, GPIO_MODE_AF, GPIO_PUPD_PULLUP,
                      config.rx_pin);
        gpio_output_options_set(config.gpio_port, GPIO_OTYPE_PP,
                                GPIO_OSPEED_50MHZ, config.rx_pin);
    }

    void initUsart() {
        rcu_periph_clock_enable(config.usart_clk);
        usart_deinit(config.usart_periph);
        usart_baudrate_set(config.usart_periph, config.baudrate);
        usart_receive_config(config.usart_periph, USART_RECEIVE_ENABLE);
        usart_transmit_config(config.usart_periph, USART_TRANSMIT_ENABLE);
        usart_dma_receive_config(config.usart_periph, USART_RECEIVE_DMA_ENABLE);
        usart_dma_transmit_config(config.usart_periph,
                                  USART_TRANSMIT_DMA_ENABLE);
        usart_enable(config.usart_periph);
        usart_interrupt_enable(config.usart_periph, USART_INT_IDLE);
    }

    void initDmaTx() {
        dma_single_data_parameter_struct dmaInitStruct;
        rcu_periph_clock_enable(config.rcu_dma_periph);
        dma_deinit(config.dma_periph, config.dma_tx_channel);
        dmaInitStruct.direction = DMA_MEMORY_TO_PERIPH;
        dmaInitStruct.memory0_addr = (uintptr_t) nullptr;
        dmaInitStruct.memory_inc = DMA_MEMORY_INCREASE_ENABLE;
        dmaInitStruct.periph_memory_width = DMA_PERIPH_WIDTH_8BIT;
        dmaInitStruct.number = 0;
        dmaInitStruct.periph_addr = (uintptr_t)&USART_DATA(config.usart_periph);
        dmaInitStruct.periph_inc = DMA_PERIPH_INCREASE_DISABLE;
        dmaInitStruct.priority = DMA_PRIORITY_ULTRA_HIGH;
        dma_single_data_mode_init(config.dma_periph, config.dma_tx_channel,
                                  &dmaInitStruct);
        dma_circulation_disable(config.dma_periph, config.dma_tx_channel);
        dma_channel_subperipheral_select(config.dma_periph,
                                         config.dma_tx_channel, DMA_SUBPERI4);
        dma_channel_disable(config.dma_periph, config.dma_tx_channel);
    }

    void initDmaRx() {
        dma_single_data_parameter_struct dmaInitStruct;
        nvic_irq_enable(config.nvic_irq, config.nvic_irq_pre_priority,
                        config.nvic_irq_sub_priority);
        rcu_periph_clock_enable(config.rcu_dma_periph);
        dma_deinit(config.dma_periph, config.dma_rx_channel);
        dmaInitStruct.direction = DMA_PERIPH_TO_MEMORY;
        dmaInitStruct.memory0_addr = (uintptr_t)dmaRxBuffer;
        dmaInitStruct.memory_inc = DMA_MEMORY_INCREASE_ENABLE;
        dmaInitStruct.number = DMA_RX_BUFFER_SIZE;
        dmaInitStruct.periph_addr = (uintptr_t)&USART_DATA(config.usart_periph);
        dmaInitStruct.periph_inc = DMA_PERIPH_INCREASE_DISABLE;
        dmaInitStruct.periph_memory_width = DMA_PERIPH_WIDTH_8BIT;
        dmaInitStruct.priority = DMA_PRIORITY_ULTRA_HIGH;
        dma_single_data_mode_init(config.dma_periph, config.dma_rx_channel,
                                  &dmaInitStruct);
        dma_circulation_disable(config.dma_periph, config.dma_rx_channel);
        dma_channel_subperipheral_select(config.dma_periph,
                                         config.dma_rx_channel, DMA_SUBPERI4);
        dma_channel_enable(config.dma_periph, config.dma_rx_channel);
    }
};
#endif
