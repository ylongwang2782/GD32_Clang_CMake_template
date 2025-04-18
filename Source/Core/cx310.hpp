#include <array>
#include <cstdint>
#include <string>
#include <vector>

#include "TaskCPP.h"
#include "bsp_uart.hpp"
#include "bsp_gpio.hpp"

class CX310 {
   public:
    struct Config {
        uint8_t preamble_idx;
        uint8_t sfd_id;
        uint8_t psr_sync_len;
        uint8_t mac_datarate;
        uint8_t channel;
        uint8_t phr_datarate;
        uint8_t mac_mode;
    };

    CX310(Uart& logUart) : logUart(logUart) {}
    Uart& logUart;

    void init(const Config& config) {
        uint8_t timedelay = 50;

        GPIO cx310_enable_pin(GPIO::Port::F,GPIO::Pin::PIN_12,GPIO::Mode::OUTPUT);
        cx310_enable_pin.bit_set();
        set_preamble_idx(config.preamble_idx);
        TaskBase::delay(timedelay);
        set_sfd_id(config.sfd_id);
        TaskBase::delay(timedelay);
        set_psr_sync_len(config.psr_sync_len);
        TaskBase::delay(timedelay);
        set_mac_datarate(config.mac_datarate);
        TaskBase::delay(timedelay);
        set_channel(config.channel);
        TaskBase::delay(timedelay);
        set_phr_datarate(config.phr_datarate);
        TaskBase::delay(timedelay);
        set_mac_mode(config.mac_mode);
    }

    void log(std::vector<uint8_t>& data) {
        logUart.data_send(const_cast<uint8_t*>(data.data()), data.size());
    }

    // CX310配置方法
    void set_preamble_idx(uint8_t idx) {
        sendConfigCommand("set_preamble_idx=" + std::to_string(idx));
    }

    void set_sfd_id(uint8_t id) {
        sendConfigCommand("set_sfd_id=" + std::to_string(id));
    }

    void set_psr_sync_len(uint8_t len) {
        sendConfigCommand("set_psr_sync_len=" + std::to_string(len));
    }

    void set_mac_datarate(uint8_t rate) {
        sendConfigCommand("set_mac_datarate=" + std::to_string(rate));
    }

    void set_channel(uint8_t channel) {
        sendConfigCommand("set_channel=" + std::to_string(channel));
    }

    void set_phr_datarate(uint8_t rate) {
        sendConfigCommand("set_phr_datarate=" + std::to_string(rate));
    }

    void set_mac_mode(uint8_t mode) {
        sendConfigCommand("set_mac_mode=" + std::to_string(mode));
    }

   private:
    void sendConfigCommand(const std::string& cmd) {
        std::vector<uint8_t> data(cmd.begin(), cmd.end());
        data.push_back('\n');    // 添加换行符
        logUart.data_send(data.data(), data.size());
    }
};