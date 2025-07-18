# GPIO适配器模块

本模块提供了统一的GPIO接口，支持硬件GPIO和虚拟GPIO两种实现方式。

## 特性

- **统一接口**: 通过`IGpio`接口提供一致的GPIO操作API
- **工厂模式**: 使用`GpioFactory`根据配置自动选择实现
- **硬件支持**: `HardwareGpio`封装了底层HAL GPIO驱动
- **虚拟实现**: `VirtualGpio`提供测试和仿真支持
- **类型安全**: 使用强类型枚举避免参数错误

## 使用方法

### 基本用法

```cpp
#include "GpioFactory.h"

// 创建GPIO实例
auto gpio = Adapter::GpioFactory::createGpio();

// 初始化LED（输出模式）
Interface::GpioConfig ledConfig(0, Interface::GpioMode::OUTPUT, Interface::GpioState::LOW);
gpio->init(ledConfig);

// 控制LED
gpio->write(0, Interface::GpioState::HIGH);  // 点亮LED
gpio->write(0, Interface::GpioState::LOW);   // 熄灭LED

// 初始化按钮（输入上拉模式）
Interface::GpioConfig buttonConfig(1, Interface::GpioMode::INPUT_PULLUP);
gpio->init(buttonConfig);

// 读取按钮状态
Interface::GpioState buttonState = gpio->read(1);
bool isPressed = (buttonState == Interface::GpioState::LOW);
```

### 批量操作

```cpp
// 批量读取多个GPIO
std::vector<uint8_t> pins = {1, 2, 3, 4};
std::vector<Interface::GpioState> states = gpio->readMultiple(pins);
```

### 使用示例类

```cpp
#include "gpio_example.hpp"

Example::GpioExample example;

// 初始化LED
example.initLed(0);
example.setLed(0, true);  // 点亮LED

// 初始化按钮
example.initButton(1);
bool pressed = example.readButton(1);
```

## 配置选项

在CMake中可以通过以下选项控制GPIO实现：

- `GPIO_USE_HARDWARE=ON`: 启用硬件GPIO实现（默认开启）
- `GPIO_USE_VIRTUAL=ON`: 启用虚拟GPIO实现（默认开启）

## 引脚映射

硬件GPIO使用以下引脚映射规则：
- 引脚 0-15: GPIOA
- 引脚 16-31: GPIOB  
- 引脚 32-47: GPIOC
- 依此类推...

## API参考

### IGpio接口

- `bool init(const GpioConfig &config)`: 初始化GPIO引脚
- `GpioState read(uint8_t pin)`: 读取GPIO状态
- `bool write(uint8_t pin, GpioState state)`: 写入GPIO状态
- `bool setMode(uint8_t pin, GpioMode mode)`: 设置GPIO模式
- `std::vector<GpioState> readMultiple(const std::vector<uint8_t> &pins)`: 批量读取
- `bool deinit(uint8_t pin)`: 去初始化GPIO

### GpioFactory工厂类

- `createGpio()`: 根据配置自动选择实现
- `createHardwareGpio()`: 创建硬件GPIO实例
- `createVirtualGpio()`: 创建虚拟GPIO实例
- `createGpio(GpioType type)`: 创建指定类型的GPIO实例 