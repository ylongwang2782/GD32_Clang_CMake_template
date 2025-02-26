#pragma once
#include "bsp_gpio.hpp"
class LED {
   public:
    LED(GPIO::Port port, GPIO::Pin pin) : gpio(port, pin, GPIO::Mode::OUTPUT) {}

    void on() { gpio.bit_set(); }
    void off() { gpio.bit_reset(); }
    void toggle() { gpio.toggle(); }

   private:
    GPIO gpio;
};