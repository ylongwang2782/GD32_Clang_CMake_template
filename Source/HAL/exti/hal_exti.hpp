#ifndef BSP_EXTI_HPP
#define BSP_EXTI_HPP
#include <cstdint>
#include <functional>

#include "gd32f4xx.h"

typedef struct {
    uint32_t gpio_periph;
    uint32_t gpio_pin;
    uint32_t pull_up_down;
    exti_trig_type_enum exti_trig;
    uint8_t priority;

} ExitCfg;

extern "C" {
void EXTI0_IRQHandler(void);
void EXTI1_IRQHandler(void);
void EXTI2_IRQHandler(void);
void EXTI3_IRQHandler(void);
void EXTI4_IRQHandler(void);
void EXTI5_9_IRQHandler(void);
void EXTI10_15_IRQHandler(void);
}
class ExtiBase {
   public:
    friend void EXTI0_IRQHandler(void);
    friend void EXTI1_IRQHandler(void);
    friend void EXTI2_IRQHandler(void);
    friend void EXTI3_IRQHandler(void);
    friend void EXTI4_IRQHandler(void);
    friend void EXTI5_9_IRQHandler(void);
    friend void EXTI10_15_IRQHandler(void);
    enum EXTI_LINE : uint8_t {
        _EXTI0 = 0,
        _EXTI1,
        _EXTI2,
        _EXTI3,
        _EXTI4,
        _EXTI5,
        _EXTI6,
        _EXTI7,
        _EXTI8,
        _EXTI9,
        _EXTI10,
        _EXTI11,
        _EXTI12,
        _EXTI13,
        _EXTI14,
        _EXTI15,

        _EXTI_NUM
    };
    ExtiBase(ExitCfg const& _cfg) : cfg(_cfg) {
        bsp_init();
        init();
    }

   public:
    ExitCfg const& cfg;

   private:
    static ExtiBase* exit[_EXTI_NUM];
    static bool is_bsp_init;
    uint32_t exti_port;
    uint32_t exti_pin;
    exti_line_enum exti_line;
    uint8_t exti_irq;

   public:
    void irq_handler() {
        if (exti_interrupt_flag_get(exti_line) != RESET) {
            exti_interrupt_flag_clear(exti_line);
            callback();
        }
    }
    virtual void callback() = 0;

   private:
    void bsp_init() {
        if (!is_bsp_init) {
            is_bsp_init = true;
            for (uint8_t i = 0; i < _EXTI_NUM; i++) {
                exit[i] = nullptr;
            }
        }
    }
    void init() {
        get_exti_port();
        get_exti_pin();
        rcu_periph_clock_enable(RCU_SYSCFG);
        gpio_mode_set(cfg.gpio_periph, GPIO_MODE_INPUT, cfg.pull_up_down,
                      cfg.gpio_pin);

        nvic_irq_enable(exti_irq, cfg.priority, 0U);
        syscfg_exti_line_config(exti_port, exti_pin);
        exti_init(exti_line, EXTI_INTERRUPT, cfg.exti_trig);
        exti_interrupt_flag_clear(exti_line);
    }
    void get_exti_port() {
        switch (cfg.gpio_periph) {
            case GPIOA: {
                rcu_periph_clock_enable(RCU_GPIOA);
                exti_port = EXTI_SOURCE_GPIOA;
                break;
            }
            case GPIOB: {
                rcu_periph_clock_enable(RCU_GPIOB);
                exti_port = EXTI_SOURCE_GPIOB;
                break;
            }
            case GPIOC: {
                rcu_periph_clock_enable(RCU_GPIOC);
                exti_port = EXTI_SOURCE_GPIOC;
                break;
            }
            case GPIOD: {
                rcu_periph_clock_enable(RCU_GPIOD);
                exti_port = EXTI_SOURCE_GPIOD;
                break;
            }
            case GPIOE: {
                rcu_periph_clock_enable(RCU_GPIOE);
                exti_port = EXTI_SOURCE_GPIOE;
                break;
            }
            case GPIOF: {
                rcu_periph_clock_enable(RCU_GPIOF);
                exti_port = EXTI_SOURCE_GPIOF;
                break;
            }
            case GPIOG: {
                rcu_periph_clock_enable(RCU_GPIOG);
                exti_port = EXTI_SOURCE_GPIOG;
                break;
            }
            case GPIOH: {
                rcu_periph_clock_enable(RCU_GPIOH);
                exti_port = EXTI_SOURCE_GPIOH;
                break;
            }
            case GPIOI: {
                rcu_periph_clock_enable(RCU_GPIOI);
                exti_port = EXTI_SOURCE_GPIOI;
                break;
            }
        }
    }
    void get_exti_pin() {
        switch (cfg.gpio_pin) {
            case GPIO_PIN_0: {
                exti_pin = EXTI_SOURCE_PIN0;
                exti_line = EXTI_0;
                exti_irq = EXTI0_IRQn;
                exit[_EXTI0] = this;  // [_EXTI0] = this;
                break;
            }
            case GPIO_PIN_1: {
                exti_pin = EXTI_SOURCE_PIN1;
                exti_line = EXTI_1;
                exti_irq = EXTI1_IRQn;
                exit[_EXTI1] = this;  // [_EXTI1] = this;
                break;
            }
            case GPIO_PIN_2: {
                exti_pin = EXTI_SOURCE_PIN2;
                exti_line = EXTI_2;
                exti_irq = EXTI2_IRQn;
                exit[_EXTI2] = this;  // [_EXTI2] = this;
                break;
            }
            case GPIO_PIN_3: {
                exti_pin = EXTI_SOURCE_PIN3;
                exti_line = EXTI_3;
                exti_irq = EXTI3_IRQn;
                exit[_EXTI3] = this;  // [_EXTI3] = this;
                break;
            }
            case GPIO_PIN_4: {
                exti_pin = EXTI_SOURCE_PIN4;
                exti_line = EXTI_4;
                exti_irq = EXTI4_IRQn;
                exit[_EXTI4] = this;  // [_EXTI4] = this;
                break;
            }
            case GPIO_PIN_5: {
                exti_pin = EXTI_SOURCE_PIN5;
                exti_line = EXTI_5;
                exti_irq = EXTI5_9_IRQn;
                exit[_EXTI5] = this;  // [_EXTI5_9] = this;
                break;
            }
            case GPIO_PIN_6: {
                exti_pin = EXTI_SOURCE_PIN6;
                exti_line = EXTI_6;
                exti_irq = EXTI5_9_IRQn;
                exit[_EXTI6] = this;  // [_EXTI5_9] = this;
                break;
            }
            case GPIO_PIN_7: {
                exti_pin = EXTI_SOURCE_PIN7;
                exti_line = EXTI_7;
                exti_irq = EXTI5_9_IRQn;
                exit[_EXTI7] = this;  // [_EXTI5_9] = this;
                break;
            }
            case GPIO_PIN_8: {
                exti_pin = EXTI_SOURCE_PIN8;
                exti_line = EXTI_8;
                exti_irq = EXTI5_9_IRQn;
                exit[_EXTI8] = this;  // [_EXTI5_9] = this;
                break;
            }
            case GPIO_PIN_9: {
                exti_pin = EXTI_SOURCE_PIN9;
                exti_line = EXTI_9;
                exti_irq = EXTI5_9_IRQn;
                exit[_EXTI9] = this;  // [_EXTI5_9] = this;

                break;
            }
            case GPIO_PIN_10: {
                exti_pin = EXTI_SOURCE_PIN10;
                exti_line = EXTI_10;
                exti_irq = EXTI10_15_IRQn;
                exit[_EXTI10] = this;  // [_EXTI10_15] = this;
                break;
            }
            case GPIO_PIN_11: {
                exti_pin = EXTI_SOURCE_PIN11;
                exti_line = EXTI_11;
                exti_irq = EXTI10_15_IRQn;
                exit[_EXTI11] = this;  // [_EXTI10_15] = this;
                break;
            }
            case GPIO_PIN_12: {
                exti_pin = EXTI_SOURCE_PIN12;
                exti_line = EXTI_12;
                exti_irq = EXTI10_15_IRQn;
                exit[_EXTI12] = this;  // [_EXTI10_15] = this;
                break;
            }
            case GPIO_PIN_13: {
                exti_pin = EXTI_SOURCE_PIN13;
                exti_line = EXTI_13;
                exti_irq = EXTI10_15_IRQn;
                exit[_EXTI13] = this;  // [_EXTI10_15] = this;
                break;
            }
            case GPIO_PIN_14: {
                exti_pin = EXTI_SOURCE_PIN14;
                exti_line = EXTI_14;
                exti_irq = EXTI10_15_IRQn;
                exit[_EXTI14] = this;  // [_EXTI10_15] = this;
                break;
            }
            case GPIO_PIN_15: {
                exti_pin = EXTI_SOURCE_PIN15;
                exti_line = EXTI_15;
                exti_irq = EXTI10_15_IRQn;
                exit[_EXTI15] = this;  // [_EXTI10_15] = this;
                break;
            }
            default: {
                break;
            }
        }
    }
};

template <class cl>
class Exit : private ExtiBase {
   public:
    Exit(cl* _obj, void (cl::*_func)(), ExitCfg const& _cfg)
        : ExtiBase(_cfg), obj(_obj), func(_func) {}

   private:
    cl* obj;
    void (cl::*func)();

   private:
    void callback() override { (obj->*func)(); }
};

#endif