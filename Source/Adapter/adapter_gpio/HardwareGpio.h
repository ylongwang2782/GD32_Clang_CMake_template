#ifndef HARDWARE_GPIO_H
#define HARDWARE_GPIO_H

#include <map>
#include <memory>

#include "../../HAL/gpio/hal_gpio.hpp"
#include "IGpio.hpp"


using namespace Interface;

namespace Adapter {

class HardwareGpio : public IGpio {
   public:
    HardwareGpio();
    virtual ~HardwareGpio() = default;

    // 实现IGpio接口
    bool init(const GpioConfig &config) override;
    GpioState read(uint8_t pin) override;
    bool write(uint8_t pin, GpioState state) override;
    bool setMode(uint8_t pin, GpioMode mode) override;
    std::vector<GpioState> readMultiple(
        const std::vector<uint8_t> &pins) override;
    bool deinit(uint8_t pin) override;

   private:
    // 存储GPIO实例的映射
    std::map<uint8_t, std::unique_ptr<GPIO>> gpioMap;

    // 辅助函数：将接口枚举转换为hal_gpio枚举
    GPIO::Mode convertMode(GpioMode mode);
    GPIO::PullUpDown convertPullUpDown(GpioMode mode);
    GPIO::Port getPortFromPin(uint8_t pin);
    GPIO::Pin getPinFromPin(uint8_t pin);
};

}    // namespace Adapter

#endif    // HARDWARE_GPIO_H