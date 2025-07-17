#ifndef ILED_HPP
#define ILED_HPP

#include <cstdint>

namespace Interface {

// LED状态枚举
enum class LedState : uint8_t {
    OFF = 0,        // 关闭
    ON = 1,         // 常亮
    BLINK_SLOW = 2, // 慢闪烁
    BLINK_FAST = 3, // 快闪烁
    PULSE = 4       // 呼吸灯效果
};

// LED颜色枚举
enum class LedColor : uint8_t {
    RED = 0,
    GREEN = 1,
    BLUE = 2,
    YELLOW = 3,
    WHITE = 4
};

// LED类型枚举
enum class LedType : uint8_t {
    SYSTEM = 0,  // 系统状态LED
    POWER = 1,   // 电源LED
    RUN = 2,     // 运行LED
    ERROR = 3    // 错误LED
};

// LED接口抽象基类
class ILed {
public:
    virtual ~ILed() = default;

    // 基本控制
    virtual bool init(LedType type, uint8_t pin) = 0;
    virtual bool setState(LedType type, LedState state) = 0;
    virtual LedState getState(LedType type) = 0;
    
    // 高级功能
    virtual bool setBrightness(LedType type, uint8_t brightness) = 0; // 0-100
    virtual bool setBlinkRate(LedType type, uint16_t periodMs) = 0;
    
    // 状态指示快捷方法
    virtual bool setSystemStatus(bool error, bool running = true) = 0;
    virtual bool setPowerStatus(bool on) = 0;
    
    // 更新函数（需要在主循环中调用）
    virtual void update() = 0;
    
    // 去初始化
    virtual bool deinit(LedType type) = 0;
};

} // namespace Interface

#endif // ILED_HPP 