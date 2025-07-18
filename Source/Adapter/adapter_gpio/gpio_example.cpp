#include "gpio_example.hpp"

namespace Example {

GpioExample::GpioExample() {
    // 使用工厂创建GPIO实例
    gpio = Adapter::GpioFactory::createGpio();
}

bool GpioExample::initLed(uint8_t pin) {
    Interface::GpioConfig config(pin, Interface::GpioMode::OUTPUT, Interface::GpioState::LOW);
    return gpio->init(config);
}

bool GpioExample::setLed(uint8_t pin, bool on) {
    Interface::GpioState state = on ? Interface::GpioState::HIGH : Interface::GpioState::LOW;
    return gpio->write(pin, state);
}

bool GpioExample::initButton(uint8_t pin) {
    Interface::GpioConfig config(pin, Interface::GpioMode::INPUT_PULLUP);
    return gpio->init(config);
}

bool GpioExample::readButton(uint8_t pin) {
    Interface::GpioState state = gpio->read(pin);
    // 按钮通常是低电平有效（按下时为低）
    return state == Interface::GpioState::LOW;
}

bool GpioExample::initMultipleLeds(const std::vector<uint8_t>& pins) {
    bool allSuccess = true;
    for (uint8_t pin : pins) {
        if (!initLed(pin)) {
            allSuccess = false;
        }
    }
    return allSuccess;
}

std::vector<bool> GpioExample::readMultipleButtons(const std::vector<uint8_t>& pins) {
    std::vector<Interface::GpioState> states = gpio->readMultiple(pins);
    std::vector<bool> results;
    results.reserve(states.size());
    
    for (Interface::GpioState state : states) {
        // 按钮通常是低电平有效
        results.push_back(state == Interface::GpioState::LOW);
    }
    
    return results;
}

} // namespace Example 