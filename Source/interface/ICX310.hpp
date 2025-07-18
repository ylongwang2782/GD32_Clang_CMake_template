#pragma once
#include <cstdint>
#include <cstdio>
#include <functional>
#include <queue>
#include <vector>

class ICX310 {
   public:
    ICX310() = default;

   public:
    /* 复位引脚初始化 */
    virtual void reset_pin_init() = 0;

    /* 产生复位信号 */
    virtual void generate_reset_signal() = 0;

    /* 关闭复位信号 */
    virtual void turn_of_reset_signal() = 0;

    /* 芯片使能引脚初始化 */
    virtual void chip_en_init() = 0;

    /* 使能芯片 */
    virtual void chip_enable() = 0;

    /* 禁用芯片 */
    virtual void chip_disable() = 0;

    /* 通信接口初始化 */
    virtual void commuication_peripheral_init() = 0;

    /**
     * @brief 发送数据
     * @param tx_data 发送数据
     * @return 发送成功返回true，失败返回false
     */
    virtual bool send(std::vector<uint8_t>& tx_data) = 0;

    /**
     * @brief 接收数据
     * @param rx_data 接收数据
     * @return 接收成功返回true，失败返回false
     */
    virtual bool get_recv_data(std::queue<uint8_t>& rx_data) = 0;

    /* 获取系统1ms时间戳 */
    virtual uint32_t get_system_1ms_ticks() = 0;

    /* 延迟1ms */
    virtual void delay_ms(uint32_t ms) = 0;

    /* 日志输出 */
    virtual void log(const char* format, ...) {}
};