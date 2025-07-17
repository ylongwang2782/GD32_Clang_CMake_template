#include "LedFactory.h"
#include "LedAdapter.h"
#include "../adapter_gpio/GpioFactory.h"

namespace Adapter {

std::unique_ptr<ILed> LedFactory::createLed() {
    // 使用GPIO工厂创建默认GPIO实例
    auto gpio = GpioFactory::createGpio();
    return std::make_unique<LedAdapter>(std::move(gpio));
}

std::unique_ptr<ILed> LedFactory::createLed(std::unique_ptr<IGpio> gpio) {
    return std::make_unique<LedAdapter>(std::move(gpio));
}

std::unique_ptr<ILed> LedFactory::createSystemLeds() {
    auto led = createLed();
    
    // 根据peripherals.hpp中的定义初始化系统LED
    // 这里使用示例引脚，实际项目中应该从配置文件读取
    led->init(LedType::RUN, 32+13);  // RUN_LED_PIN (PC13)
    led->init(LedType::POWER, 96+9);   // PWR_LED_PIN (PG9)  
    led->init(LedType::SYSTEM, 96+12);     // SYS_LED_PIN (PG12)
    led->init(LedType::ERROR, 96+15);   // ERR_LED_PIN (PG15)
    
    // 设置初始状态
    led->setPowerStatus(true);  // 电源LED常亮
    led->setSystemStatus(false, true);  // 系统正常，运行中
    
    return led;
}

} // namespace Adapter 