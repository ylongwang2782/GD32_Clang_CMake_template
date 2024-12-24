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

   private:
    // 初始化 DMA
    void setup();
    void init();
    void dma_tx_config();
    void idle_dma_rx_config();
    uint8_t txbuffer[1];
    const UasrtConfig &config;
};