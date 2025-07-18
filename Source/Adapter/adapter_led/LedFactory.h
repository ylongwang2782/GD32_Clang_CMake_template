#ifndef LED_FACTORY_H
#define LED_FACTORY_H

#include "ILed.hpp"
#include "IGpio.hpp"
#include <memory>

using namespace Interface;

namespace Adapter {

// LED工厂类
class LedFactory {
public:
    // 创建LED适配器实例，使用默认GPIO
    static std::unique_ptr<ILed> createLed();
    
    // 创建LED适配器实例，使用指定的GPIO
    static std::unique_ptr<ILed> createLed(std::unique_ptr<IGpio> gpio);
    
    // 创建预配置的系统LED实例
    static std::unique_ptr<ILed> createSystemLeds();
};

} // namespace Adapter

#endif // LED_FACTORY_H 