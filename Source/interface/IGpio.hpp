#ifndef IGPIO_H
#define IGPIO_H

#include <cstdint>
#include <memory>
#include <vector>

namespace Interface {

// GPIO引脚状态枚举
enum class GpioState : uint8_t {
    LOW = 0, // 低电平
    HIGH = 1 // 高电平
};

// GPIO引脚模式枚举
enum class GpioMode : uint8_t {
    INPUT = 0,         // 输入模式
    OUTPUT = 1,        // 输出模式
    INPUT_PULLUP = 2,  // 输入上拉模式
    INPUT_PULLDOWN = 3 // 输入下拉模式
};

// GPIO引脚配置结构
struct GpioConfig {
    uint8_t pin;         // 引脚编号
    GpioMode mode;       // 引脚模式
    GpioState initState; // 初始状态（仅对输出模式有效）

    GpioConfig(uint8_t p, GpioMode m, GpioState init = GpioState::LOW)
        : pin(p), mode(m), initState(init) {}
};

// GPIO接口抽象基类
class IGpio {
  public:
    virtual ~IGpio() = default;

    // 初始化GPIO引脚
    virtual bool init(const GpioConfig &config) = 0;

    // 读取GPIO引脚状态
    virtual GpioState read(uint8_t pin) = 0;

    // 写入GPIO引脚状态（仅对输出模式有效）
    virtual bool write(uint8_t pin, GpioState state) = 0;

    // 设置GPIO引脚模式
    virtual bool setMode(uint8_t pin, GpioMode mode) = 0;

    // 批量读取多个GPIO引脚状态
    virtual std::vector<GpioState>
    readMultiple(const std::vector<uint8_t> &pins) = 0;

    // 去初始化GPIO引脚
    virtual bool deinit(uint8_t pin) = 0;
};

} // namespace Interface

#endif // IGPIO_H