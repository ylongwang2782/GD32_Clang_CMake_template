#include "VirtualGpio.h"

namespace Adapter {

bool VirtualGpio::init(const GpioConfig &config) {
    PinInfo &pinInfo = pinMap[config.pin];
    pinInfo.mode = config.mode;
    pinInfo.initialized = true;
    
    // 如果是输出模式，设置初始状态
    if (config.mode == GpioMode::OUTPUT) {
        pinInfo.state = config.initState;
    }
    
    return true;
}

GpioState VirtualGpio::read(uint8_t pin) {
    auto it = pinMap.find(pin);
    if (it != pinMap.end() && it->second.initialized) {
        return it->second.state;
    }
    return GpioState::LOW; // 默认返回低电平
}

bool VirtualGpio::write(uint8_t pin, GpioState state) {
    auto it = pinMap.find(pin);
    if (it != pinMap.end() && it->second.initialized) {
        // 只有输出模式才能写入
        if (it->second.mode == GpioMode::OUTPUT) {
            it->second.state = state;
            return true;
        }
    }
    return false;
}

bool VirtualGpio::setMode(uint8_t pin, GpioMode mode) {
    auto it = pinMap.find(pin);
    if (it != pinMap.end() && it->second.initialized) {
        it->second.mode = mode;
        return true;
    }
    return false;
}

std::vector<GpioState> VirtualGpio::readMultiple(const std::vector<uint8_t> &pins) {
    std::vector<GpioState> results;
    results.reserve(pins.size());
    
    for (uint8_t pin : pins) {
        results.push_back(read(pin));
    }
    
    return results;
}

bool VirtualGpio::deinit(uint8_t pin) {
    auto it = pinMap.find(pin);
    if (it != pinMap.end()) {
        pinMap.erase(it);
        return true;
    }
    return false;
}

} // namespace Adapter 