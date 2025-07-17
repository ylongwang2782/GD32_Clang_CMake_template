#pragma once
#include <cstdint>
extern "C" {
#include "gd32f4xx.h"
}

class GPIO {
   public:
    enum class Mode {
        INPUT = GPIO_MODE_INPUT,
        OUTPUT = GPIO_MODE_OUTPUT,
        AF = GPIO_MODE_AF,
        ANALOG = GPIO_MODE_ANALOG
    };

    enum class PullUpDown {
        NONE = GPIO_PUPD_NONE,
        PULLUP = GPIO_PUPD_PULLUP,
        PULLDOWN = GPIO_PUPD_PULLDOWN
    };

    enum class OType {
        PP = GPIO_OTYPE_PP,    // Push-pull
        OD = GPIO_OTYPE_OD     // Open drain
    };

    enum class Speed {
        SPEED_2MHZ = GPIO_OSPEED_2MHZ,
        SPEED_25MHZ = GPIO_OSPEED_25MHZ,
        SPEED_50MHZ = GPIO_OSPEED_50MHZ,
        SPEED_MAX = GPIO_OSPEED_MAX
    };

    enum class BitStatus { RESET = 0, SET = 1 };

    enum class Port {
        A = GPIOA,
        B = GPIOB,
        C = GPIOC,
        D = GPIOD,
        E = GPIOE,
        F = GPIOF,
        G = GPIOG,
    };

    enum class Pin {
        PIN_0 = GPIO_PIN_0,
        PIN_1 = GPIO_PIN_1,
        PIN_2 = GPIO_PIN_2,
        PIN_3 = GPIO_PIN_3,
        PIN_4 = GPIO_PIN_4,
        PIN_5 = GPIO_PIN_5,
        PIN_6 = GPIO_PIN_6,
        PIN_7 = GPIO_PIN_7,
        PIN_8 = GPIO_PIN_8,
        PIN_9 = GPIO_PIN_9,
        PIN_10 = GPIO_PIN_10,
        PIN_11 = GPIO_PIN_11,
        PIN_12 = GPIO_PIN_12,
        PIN_13 = GPIO_PIN_13,
        PIN_14 = GPIO_PIN_14,
        PIN_15 = GPIO_PIN_15,
        ALL = GPIO_PIN_ALL
    };

    static constexpr rcu_periph_enum get_rcu_port(Port port) {
        switch (port) {
            case Port::A:
                return RCU_GPIOA;
            case Port::B:
                return RCU_GPIOB;
            case Port::C:
                return RCU_GPIOC;
            case Port::D:
                return RCU_GPIOD;
            case Port::E:
                return RCU_GPIOE;
            case Port::F:
                return RCU_GPIOF;
            case Port::G:
                return RCU_GPIOG;
            default:
                return RCU_GPIOA;    // 默认返回 A 避免编译错误
        }
    }

    GPIO(Port port, Pin pin, Mode mode, PullUpDown pull = PullUpDown::PULLDOWN,
         OType otype = OType::PP, Speed speed = Speed::SPEED_50MHZ)
        : port(port), pin(pin) {
        rcu_periph_clock_enable(get_rcu_port(port));
        mode_set(mode, pull);
        output_options_set(otype, speed);
        if (mode == Mode::OUTPUT) {
            bit_reset();
        }
    }
    void mode_set(Mode mode, PullUpDown pull = PullUpDown::PULLDOWN) {
        gpio_mode_set(port_base(), static_cast<uint32_t>(mode),
                      static_cast<uint32_t>(pull), pin_mask());
    }

    void switch_to_input() {
        gpio_mode_set(port_base(), static_cast<uint32_t>(Mode::INPUT),
                      static_cast<uint32_t>(PullUpDown::PULLDOWN), pin_mask());
    }

    void switch_to_output() {
        gpio_mode_set(port_base(), static_cast<uint32_t>(Mode::OUTPUT),
                      static_cast<uint32_t>(PullUpDown::PULLDOWN), pin_mask());
    }

    void output_options_set(OType otype, Speed speed) {
        gpio_output_options_set(port_base(), static_cast<uint8_t>(otype),
                                static_cast<uint32_t>(speed), pin_mask());
    }

    void bit_set() { gpio_bit_set(port_base(), pin_mask()); }

    void bit_reset() { gpio_bit_reset(port_base(), pin_mask()); }

    void bit_write(bool bit_value) {
        gpio_bit_write(port_base(), pin_mask(),
                       static_cast<bit_status>(bit_value));
    }

    uint16_t input_port_get() { return gpio_input_port_get(port_base()); }

    FlagStatus input_bit_get() const {
        return gpio_input_bit_get(port_base(), pin_mask());
    }

    void af_set(uint32_t alt_func_num) {
        gpio_af_set(port_base(), alt_func_num, pin_mask());
    }

    void toggle() { gpio_bit_toggle(port_base(), pin_mask()); }

   private:
    Port port;
    Pin pin;

    constexpr uint32_t port_base() const { return static_cast<uint32_t>(port); }
    constexpr uint32_t pin_mask() const { return static_cast<uint32_t>(pin); }
};
