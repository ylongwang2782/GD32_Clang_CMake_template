#include "InputAdapter.h"

InputAdapter::InputAdapter(IButton& button, ISensor& sensor, IValve& valve, IDipSwitch& dipSwitch)
    : button(button), sensor(sensor), valve(valve), dipSwitch(dipSwitch) {
}

// 按钮相关接口实现
void InputAdapter::updateButton() {
    button.update();
}

bool InputAdapter::isButtonPressed() const {
    return button.isPressed();
}

bool InputAdapter::wasButtonPressed() const {
    return button.wasPressed();
}

const char* InputAdapter::getButtonName() const {
    return button.getName();
}

// 传感器相关接口实现
bool InputAdapter::isSensorTriggered() const {
    return sensor.isTrigger();
}

const char* InputAdapter::getSensorName() const {
    return sensor.getName();
}

// 阀门相关接口实现
void InputAdapter::openValve() {
    valve.open();
}

void InputAdapter::closeValve() {
    valve.close();
}

void InputAdapter::toggleValve() {
    valve.toggle();
}

bool InputAdapter::isValveOpen() const {
    return valve.isOpen();
}

const char* InputAdapter::getValveName() const {
    return valve.getName();
}

// 拨码开关相关接口实现
bool InputAdapter::isDipSwitchOn(int index) const {
    return dipSwitch.isOn(index);
}

uint8_t InputAdapter::getDipSwitchValue() const {
    return dipSwitch.value();
}
