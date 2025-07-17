#include "button.h"

// HalButton实现
HalButton::HalButton(const char* name, GPIO::Port port, GPIO::Pin pin)
    : name(name), gpio(port, pin, GPIO::Mode::INPUT), lastState(false), pressedEdge(false) {
}

void HalButton::update() {
    bool current = isPressed();
    pressedEdge = (!lastState && current);
    lastState = current;
}

bool HalButton::isPressed() const {
    return !gpio.input_bit_get(); // 假设低电平为按下
}

bool HalButton::wasPressed() const {
    return pressedEdge;
}

const char* HalButton::getName() const {
    return name;
}

// HalSensor实现
HalSensor::HalSensor(const char* name, GPIO::Port port, GPIO::Pin pin)
    : name(name), gpio(port, pin, GPIO::Mode::INPUT) {
}

bool HalSensor::isTrigger() const {
    return gpio.input_bit_get();
}

const char* HalSensor::getName() const {
    return name;
}

// HalValve实现
HalValve::HalValve(const char* name, GPIO::Port port, GPIO::Pin pin, bool active_high)
    : name(name), active_high(active_high), gpio(port, pin, GPIO::Mode::OUTPUT), isOpenState(false) {
}

void HalValve::open() {
    gpio.bit_write(active_high);
    isOpenState = true;
}

void HalValve::close() {
    gpio.bit_write(!active_high);
    isOpenState = false;
}

void HalValve::toggle() {
    gpio.toggle();
    isOpenState = !isOpenState;
}

bool HalValve::isOpen() const {
    return isOpenState;
}

const char* HalValve::getName() const {
    return name;
}

// HalDipSwitch实现
HalDipSwitch::HalDipSwitch(const DipSwitchInfo& info)
    : keys{HalButton("DIP0", info.pins[0].port, info.pins[0].pin),
           HalButton("DIP1", info.pins[1].port, info.pins[1].pin),
           HalButton("DIP2", info.pins[2].port, info.pins[2].pin),
           HalButton("DIP3", info.pins[3].port, info.pins[3].pin),
           HalButton("DIP4", info.pins[4].port, info.pins[4].pin),
           HalButton("DIP5", info.pins[5].port, info.pins[5].pin),
           HalButton("DIP6", info.pins[6].port, info.pins[6].pin),
           HalButton("DIP7", info.pins[7].port, info.pins[7].pin)} {
}

bool HalDipSwitch::isOn(int index) const {
    if (index < 0 || index >= 8) return false;
    return keys[index].isPressed();
}

uint8_t HalDipSwitch::value() const {
    uint8_t val = 0;
    for (int i = 0; i < 8; ++i) {
        if (isOn(i)) {
            val |= (1 << i);
        }
    }
    return val;
}
