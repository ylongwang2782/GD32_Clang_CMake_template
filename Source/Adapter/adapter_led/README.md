# LED适配器模块

基于GPIO适配器构建的高级LED控制模块，提供丰富的LED功能和状态指示。

## 特性

- **高级控制**: 支持常亮、闪烁、呼吸灯等多种效果
- **状态管理**: 提供系统状态、电源状态等语义化控制
- **亮度调节**: 支持0-100%亮度控制（通过软件PWM模拟）
- **灵活配置**: 可自定义闪烁频率和LED引脚映射
- **基于GPIO**: 底层使用GPIO适配器，支持硬件和虚拟实现

## 架构设计

```
应用层 (Application)
    ↓
LED适配器 (LedAdapter)
    ↓
GPIO适配器 (GpioAdapter)
    ↓
HAL层 (hal_gpio.hpp)
```

## 使用方法

### 基本用法

```cpp
#include "LedFactory.h"

// 创建LED实例
auto led = Adapter::LedFactory::createLed();

// 初始化LED
led->init(Interface::LedType::SYSTEM, 13);  // 系统LED在引脚13

// 控制LED
led->setState(Interface::LedType::SYSTEM, Interface::LedState::ON);       // 常亮
led->setState(Interface::LedType::SYSTEM, Interface::LedState::BLINK_SLOW); // 慢闪烁

// 在主循环中更新
while(true) {
    led->update();  // 处理闪烁等效果
    // ... 其他代码
}
```

### 系统状态控制

```cpp
// 创建预配置的系统LED
auto systemLed = Adapter::LedFactory::createSystemLeds();

// 设置系统状态
systemLed->setSystemStatus(false, true);  // 无错误，正在运行
systemLed->setSystemStatus(true, false);  // 有错误，停止运行
systemLed->setPowerStatus(true);          // 电源开启

// 在主循环中更新
while(true) {
    systemLed->update();
}
```

### 高级功能

```cpp
// 亮度控制
led->setBrightness(Interface::LedType::POWER, 50);  // 50%亮度

// 自定义闪烁频率
led->setBlinkRate(Interface::LedType::RUN, 500);    // 500ms周期

// 不同效果演示
led->setState(Interface::LedType::ERROR, Interface::LedState::PULSE);     // 呼吸灯
led->setState(Interface::LedType::RUN, Interface::LedState::BLINK_FAST);  // 快闪烁
```

### 使用示例类

```cpp
#include "led_example.hpp"

Example::LedExample example;

// 设置系统状态
example.setSystemError(false);    // 无错误
example.setSystemRunning(true);   // 正在运行
example.setPowerOn(true);         // 电源开启

// 演示效果
example.demonstrateBlinking();    // 闪烁演示
example.demonstrateBrightness();  // 亮度演示

// 在主循环中更新
while(true) {
    example.update();
}
```

## LED类型定义

- `LedType::SYSTEM` - 系统状态LED
- `LedType::POWER` - 电源状态LED  
- `LedType::RUN` - 运行状态LED
- `LedType::ERROR` - 错误状态LED

## LED状态定义

- `LedState::OFF` - 关闭
- `LedState::ON` - 常亮
- `LedState::BLINK_SLOW` - 慢闪烁（1秒周期）
- `LedState::BLINK_FAST` - 快闪烁（200ms周期）
- `LedState::PULSE` - 呼吸灯效果

## 引脚映射

默认系统LED引脚映射（可在LedFactory中修改）：
- 系统LED: 引脚13 (PC13)
- 电源LED: 引脚25 (PG9)
- 运行LED: 引脚28 (PG12)
- 错误LED: 引脚31 (PG15)

## API参考

### ILed接口

- `bool init(LedType type, uint8_t pin)`: 初始化LED
- `bool setState(LedType type, LedState state)`: 设置LED状态
- `LedState getState(LedType type)`: 获取LED状态
- `bool setBrightness(LedType type, uint8_t brightness)`: 设置亮度(0-100)
- `bool setBlinkRate(LedType type, uint16_t periodMs)`: 设置闪烁周期
- `bool setSystemStatus(bool error, bool running)`: 设置系统状态
- `bool setPowerStatus(bool on)`: 设置电源状态
- `void update()`: 更新LED状态（需在主循环调用）
- `bool deinit(LedType type)`: 去初始化LED

### LedFactory工厂类

- `createLed()`: 创建LED适配器实例
- `createLed(std::unique_ptr<IGpio> gpio)`: 使用指定GPIO创建LED实例
- `createSystemLeds()`: 创建预配置的系统LED实例

## 设计优势

相比直接使用GPIO适配器，LED适配器提供了：

1. **语义清晰**: `setSystemStatus(ERROR)` vs `gpio->write(13, HIGH)`
2. **功能丰富**: 支持闪烁、亮度、呼吸灯等效果
3. **状态管理**: 自动处理复杂的LED状态逻辑
4. **代码复用**: 多个应用可共享LED控制逻辑
5. **易于测试**: 可以创建虚拟LED用于单元测试
6. **配置灵活**: 引脚映射和效果参数可配置

这种分层设计既保持了GPIO的灵活性，又提供了LED特定的高级功能。 