#include "LedAdapter.h"
#include "FreeRTOS.h"
#include "task.h"
#include "Logger.h"
#include "hal_hptimer.hpp"

namespace Adapter {

LedAdapter::LedAdapter(std::unique_ptr<IGpio> gpio) : gpio(std::move(gpio)) {
    // 构造函数
}

bool LedAdapter::init(LedType type, uint8_t pin) {
    // 初始化GPIO
    GpioConfig config(pin, GpioMode::OUTPUT, GpioState::LOW);
    if (!gpio->init(config)) {
        return false;
    }
    
    // 初始化LED信息
    LedInfo& info = ledMap[type];
    info.pin = pin;
    info.state = LedState::OFF;
    info.initialized = true;
    info.lastToggleTime = getCurrentTime();
    
    // 确保LED初始状态为关闭
    setPhysicalState(type, false);
    
    return true;
}

bool LedAdapter::setState(LedType type, LedState state) {
    auto it = ledMap.find(type);
    if (it == ledMap.end() || !it->second.initialized) {
        return false;
    }
    
    // 只在状态真正改变时重置时间
    if (it->second.state != state) {
        it->second.state = state;
        it->second.lastToggleTime = getCurrentTime();
        
        // 对于闪烁状态，初始化物理状态
        if (state == LedState::BLINK_SLOW || state == LedState::BLINK_FAST || state == LedState::PULSE) {
            it->second.currentPhysicalState = false;  // 从OFF开始闪烁
        }
    }
    
    // 立即更新物理状态
    updateLedPhysicalState(type);
    
    return true;
}

LedState LedAdapter::getState(LedType type) {
    auto it = ledMap.find(type);
    if (it != ledMap.end() && it->second.initialized) {
        return it->second.state;
    }
    return LedState::OFF;
}

bool LedAdapter::setBrightness(LedType type, uint8_t brightness) {
    auto it = ledMap.find(type);
    if (it == ledMap.end() || !it->second.initialized) {
        return false;
    }
    
    it->second.brightness = brightness > 100 ? 100 : brightness;
    return true;
}

bool LedAdapter::setBlinkRate(LedType type, uint16_t periodMs) {
    auto it = ledMap.find(type);
    if (it == ledMap.end() || !it->second.initialized) {
        return false;
    }
    
    it->second.blinkPeriodMs = periodMs;
    return true;
}

bool LedAdapter::setSystemStatus(bool error, bool running) {
    bool success = true;
    
    // 设置错误LED
    if (ledMap.find(LedType::ERROR) != ledMap.end()) {
        success &= setState(LedType::ERROR, error ? LedState::ON : LedState::OFF);
    }
    
    // 设置运行LED
    if (ledMap.find(LedType::RUN) != ledMap.end()) {
        if (error) {
            success &= setState(LedType::RUN, LedState::OFF);
        } else {
            LedState targetState = running ? LedState::BLINK_SLOW : LedState::OFF;
            success &= setState(LedType::RUN, targetState);
        }
    }
    
    return success;
}

bool LedAdapter::setPowerStatus(bool on) {
    auto it = ledMap.find(LedType::POWER);
    if (it != ledMap.end() && it->second.initialized) {
        return setState(LedType::POWER, on ? LedState::ON : LedState::OFF);
    }
    return false;
}

void LedAdapter::update() {
    for (auto& pair : ledMap) {
        if (pair.second.initialized) {
            updateLedPhysicalState(pair.first);
        }
    }
}

bool LedAdapter::deinit(LedType type) {
    auto it = ledMap.find(type);
    if (it != ledMap.end()) {
        // 关闭LED
        setPhysicalState(type, false);
        // 去初始化GPIO
        gpio->deinit(it->second.pin);
        // 移除记录
        ledMap.erase(it);
        return true;
    }
    return false;
}

// 私有辅助函数实现
uint32_t LedAdapter::getCurrentTime() {
    return  hal_hptimer_get_ms();
}

void LedAdapter::updateLedPhysicalState(LedType type) {
    auto it = ledMap.find(type);
    if (it == ledMap.end() || !it->second.initialized) {
        return;
    }
    
    LedInfo& info = it->second;
    uint32_t currentTime = getCurrentTime();
    bool newState = false;
    
    switch (info.state) {
        case LedState::OFF:
            newState = false;
            break;
            
        case LedState::ON:
            newState = true;
            break;
            
        case LedState::BLINK_SLOW:
            // 慢闪烁：默认1秒周期
            {
                uint32_t timeDiff = currentTime - info.lastToggleTime;
                uint32_t halfPeriod = info.blinkPeriodMs / 2;
                
                if (timeDiff >= halfPeriod) {
                    info.currentPhysicalState = !info.currentPhysicalState;
                    info.lastToggleTime = currentTime;
                }
                newState = info.currentPhysicalState;
            }
            break;
            
        case LedState::BLINK_FAST: {
            // 快闪烁：默认200ms周期
            uint16_t fastPeriod = info.blinkPeriodMs / 5;
            if (currentTime - info.lastToggleTime >= fastPeriod / 2) {
                info.currentPhysicalState = !info.currentPhysicalState;
                info.lastToggleTime = currentTime;
            }
            newState = info.currentPhysicalState;
            break;
        }
            
        case LedState::PULSE:
            // 呼吸灯效果（简化为慢闪烁）
            if (currentTime - info.lastToggleTime >= info.blinkPeriodMs / 4) {
                info.currentPhysicalState = !info.currentPhysicalState;
                info.lastToggleTime = currentTime;
            }
            newState = info.currentPhysicalState;
            break;
    }
    
    // // 应用亮度调节（简化实现，实际项目中可能需要PWM）
    // if (newState && info.brightness < 100) {
    //     // 简单的亮度控制：通过快速闪烁模拟PWM
    //     uint32_t pwmPeriod = 10; // 10ms PWM周期
    //     uint32_t onTime = (pwmPeriod * info.brightness) / 100;
    //     uint32_t cycleTime = currentTime % pwmPeriod;
    //     newState = (cycleTime < onTime);
    // }
    
    setPhysicalState(type, newState);
}

bool LedAdapter::setPhysicalState(LedType type, bool on) {
    auto it = ledMap.find(type);
    if (it == ledMap.end() || !it->second.initialized) {
        return false;
    }
    
    GpioState state = on ? GpioState::HIGH : GpioState::LOW;
    // Log::d("LedAdapter", "setPhysicalState: pin=%d, state=%d", it->second.pin, state);
    return gpio->write(it->second.pin, state);
}

} // namespace Adapter 