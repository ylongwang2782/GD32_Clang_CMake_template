// #include "cstdio.h"
#ifndef BSP_UART_HPP
#define BSP_UART_HPP

#include <cstdint>
#include <cstring>
#include <string>
#include <vector>

#include "QueueCPP.h"
#include "bsp_gpio.hpp"

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
    uint32_t baudrate;                     // 波特率
    uint32_t gpio_port;                    // GPIO端口
    uint32_t tx_pin;                       // 发送引脚
    uint32_t rx_pin;                       // 接收引脚
    uint32_t usart_periph;                 // USART外设
    rcu_periph_enum usart_clk;             // USART时钟
    rcu_periph_enum usart_port_clk;        // USART时钟
    uint8_t gpio_af;                       // GPIO复用功能
    rcu_periph_enum rcu_dma_periph;        // DMA发送通道
    uint32_t dma_periph;                   // DMA发送通道
    dma_subperipheral_enum dma_sub_per;    // DMA子通道
    dma_channel_enum dma_tx_channel;       // DMA发送通道
    dma_channel_enum dma_rx_channel;       // DMA接收通道
    uint8_t nvic_irq;                      // NVIC中断号
    uint8_t nvic_irq_pre_priority;         // NVIC中断优先级
    uint8_t nvic_irq_sub_priority;         // NVIC中断子优先级
    uint16_t rx_count;
    SemaphoreHandle_t dmaRxDoneSema;
    bool use_dma;
} UasrtInfo;

class UartConfig {
   public:
    uint32_t baudrate;                     // 波特率
    uint32_t gpio_port;                    // GPIO端口
    uint32_t tx_pin;                       // 发送引脚
    uint32_t rx_pin;                       // 接收引脚
    uint32_t usart_periph;                 // USART外设
    rcu_periph_enum usart_clk;             // USART时钟
    rcu_periph_enum usart_port_clk;        // USART端口时钟
    uint8_t gpio_af;                       // GPIO复用功能
    rcu_periph_enum rcu_dma_periph;        // DMA时钟
    uint32_t dma_periph;                   // DMA外设
    dma_subperipheral_enum dma_sub_per;    // DMA子通道
    dma_channel_enum dma_tx_channel;       // DMA发送通道
    dma_channel_enum dma_rx_channel;       // DMA接收通道
    uint8_t nvic_irq;                      // NVIC中断号
    uint8_t nvic_irq_pre_priority;         // NVIC中断优先级
    uint8_t nvic_irq_sub_priority;         // NVIC中断子优先级
    uint16_t *rx_count;                    // 接收计数
    bool use_dma;                          // 是否使用DMA
    uint16_t rxQueueSize;                  // 接收队列大小

    UartConfig(UasrtInfo &info, bool enable_dma = true,
               uint16_t _rxQueueSize = 1)
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
          dma_sub_per(info.dma_sub_per),
          dma_tx_channel(info.dma_tx_channel),
          dma_rx_channel(info.dma_rx_channel),
          nvic_irq(info.nvic_irq),
          nvic_irq_pre_priority(info.nvic_irq_pre_priority),
          nvic_irq_sub_priority(info.nvic_irq_sub_priority),
          rx_count(&info.rx_count),    // 传递 rx_count 指针
          use_dma(enable_dma),
          rxQueueSize(_rxQueueSize) {
        info.use_dma = enable_dma;
    }
};

extern UasrtInfo usart0_info;
extern UasrtInfo usart0_info_PA9PA10;
extern UasrtInfo usart1_info;
extern UasrtInfo usart2_info;
extern UasrtInfo uart3_info;
extern UasrtInfo usart5_info;
extern UasrtInfo uart6_info;
extern UasrtInfo uart7_info;

extern "C" {
void USART0_IRQHandler(void);
void USART1_IRQHandler(void);
void USART2_IRQHandler(void);
void UART3_IRQHandler(void);
void USART5_IRQHandler(void);
void UART6_IRQHandler(void);
void UART7_IRQHandler(void);
}

class Uart {
   public:
    friend void USART0_IRQHandler(void);
    friend void USART1_IRQHandler(void);
    friend void USART2_IRQHandler(void);
    friend void UART3_IRQHandler(void);
    friend void USART5_IRQHandler(void);
    friend void UART6_IRQHandler(void);
    friend void UART7_IRQHandler(void);

    Uart(UartConfig &config) : config(config), rxQueue(config.rxQueueSize) {
        initBSP();
        initGpio();
        initUsart();
        deviceRegister();
        if (config.use_dma) {
            initDmaTx();
            initDmaRx();
        }
    }

    void data_send(const uint8_t *data, uint16_t len) {
        for (uint16_t i = 0; i < len; i++) {
            usart_data_transmit(config.usart_periph, data[i]);
            while (RESET == usart_flag_get(config.usart_periph, USART_FLAG_TC));
        }
    }

    void data_send(const std::string &str) {
        data_send((const uint8_t *)str.c_str(), str.size());
    }

    void data_send(std::vector<uint8_t> &data) {
        data_send(data.data(), data.size());
    }

    void send(const uint8_t *data, uint16_t len) {
        if (config.use_dma) {
            dma_channel_disable(config.dma_periph, config.dma_tx_channel);
            dma_flag_clear(config.dma_periph, config.dma_tx_channel,
                           DMA_FLAG_FTF);
            dma_memory_address_config(config.dma_periph, config.dma_tx_channel,
                                      DMA_MEMORY_0, (uintptr_t)data);
            dma_transfer_number_config(config.dma_periph, config.dma_tx_channel,
                                       len);
            dma_channel_enable(config.dma_periph, config.dma_tx_channel);
            while (RESET == usart_flag_get(config.usart_periph, USART_FLAG_TC));
        } else {
            for (uint16_t i = 0; i < len; i++) {
                usart_data_transmit(config.usart_periph, *(data + i));
                while (RESET ==
                       usart_flag_get(config.usart_periph, USART_FLAG_TC));
            }
        }
    }

    bool recv_1byte(uint8_t &data, TickType_t time = portMAX_DELAY) {
        if (!config.use_dma) {
            return rxQueue.pop(data, time);
        }
        return false;
    }

    std::vector<uint8_t> getReceivedData() {
        std::vector<uint8_t> buffer;
        // Resize the buffer to ensure it has enough space
        if (config.use_dma) {
            buffer.resize(*config.rx_count);
            memcpy(buffer.data(), dmaRxBuffer, *config.rx_count);
            // clear
            *config.rx_count = 0;
        } else {
            uint8_t data;
            buffer.reserve(rxQueue.waiting());
            while (rxQueue.pop(data, 0)) {
                buffer.push_back(data);
            }
        }

        return buffer;
    }

   private:
    UartConfig &config;
    uint8_t dmaRxBuffer[DMA_RX_BUFFER_SIZE];

    enum UART_ID : uint8_t {
        _UART0 = 0,
        _UART1,
        _UART2,
        _UART3,
        _UART4,
        _UART5,
        _UART6,
        _UART7,

        _UART_NUM,
    };
    static Uart *dev[_UART_NUM];
    static bool is_bsp_init;
    Queue<uint8_t> rxQueue;
    long waswoken = pdFALSE;

    void irq_handler() {
        if (!config.use_dma) {
            if (RESET != usart_interrupt_flag_get(config.usart_periph,
                                                  USART_INT_FLAG_RBNE)) {
            }
            rxQueue.add_ISR(usart_data_receive(config.usart_periph), waswoken);
            portYIELD_FROM_ISR(waswoken);
        }
    }

    void initBSP() {
        if (!is_bsp_init) {
            for (uint8_t i = 0; i < _UART_NUM; i++) {
                dev[i] = nullptr;
            }
            is_bsp_init = true;
        }
    }

    void deviceRegister() {
        switch (config.usart_periph) {
            case USART0: {
                dev[_UART0] = this;
                break;
            }
            case USART1: {
                dev[_UART1] = this;
                break;
            }
            case USART2: {
                dev[_UART2] = this;
                break;
            }
            case UART3: {
                dev[_UART3] = this;
                break;
            }
            case UART4: {
                dev[_UART4] = this;
                break;
            }
            case USART5: {
                dev[_UART5] = this;
                break;
            }
            case UART6: {
                dev[_UART6] = this;
                break;
            }
            case UART7: {
                dev[_UART7] = this;
                break;
            }
        }
    }

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

        if (config.use_dma) {
            usart_interrupt_enable(config.usart_periph, USART_INT_IDLE);
        } else {
            // usart_interrupt_enable(config.usart_periph, USART_INT_IDLE);
            nvic_irq_enable(config.nvic_irq, config.nvic_irq_pre_priority,
                            config.nvic_irq_sub_priority);
            usart_interrupt_enable(config.usart_periph, USART_INT_RBNE);
        }
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
        dma_channel_subperipheral_select(
            config.dma_periph, config.dma_tx_channel, config.dma_sub_per);
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
        dma_channel_subperipheral_select(
            config.dma_periph, config.dma_rx_channel, config.dma_sub_per);
        dma_channel_enable(config.dma_periph, config.dma_rx_channel);
    }
};

class Rs485 {
   public:
    Rs485(Uart &uart, GPIO::Port port, GPIO::Pin pin)
        : uart(uart), ctrl(port, pin, GPIO::Mode::OUTPUT) {}
    Uart &uart;

    std::vector<uint8_t> getReceivedData() {
        ctrl.bit_reset();
        return uart.getReceivedData();
    }

    void data_send(std::vector<uint8_t> &data) {
        ctrl.bit_set();
        uart.data_send(data);
    }

   private:
    GPIO ctrl;
};
#endif
