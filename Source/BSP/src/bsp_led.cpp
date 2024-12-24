/**
 * @file led.cpp
 * @author wang.yunlong (wang.yunlong9@byd.com)
 * @brief
 * @version 0.1
 * @date 2024-10-22
 *
 * @copyright Copyright (c) 2024
 *
 */
#include "bsp_led.h"

// 根据端口自动返回对应的 RCC 外设时钟
rcu_periph_enum get_rcc_from_port(uint32_t port) {
    switch (port) {
        case GPIOA:
            return RCU_GPIOA;
        case GPIOB:
            return RCU_GPIOB;
        case GPIOC:
            return RCU_GPIOC;
        case GPIOD:
            return RCU_GPIOD;
        case GPIOE:
            return RCU_GPIOE;
        case GPIOF:
            return RCU_GPIOF;
        case GPIOG:
            return RCU_GPIOG;
        default:
            // 错误处理: 无效端口
            while (1) {
                // 可添加日志或错误指示
            }
    }
}

LED::LED(uint32_t port, uint32_t pin) {
    this->port = port;
    this->pin = pin;

    // 自动根据 port 判断 RCC 外设时钟
    rcu_periph_clock_enable(get_rcc_from_port(this->port));

    gpio_mode_set(this->port, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, this->pin);
    gpio_output_options_set(this->port, GPIO_OTYPE_PP, GPIO_OSPEED_50MHZ,
                            this->pin);
    gpio_bit_reset(this->port, this->pin);
}

LED::~LED() {}

void LED::on(void) { gpio_bit_set(this->port, this->pin); }

void LED::off(void) { gpio_bit_reset(this->port, this->pin); }

void LED::toggle(void) { gpio_bit_toggle(this->port, this->pin); }