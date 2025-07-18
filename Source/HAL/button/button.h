#pragma once

#include "IButton.hpp"
#include "hal_gpio.hpp"

// HAL层按钮实现
class HalButton : public IButton {
public:
    HalButton(const char* name, GPIO::Port port, GPIO::Pin pin);
    
    // 实现IButton接口
    void update() override;
    bool isPressed() const override;
    bool wasPressed() const override;
    const char* getName() const override;

private:
    const char* name;
    GPIO gpio;
    bool lastState;
    bool pressedEdge;
};

// HAL层传感器实现
class HalSensor : public ISensor {
public:
    HalSensor(const char* name, GPIO::Port port, GPIO::Pin pin);
    
    // 实现ISensor接口
    bool isTrigger() const override;
    const char* getName() const override;

private:
    const char* name;
    GPIO gpio;
};

// HAL层阀门实现
class HalValve : public IValve {
public:
    HalValve(const char* name, GPIO::Port port, GPIO::Pin pin, bool active_high = true);
    
    // 实现IValve接口
    void open() override;
    void close() override;
    void toggle() override;
    bool isOpen() const override;
    const char* getName() const override;

private:
    const char* name;
    bool active_high;
    GPIO gpio;
    bool isOpenState;
};

// 拨码开关信息结构体
#ifndef DIPSWITCHINFO_DEFINED
#define DIPSWITCHINFO_DEFINED
struct DipSwitchInfo {
    struct PinInfo {
        GPIO::Port port;
        GPIO::Pin pin;
    };
    PinInfo pins[8];
};
#endif

// HAL层拨码开关实现
class HalDipSwitch : public IDipSwitch {
public:
    HalDipSwitch(const DipSwitchInfo& info);
    
    // 实现IDipSwitch接口
    bool isOn(int index) const override;
    uint8_t value() const override;

private:
    HalButton keys[8];
};
