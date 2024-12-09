#include "bsp_uart.h"

SerialConfig usart1_config = {.baudrate = 115200,
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
                              .nvic_irq_pre_priority = 1,
                              .nvic_irq_sub_priority = 1,
                              .rx_count = 0};

SerialConfig usart2_config = {.baudrate = 115200,
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

void Serial::setup() {
    Serial::init();
    Serial::dma_tx_config();
    Serial::idle_dma_rx_config();
}

void Serial::init() {
    rcu_periph_clock_enable(config.usart_port_clk);
    rcu_periph_clock_enable(config.usart_clk);
    gpio_af_set(config.gpio_port, config.gpio_af, config.tx_pin);
    gpio_af_set(config.gpio_port, config.gpio_af, config.rx_pin);
    gpio_mode_set(config.gpio_port, GPIO_MODE_AF, GPIO_PUPD_PULLUP,
                  config.tx_pin);
    gpio_output_options_set(config.gpio_port, GPIO_OTYPE_PP, GPIO_OSPEED_50MHZ,
                            config.tx_pin);
    gpio_mode_set(config.gpio_port, GPIO_MODE_AF, GPIO_PUPD_PULLUP,
                  config.rx_pin);
    gpio_output_options_set(config.gpio_port, GPIO_OTYPE_PP, GPIO_OSPEED_50MHZ,
                            config.rx_pin);

    usart_deinit(config.usart_periph);
    usart_baudrate_set(config.usart_periph, config.baudrate);
    usart_receive_config(config.usart_periph, USART_RECEIVE_ENABLE);
    usart_transmit_config(config.usart_periph, USART_TRANSMIT_ENABLE);
    usart_dma_receive_config(config.usart_periph, USART_RECEIVE_DMA_ENABLE);
    usart_dma_transmit_config(config.usart_periph, USART_RECEIVE_DMA_ENABLE);
    usart_enable(config.usart_periph);

    usart_interrupt_enable(config.usart_periph, USART_INT_IDLE);
}

void Serial::dma_tx_config() {
    dma_single_data_parameter_struct dma_init_struct;
    rcu_periph_clock_enable(config.rcu_dma_periph);
    dma_deinit(config.dma_periph, config.dma_tx_channel);
    dma_init_struct.direction = DMA_MEMORY_TO_PERIPH;
    dma_init_struct.memory0_addr = (uintptr_t)txbuffer;
    dma_init_struct.memory_inc = DMA_MEMORY_INCREASE_ENABLE;
    dma_init_struct.periph_memory_width = DMA_PERIPH_WIDTH_8BIT;
    dma_init_struct.number = ARRAYNUM(txbuffer);
    dma_init_struct.periph_addr = (uintptr_t)&USART_DATA(config.usart_periph);
    dma_init_struct.periph_inc = DMA_PERIPH_INCREASE_DISABLE;
    dma_init_struct.priority = DMA_PRIORITY_ULTRA_HIGH;
    dma_single_data_mode_init(config.dma_periph, config.dma_tx_channel,
                              &dma_init_struct);

    dma_circulation_disable(config.dma_periph, config.dma_tx_channel);
    dma_channel_subperipheral_select(config.dma_periph, config.dma_tx_channel,
                                     DMA_SUBPERI4);
    dma_channel_disable(config.dma_periph, config.dma_tx_channel);
    usart_dma_transmit_config(config.usart_periph, USART_TRANSMIT_DMA_ENABLE);
}

void Serial::dma_tx(uint8_t *data, uint16_t len) {
    dma_channel_disable(config.dma_periph, config.dma_tx_channel);
    dma_flag_clear(config.dma_periph, config.dma_tx_channel, DMA_FLAG_FTF);
    dma_memory_address_config(config.dma_periph, config.dma_tx_channel,
                              DMA_MEMORY_0, (uintptr_t)data);
    dma_transfer_number_config(config.dma_periph, config.dma_tx_channel, len);
    dma_channel_enable(config.dma_periph, config.dma_tx_channel);
    while (RESET == usart_flag_get(config.usart_periph, USART_FLAG_TC));
}

void Serial::data_send(uint8_t *data, uint16_t len) {
    for (size_t i = 0; i < len; i++) {
        usart_data_transmit(config.usart_periph, data[i]);
        while (RESET == usart_flag_get(config.usart_periph, USART_FLAG_TBE)) {
        }
    }
}

void Serial::idle_dma_rx_config() {
    dma_single_data_parameter_struct dma_init_struct;
    nvic_irq_enable(config.nvic_irq, config.nvic_irq_pre_priority,
                    config.nvic_irq_sub_priority);
    rcu_periph_clock_enable(config.rcu_dma_periph);

    dma_deinit(config.dma_periph, config.dma_rx_channel);
    dma_init_struct.direction = DMA_PERIPH_TO_MEMORY;
    dma_init_struct.memory0_addr = (uintptr_t)rxbuffer;
    dma_init_struct.memory_inc = DMA_MEMORY_INCREASE_ENABLE;
    dma_init_struct.number = DMA_RX_BUFFER_SIZE;
    dma_init_struct.periph_addr = (uintptr_t)&USART_DATA(config.usart_periph);
    dma_init_struct.periph_inc = DMA_PERIPH_INCREASE_DISABLE;
    dma_init_struct.periph_memory_width = DMA_PERIPH_WIDTH_8BIT;
    dma_init_struct.priority = DMA_PRIORITY_ULTRA_HIGH;
    dma_single_data_mode_init(config.dma_periph, config.dma_rx_channel,
                              &dma_init_struct);

    dma_circulation_disable(config.dma_periph, config.dma_rx_channel);
    dma_channel_subperipheral_select(config.dma_periph, config.dma_rx_channel,
                                     DMA_SUBPERI4);
    dma_channel_enable(config.dma_periph, config.dma_rx_channel);

    usart_interrupt_enable(config.usart_periph, USART_INT_IDLE);
}

void Serial::sendString(const std::string &str) {
    // 获取 std::string 的数据指针和长度
    const uint8_t *data = reinterpret_cast<const uint8_t *>(str.data());
    uint16_t len = static_cast<uint16_t>(str.size());

    // 调用 dma_tx 发送数据
    dma_tx(const_cast<uint8_t *>(data), len);    // const_cast 去除 const 性
}

void handle_usart_interrupt(SerialConfig *config) {
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
        dma_transfer_number_config(config->dma_periph, config->dma_rx_channel,
                                   DMA_RX_BUFFER_SIZE);
        dma_channel_enable(config->dma_periph, config->dma_rx_channel);
    }
}
extern "C" {
void USART1_IRQHandler(void) { handle_usart_interrupt(&usart1_config); }
void USART2_IRQHandler(void) { handle_usart_interrupt(&usart2_config); }
}