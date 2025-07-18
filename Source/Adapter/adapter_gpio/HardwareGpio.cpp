#include "HardwareGpio.h"

#include "Logger.h"

namespace Adapter {

HardwareGpio::HardwareGpio() {
}

bool HardwareGpio::init(const GpioConfig &config) {
    Log::t("HardwareGpio", "init pin: %d", config.pin);
    GPIO::Port port = getPortFromPin(config.pin);
    Log::t("HardwareGpio", "port: %d", port);
    GPIO::Pin pin = getPinFromPin(config.pin);
    Log::t("HardwareGpio", "pin: %d", pin);
    GPIO::Mode mode = convertMode(config.mode);
    Log::t("HardwareGpio", "mode: %d", mode);
    GPIO::PullUpDown pullUpDown = convertPullUpDown(config.mode);
    Log::t("HardwareGpio", "pullUpDown: %d", pullUpDown);

    // 创建GPIO实例
    auto gpio = std::make_unique<GPIO>(port, pin, mode, pullUpDown);
    Log::t("HardwareGpio", "gpio: %p", gpio.get());

    // 如果是输出模式，设置初始状态
    if (config.mode == GpioMode::OUTPUT) {
        if (config.initState == GpioState::HIGH) {
            gpio->bit_set();
        } else {
            gpio->bit_reset();
        }
    }

    // 存储GPIO实例
    gpioMap[config.pin] = std::move(gpio);
    Log::t("HardwareGpio", "gpioMap[%d] = %p", config.pin,
           gpioMap[config.pin].get());
    return true;
}

GpioState HardwareGpio::read(uint8_t pin) {
    auto it = gpioMap.find(pin);
    if (it != gpioMap.end()) {
        FlagStatus status = it->second->input_bit_get();
        return (status == SET) ? GpioState::HIGH : GpioState::LOW;
    }
    return GpioState::LOW;    // 默认返回低电平
}

bool HardwareGpio::write(uint8_t pin, GpioState state) {
    auto it = gpioMap.find(pin);
    if (it != gpioMap.end()) {
        if (state == GpioState::HIGH) {
            it->second->bit_set();
        } else {
            it->second->bit_reset();
        }
        return true;
    }
    return false;
}

bool HardwareGpio::setMode(uint8_t pin, GpioMode mode) {
    auto it = gpioMap.find(pin);
    if (it != gpioMap.end()) {
        GPIO::Mode gpioMode = convertMode(mode);
        GPIO::PullUpDown pullUpDown = convertPullUpDown(mode);
        it->second->mode_set(gpioMode, pullUpDown);
        return true;
    }
    return false;
}

std::vector<GpioState> HardwareGpio::readMultiple(
    const std::vector<uint8_t> &pins) {
    std::vector<GpioState> results;
    results.reserve(pins.size());

    for (uint8_t pin : pins) {
        results.push_back(read(pin));
    }

    return results;
}

bool HardwareGpio::deinit(uint8_t pin) {
    auto it = gpioMap.find(pin);
    if (it != gpioMap.end()) {
        gpioMap.erase(it);
        return true;
    }
    return false;
}

// 私有辅助函数实现
GPIO::Mode HardwareGpio::convertMode(GpioMode mode) {
    switch (mode) {
        case GpioMode::INPUT:
        case GpioMode::INPUT_PULLUP:
        case GpioMode::INPUT_PULLDOWN:
            return GPIO::Mode::INPUT;
        case GpioMode::OUTPUT:
            return GPIO::Mode::OUTPUT;
        default:
            return GPIO::Mode::INPUT;
    }
}

GPIO::PullUpDown HardwareGpio::convertPullUpDown(GpioMode mode) {
    switch (mode) {
        case GpioMode::INPUT_PULLUP:
            return GPIO::PullUpDown::PULLUP;
        case GpioMode::INPUT_PULLDOWN:
            return GPIO::PullUpDown::PULLDOWN;
        case GpioMode::INPUT:
        case GpioMode::OUTPUT:
        default:
            return GPIO::PullUpDown::NONE;
    }
}

GPIO::Port HardwareGpio::getPortFromPin(uint8_t pin) {
    // 简单的引脚到端口映射，可以根据具体硬件调整
    // 假设引脚编号规则：0-15为PORTA，16-31为PORTB，以此类推
    uint8_t portIndex = pin / 16;
    switch (portIndex) {
        case 0:
            return GPIO::Port::A;
        case 1:
            return GPIO::Port::B;
        case 2:
            return GPIO::Port::C;
        case 3:
            return GPIO::Port::D;
        case 4:
            return GPIO::Port::E;
        case 5:
            return GPIO::Port::F;
        case 6:
            return GPIO::Port::G;
        default:
            return GPIO::Port::A;
    }
}

GPIO::Pin HardwareGpio::getPinFromPin(uint8_t pin) {
    // 获取端口内的引脚编号
    uint8_t pinIndex = pin % 16;
    switch (pinIndex) {
        case 0:
            return GPIO::Pin::PIN_0;
        case 1:
            return GPIO::Pin::PIN_1;
        case 2:
            return GPIO::Pin::PIN_2;
        case 3:
            return GPIO::Pin::PIN_3;
        case 4:
            return GPIO::Pin::PIN_4;
        case 5:
            return GPIO::Pin::PIN_5;
        case 6:
            return GPIO::Pin::PIN_6;
        case 7:
            return GPIO::Pin::PIN_7;
        case 8:
            return GPIO::Pin::PIN_8;
        case 9:
            return GPIO::Pin::PIN_9;
        case 10:
            return GPIO::Pin::PIN_10;
        case 11:
            return GPIO::Pin::PIN_11;
        case 12:
            return GPIO::Pin::PIN_12;
        case 13:
            return GPIO::Pin::PIN_13;
        case 14:
            return GPIO::Pin::PIN_14;
        case 15:
            return GPIO::Pin::PIN_15;
        default:
            return GPIO::Pin::PIN_0;
    }
}

}    // namespace Adapter