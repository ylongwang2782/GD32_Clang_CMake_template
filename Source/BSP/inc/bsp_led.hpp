#ifndef BSP_LED_H
#define BSP_LED_H
extern "C" {
#include "gd32f4xx.h"
}
#include "TaskCPP.h"
class LED {
   private:
    uint32_t port;
    uint32_t pin;

    // 获取 RCC 时钟
    static rcu_periph_enum get_rcc_from_port(uint32_t port) {
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
                while (1) {
                }    // 错误处理
        }
    }

   public:
    LED(uint32_t port, uint32_t pin) : port(port), pin(pin) {
        rcu_periph_clock_enable(get_rcc_from_port(port));

        gpio_mode_set(port, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, pin);
        gpio_output_options_set(port, GPIO_OTYPE_PP, GPIO_OSPEED_50MHZ, pin);
        gpio_bit_reset(port, pin);
    }

    ~LED() {}

    void on() { gpio_bit_set(port, pin); }
    void off() { gpio_bit_reset(port, pin); }
    void toggle() { gpio_bit_toggle(port, pin); }
};

class LedBlinkTask : public TaskClassS<128> {
    private:
     LED &led;
     uint32_t blink_interval;
 
    public:
     LedBlinkTask(LED &led, uint32_t interval)
         : TaskClassS<128>("LedBlinkTask", TaskPrio_Mid),
           led(led),
           blink_interval(interval) {}
 
     void task() override {
         for (;;) {
             led.toggle();
             TaskBase::delay(blink_interval);    // 使用传入的闪烁间隔
         }
     }
 };

#endif    // BSP_LED_H
