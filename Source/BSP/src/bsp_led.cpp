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

LED::LED(rcu_periph_enum rcc, uint32_t port, uint32_t pin) {
    this->port = port;
    this->pin = pin;

    rcu_periph_clock_enable(rcc);
    gpio_mode_set(this->port, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, this->pin);
    gpio_output_options_set(this->port, GPIO_OTYPE_PP, GPIO_OSPEED_50MHZ,
                            this->pin);
    gpio_bit_reset(this->port, this->pin);
}

LED::~LED() {}

void LED::on(void) { gpio_bit_set(this->port, this->pin); }

void LED::off(void) { gpio_bit_reset(this->port, this->pin); }

void LED::toggle(void) { gpio_bit_toggle(this->port, this->pin); }