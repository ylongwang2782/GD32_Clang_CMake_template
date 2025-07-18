#pragma once

#include <stdint.h>

// 按钮接口定义
class IButton {
public:
    virtual ~IButton() = default;
    
    // 更新按钮状态
    virtual void update() = 0;
    
    // 检查是否按下
    virtual bool isPressed() const = 0;
    
    // 检查是否刚刚按下（边沿触发）
    virtual bool wasPressed() const = 0;
    
    // 获取按钮名称
    virtual const char* getName() const = 0;
};

// 传感器接口定义
class ISensor {
public:
    virtual ~ISensor() = default;
    
    // 检查是否触发
    virtual bool isTrigger() const = 0;
    
    // 获取传感器名称
    virtual const char* getName() const = 0;
};

// 阀门接口定义
class IValve {
public:
    virtual ~IValve() = default;
    
    // 打开阀门
    virtual void open() = 0;
    
    // 关闭阀门
    virtual void close() = 0;
    
    // 切换阀门状态
    virtual void toggle() = 0;
    
    // 检查是否打开
    virtual bool isOpen() const = 0;
    
    // 获取阀门名称
    virtual const char* getName() const = 0;
};

// 拨码开关接口定义
class IDipSwitch {
public:
    virtual ~IDipSwitch() = default;
    
    // 获取某一位拨码开关的状态
    virtual bool isOn(int index) const = 0;
    
    // 获取整个8位拨码开关的值
    virtual uint8_t value() const = 0;
};
