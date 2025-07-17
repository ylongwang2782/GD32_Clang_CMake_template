#ifndef LED_ADAPTER_H
#define LED_ADAPTER_H

#include "ILed.hpp"
#include "IGpio.hpp"
#include <map>
#include <memory>

using namespace Interface;

namespace Adapter {

class LedAdapter : public ILed {
public:
    explicit LedAdapter(std::unique_ptr<IGpio> gpio);
    virtual ~LedAdapter() = default;

    // 实现ILed接口
    bool init(LedType type, uint8_t pin) override;
    bool setState(LedType type, LedState state) override;
    LedState getState(LedType type) override;
    
    bool setBrightness(LedType type, uint8_t brightness) override;
    bool setBlinkRate(LedType type, uint16_t periodMs) override;
    
    bool setSystemStatus(bool error, bool running = true) override;
    bool setPowerStatus(bool on) override;
    
    void update() override;
    
    bool deinit(LedType type) override;

private:
    struct LedInfo {
        uint8_t pin;
        LedState state;
        uint8_t brightness;
        uint16_t blinkPeriodMs;
        uint32_t lastToggleTime;
        bool currentPhysicalState;
        bool initialized;
        
        LedInfo() : pin(0), state(LedState::OFF), brightness(100), 
                   blinkPeriodMs(1000), lastToggleTime(0), 
                   currentPhysicalState(false), initialized(false) {}
    };
    
    std::unique_ptr<IGpio> gpio;
    std::map<LedType, LedInfo> ledMap;
    
    // 辅助函数
    uint32_t getCurrentTime(); // 获取当前时间（毫秒）
    void updateLedPhysicalState(LedType type);
    bool setPhysicalState(LedType type, bool on);
};

} // namespace Adapter

#endif // LED_ADAPTER_H 