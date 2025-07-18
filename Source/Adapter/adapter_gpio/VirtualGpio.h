#ifndef VIRTUAL_GPIO_H
#define VIRTUAL_GPIO_H

#include "IGpio.hpp"
#include <map>
#include <vector>

using namespace Interface;

namespace Adapter {

class VirtualGpio : public IGpio {
  public:
    VirtualGpio() = default;
    virtual ~VirtualGpio() = default;

    // 实现IGpio接口
    bool init(const GpioConfig &config) override;
    GpioState read(uint8_t pin) override;
    bool write(uint8_t pin, GpioState state) override;
    bool setMode(uint8_t pin, GpioMode mode) override;
    std::vector<GpioState> readMultiple(const std::vector<uint8_t> &pins) override;
    bool deinit(uint8_t pin) override;

  private:
    struct PinInfo {
        GpioMode mode;
        GpioState state;
        bool initialized;
        
        PinInfo() : mode(GpioMode::INPUT), state(GpioState::LOW), initialized(false) {}
    };
    
    std::map<uint8_t, PinInfo> pinMap;
};

} // namespace Adapter

#endif // VIRTUAL_GPIO_H 