// #include "cstdio.h"
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
#define com_idle_rx_size   256
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
} UasrtConfig;

#define DMA_BUFFER_SIZE 1024

class USART_DMA_Handler {
   public:
    USART_DMA_Handler(UasrtConfig &config) : config(config) { setup(); }
    uint8_t rxbuffer[DMA_RX_BUFFER_SIZE];
    uint8_t DMA_RX_Buffer[DMA_RX_BUFFER_SIZE];
    void dma_tx(uint8_t *data, uint16_t len) {
        dma_channel_disable(config.dma_periph, config.dma_tx_channel);
        dma_flag_clear(config.dma_periph, config.dma_tx_channel, DMA_FLAG_FTF);
        dma_memory_address_config(config.dma_periph, config.dma_tx_channel,
                                  DMA_MEMORY_0, (uintptr_t)data);
        dma_transfer_number_config(config.dma_periph, config.dma_tx_channel,
                                   len);
        dma_channel_enable(config.dma_periph, config.dma_tx_channel);
        while (RESET == usart_flag_get(config.usart_periph, USART_FLAG_TC));
    }

   private:
    uint8_t txbuffer[1];
    const UasrtConfig &config;

    // 初始化 DMA
    void setup() {
        init();
        dma_tx_config();
        idle_dma_rx_config();
    }
    void init() {
        rcu_periph_clock_enable(config.usart_port_clk);
        rcu_periph_clock_enable(config.usart_clk);
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

        usart_deinit(config.usart_periph);
        usart_baudrate_set(config.usart_periph, config.baudrate);
        usart_receive_config(config.usart_periph, USART_RECEIVE_ENABLE);
        usart_transmit_config(config.usart_periph, USART_TRANSMIT_ENABLE);
        usart_dma_receive_config(config.usart_periph, USART_RECEIVE_DMA_ENABLE);
        usart_dma_transmit_config(config.usart_periph,
                                  USART_RECEIVE_DMA_ENABLE);
        usart_enable(config.usart_periph);

        usart_interrupt_enable(config.usart_periph, USART_INT_IDLE);
    }
    void dma_tx_config() {
        dma_single_data_parameter_struct dma_init_struct;
        rcu_periph_clock_enable(config.rcu_dma_periph);
        dma_deinit(config.dma_periph, config.dma_tx_channel);
        dma_init_struct.direction = DMA_MEMORY_TO_PERIPH;
        dma_init_struct.memory0_addr = (uintptr_t)txbuffer;
        dma_init_struct.memory_inc = DMA_MEMORY_INCREASE_ENABLE;
        dma_init_struct.periph_memory_width = DMA_PERIPH_WIDTH_8BIT;
        dma_init_struct.number = ARRAYNUM(txbuffer);
        dma_init_struct.periph_addr =
            (uintptr_t)&USART_DATA(config.usart_periph);
        dma_init_struct.periph_inc = DMA_PERIPH_INCREASE_DISABLE;
        dma_init_struct.priority = DMA_PRIORITY_ULTRA_HIGH;
        dma_single_data_mode_init(config.dma_periph, config.dma_tx_channel,
                                  &dma_init_struct);

        dma_circulation_disable(config.dma_periph, config.dma_tx_channel);
        dma_channel_subperipheral_select(config.dma_periph,
                                         config.dma_tx_channel, DMA_SUBPERI4);
        dma_channel_disable(config.dma_periph, config.dma_tx_channel);
        usart_dma_transmit_config(config.usart_periph,
                                  USART_TRANSMIT_DMA_ENABLE);
    }

    void idle_dma_rx_config() {
        dma_single_data_parameter_struct dma_init_struct;
        nvic_irq_enable(config.nvic_irq, config.nvic_irq_pre_priority,
                        config.nvic_irq_sub_priority);
        rcu_periph_clock_enable(config.rcu_dma_periph);

        dma_deinit(config.dma_periph, config.dma_rx_channel);
        dma_init_struct.direction = DMA_PERIPH_TO_MEMORY;
        dma_init_struct.memory0_addr = (uintptr_t)DMA_RX_Buffer;
        dma_init_struct.memory_inc = DMA_MEMORY_INCREASE_ENABLE;
        dma_init_struct.number = DMA_RX_BUFFER_SIZE;
        dma_init_struct.periph_addr =
            (uintptr_t)&USART_DATA(config.usart_periph);
        dma_init_struct.periph_inc = DMA_PERIPH_INCREASE_DISABLE;
        dma_init_struct.periph_memory_width = DMA_PERIPH_WIDTH_8BIT;
        dma_init_struct.priority = DMA_PRIORITY_ULTRA_HIGH;
        dma_single_data_mode_init(config.dma_periph, config.dma_rx_channel,
                                  &dma_init_struct);

        dma_circulation_disable(config.dma_periph, config.dma_rx_channel);
        dma_channel_subperipheral_select(config.dma_periph,
                                         config.dma_rx_channel, DMA_SUBPERI4);
        dma_channel_enable(config.dma_periph, config.dma_rx_channel);

        usart_interrupt_enable(config.usart_periph, USART_INT_IDLE);
    }
};