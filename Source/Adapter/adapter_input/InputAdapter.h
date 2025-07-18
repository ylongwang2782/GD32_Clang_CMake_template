#pragma once

#include "IButton.hpp"

// InputAdapter适配器层，提供应用层接口
class InputAdapter {
public:
    InputAdapter(IButton& button, ISensor& sensor, IValve& valve, IDipSwitch& dipSwitch);
    
    // 按钮相关接口
    void updateButton();
    bool isButtonPressed() const;
    bool wasButtonPressed() const;
    const char* getButtonName() const;
    
    // 传感器相关接口
    bool isSensorTriggered() const;
    const char* getSensorName() const;
    
    // 阀门相关接口
    void openValve();
    void closeValve();
    void toggleValve();
    bool isValveOpen() const;
    const char* getValveName() const;
    
    // 拨码开关相关接口
    bool isDipSwitchOn(int index) const;
    uint8_t getDipSwitchValue() const;

private:
    IButton& button;
    ISensor& sensor;
    IValve& valve;
    IDipSwitch& dipSwitch;
};
