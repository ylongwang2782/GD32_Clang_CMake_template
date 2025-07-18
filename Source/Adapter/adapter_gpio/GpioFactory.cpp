#include "GpioFactory.h"

// 根据编译配置选择性包含硬件GPIO
#include "HardwareGpio.h"
#include "VirtualGpio.h"

namespace Adapter {

// 统一的GPIO创建接口 - 根据CMake配置自动选择实现
std::unique_ptr<IGpio> GpioFactory::createGpio() {
#ifdef GPIO_USE_HARDWARE
    return createHardwareGpio();
#else
    return createVirtualGpio();
#endif
}

std::unique_ptr<IGpio> GpioFactory::createVirtualGpio() {
    return std::make_unique<VirtualGpio>();
}

std::unique_ptr<IGpio> GpioFactory::createHardwareGpio() {
    return std::make_unique<HardwareGpio>();
}

std::unique_ptr<IGpio> GpioFactory::createGpio(GpioType type) {
    switch (type) {
        case GpioType::HARDWARE:
            return createHardwareGpio();
        case GpioType::VIRTUAL:
            return createVirtualGpio();
        default:
            return createVirtualGpio();    // 默认返回虚拟GPIO
    }
}

}    // namespace Adapter