#ifndef GPIO_EXAMPLE_HPP
#define GPIO_EXAMPLE_HPP

#include "GpioFactory.h"
#include <memory>

namespace Example {

class GpioExample {
public:
    GpioExample();
    ~GpioExample() = default;
    
    // 示例：初始化LED GPIO
    bool initLed(uint8_t pin);
    
    // 示例：控制LED
    bool setLed(uint8_t pin, bool on);
    
    // 示例：初始化按钮GPIO
    bool initButton(uint8_t pin);
    
    // 示例：读取按钮状态
    bool readButton(uint8_t pin);
    
    // 示例：批量操作多个GPIO
    bool initMultipleLeds(const std::vector<uint8_t>& pins);
    std::vector<bool> readMultipleButtons(const std::vector<uint8_t>& pins);

private:
    std::unique_ptr<Interface::IGpio> gpio;
};

} // namespace Example

#endif // GPIO_EXAMPLE_HPP 