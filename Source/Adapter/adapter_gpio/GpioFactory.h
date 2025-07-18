#ifndef GPIO_FACTORY_H
#define GPIO_FACTORY_H

#include "IGpio.hpp"
#include <memory>

using namespace Interface;

namespace Adapter {

// GPIO工厂类
class GpioFactory {
  public:
    // 统一的GPIO创建接口 - 根据CMake配置自动选择实现
    static std::unique_ptr<IGpio> createGpio();

    // 显式创建特定类型的GPIO（用于测试和调试）
    static std::unique_ptr<IGpio> createVirtualGpio();
    static std::unique_ptr<IGpio> createHardwareGpio();

    // 根据配置创建GPIO实例（保留用于特殊需求）
    enum class GpioType {
        VIRTUAL, // 虚拟GPIO
        HARDWARE // 硬件GPIO
    };

    static std::unique_ptr<IGpio> createGpio(GpioType type);
};

} // namespace Adapter

#endif // GPIO_FACTORY_H