#ifndef _UWB_HPP
#define _UWB_HPP

#include <cstdarg>
#include <cstdint>
#include <cstdio>
#include <functional>
#include <queue>
#include <vector>

// #include "bsp_log.hpp"
#include "cx_uci.hpp"

// extern Logger Log;

// extern Uart log_com;
// #define DEBUG_IN                                  \
//     if (ds) {                                     \
//         ds = false;                               \
//         log_com.data_send((uint8_t*)"in\r\n", 5); \
//     }

#define UWB_GENERAL_TIMEOUT_MS 1000

class CxUwbInterface {
   public:
    CxUwbInterface() = default;

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
    // virtual bool get_recv_data(std::vector<uint8_t>& rx_data) = 0;
    virtual bool get_recv_data(std::queue<uint8_t>& rx_data) = 0;

    /* 获取系统1ms时间戳 */
    virtual uint32_t get_system_1ms_ticks() = 0;

    /* 延迟1ms */
    virtual void delay_ms(uint32_t ms) = 0;

    /* 日志输出 */
    virtual void log(const char* format, ...) {}
};

template <class Interface>
class UWB {
   public:
    explicit UWB() : interface() {}
    explicit UWB(const Interface& i) : interface(i) { __init(); }

    ~UWB() {}

   private:
    enum UwbsSTA : uint8_t { BOOT = 0, READY, ACTIVE, ERROR };

    Interface interface;
    UwbsSTA uwbs_sta = BOOT;
    bool init_success = false;

    UciCtrlPacket recv_packet;
    UciCMD uci_cmd;
    UciNTF uci_ntf;

    std::vector<uint8_t> rx_raw_buffer_vec;
    // std::vector<uint8_t> rx_payload;
    std::queue<uint8_t, std::deque<uint8_t>> rx_data_queue;
    std::queue<uint8_t, std::deque<uint8_t>> transparent_data;
    uint8_t _data;

    std::function<bool(const UciCtrlPacket&)> check_rsp = nullptr;
    std::function<bool()> cmd_packer = nullptr;
    // bool ds = false;

   public:
    bool init() {
        if (init_success) {
            if (!set_hprf()) {
                return false;
            }
            if (!set_channel(PARAM_CHANNEL_NUMBER_9)) {
                return false;
            }
            if (!set_prf_mode(PARAM_PRF_NOMINAL_64_M)) {
                return false;
            }
            if (!set_preamble_length(PARAM_PREAMBLE_LEN_BPRF_64)) {
                return false;
            }
            if (!set_preamble_index(9)) {
                return false;
            }
            if (!set_psdu_data_rate(PARAM_PSDU_DATA_RATE_7_8)) {
                return false;
            }
            if (!set_phr_mode(PARAM_PHYDATARATE_DRHM_HR)) {
                return false;
            }
            if (!set_sfd_id(2)) {
                return false;
            }
            return true;
        }
        return false;
    }

    /**
     * @brief 初始化
     * @return 初始化成功返回true，失败返回false
     */
    bool reset(uint16_t timeout_ms = UWB_GENERAL_TIMEOUT_MS) {
        uwbs_sta = BOOT;
        uci_cmd.core_device_reset();
        check_rsp = [this](const UciCtrlPacket& rsp) {
            return uci_cmd.check_core_device_reset_rsp(rsp);
        };

        cmd_packer = [this]() { return uci_cmd.core_device_reset(); };

        if (__send_packet()) {
            uint32_t start_tick = interface.get_system_1ms_ticks();
            while (interface.get_system_1ms_ticks() - start_tick < timeout_ms) {
                update();
                if (uwbs_sta == READY) {
                    interface.log("UWB: UWBS software reset successfully");
                    return true;
                }
            }
        }
        interface.log("UWB: error: software reset fail");
        interface.log("UWB: error: hardware reset start");

        interface.generate_reset_signal();
        interface.delay_ms(100);
        interface.turn_of_reset_signal();
        uint32_t start_tick = interface.get_system_1ms_ticks();
        while (interface.get_system_1ms_ticks() - start_tick < timeout_ms) {
            update();
            if (uwbs_sta == READY) {
                interface.log("UWB: hardware reset successfully");
                return true;
            }
        }
        interface.log("UWB: error: UWBS hardware reset failed");
        return false;
    }

    bool set_channel(uint8_t channel) {
        if (!__check_rdy()) {
            return false;
        }
        cmd_packer = [this, &channel]() {
            return uci_cmd.core_set_config(PARAM_CHANNEL_NUMBER_ID, 1,
                                           &channel);
        };
        check_rsp = [this](const UciCtrlPacket& rsp) {
            return uci_cmd.check_core_set_config_rsp(rsp);
        };
        if (__send_packet()) {
            interface.log("UWB: set channel %d", channel);
            return true;
        }
        interface.log("UWB: error: set channel fail");
        return false;
    }

    bool get_channel(uint8_t& channel) {
        uint8_t param_id;
        uint8_t val_len;
        if (!__check_rdy()) {
            return false;
        }
        cmd_packer = [this]() {
            return uci_cmd.core_get_config(PARAM_CHANNEL_NUMBER_ID);
        };
        check_rsp = [this, &param_id, &val_len,
                     &channel](const UciCtrlPacket& rsp) {
            return uci_cmd.check_core_get_config_rsp(rsp, &param_id, &val_len,
                                                     &channel);
        };
        if (__send_packet()) {
            if (param_id != PARAM_CHANNEL_NUMBER_ID) {
                interface.log("UWB: error: get config id %d", param_id);
                return false;
            } else {
                interface.log("UWB: get channel %d", channel);
                return true;
            }
        }
        interface.log("UWB: error: get channel fail");
        return false;
    }

    bool set_prf_mode(uint8_t prf_mode) {
        if (!__check_rdy()) {
            return false;
        }
        cmd_packer = [this, &prf_mode]() {
            return uci_cmd.core_set_config(PARAM_PRF_MODE_ID, 1, &prf_mode);
        };
        check_rsp = [this](const UciCtrlPacket& rsp) {
            return uci_cmd.check_core_set_config_rsp(rsp);
        };
        if (__send_packet()) {
            interface.log("UWB: set prf mode %d", prf_mode);
            return true;
        }
        interface.log("UWB: error: set prf mode fail");
        return false;
    }

    bool get_prf_mode(uint8_t& prf_mode) {
        uint8_t param_id;
        uint8_t val_len;
        if (!__check_rdy()) {
            return false;
        }
        cmd_packer = [this]() {
            return uci_cmd.core_get_config(PARAM_PRF_MODE_ID);
        };
        check_rsp = [this, &param_id, &val_len,
                     &prf_mode](const UciCtrlPacket& rsp) {
            return uci_cmd.check_core_get_config_rsp(rsp, &param_id, &val_len,
                                                     &prf_mode);
        };
        if (__send_packet()) {
            if (param_id != PARAM_PRF_MODE_ID) {
                interface.log("UWB: error: get config id %d", param_id);
                return false;
            } else {
                interface.log("UWB: get prf mode %d", prf_mode);
                return true;
            }
        }
        interface.log("UWB: error: get prf mode fail");
        return false;
    }

    bool set_preamble_length(uint8_t preamble_length) {
        if (!__check_rdy()) {
            return false;
        }
        cmd_packer = [this, &preamble_length]() {
            return uci_cmd.core_set_config(PARAM_PREAMBLE_LENGTH_ID, 1,
                                           &preamble_length);
        };
        check_rsp = [this](const UciCtrlPacket& rsp) {
            return uci_cmd.check_core_set_config_rsp(rsp);
        };
        if (__send_packet()) {
            interface.log("UWB: set preamble length %d", preamble_length);
            return true;
        }
        interface.log("UWB: error: set preamble length fail");
        return false;
    }

    bool set_preamble_index(uint8_t preamble_index) {
        if (!__check_rdy()) {
            return false;
        }
        cmd_packer = [this, &preamble_index]() {
            return uci_cmd.core_set_config(PARAM_PREAMBLE_CODE_INDEX_ID, 1,
                                           &preamble_index);
        };
        check_rsp = [this](const UciCtrlPacket& rsp) {
            return uci_cmd.check_core_set_config_rsp(rsp);
        };
        if (__send_packet()) {
            interface.log("UWB: set preamble index %d", preamble_index);
            return true;
        }
        interface.log("UWB: error: set preamble index fail");
        return false;
    }

    bool set_psdu_data_rate(uint8_t psdu_data_rate) {
        if (!__check_rdy()) {
            return false;
        }
        cmd_packer = [this, &psdu_data_rate]() {
            return uci_cmd.core_set_config(PARAM_PSDU_DATA_RATE_ID, 1,
                                           &psdu_data_rate);
        };
        check_rsp = [this](const UciCtrlPacket& rsp) {
            return uci_cmd.check_core_set_config_rsp(rsp);
        };
        if (__send_packet()) {
            interface.log("UWB: set psdu data rate %d", psdu_data_rate);
            return true;
        }
        interface.log("UWB: error: set psdu data rate fail");
        return false;
    }

    bool set_phr_mode(uint8_t phr_mode) {
        if (!__check_rdy()) {
            return false;
        }
        cmd_packer = [this, &phr_mode]() {
            return uci_cmd.core_set_config(PARAM_PHR_MODE_ID, 1, &phr_mode);
        };
        check_rsp = [this](const UciCtrlPacket& rsp) {
            return uci_cmd.check_core_set_config_rsp(rsp);
        };
        if (__send_packet()) {
            interface.log("UWB: set phr mode %d", phr_mode);
            return true;
        }
        interface.log("UWB: error: set phr mode fail");
        return false;
    }
    bool get_phr_mode(uint8_t& phr_mode) {
        uint8_t param_id;
        uint8_t val_len;
        if (!__check_rdy()) {
            return false;
        }
        cmd_packer = [this]() {
            return uci_cmd.core_get_config(PARAM_PHR_MODE_ID);
        };
        check_rsp = [this, &param_id, &val_len,
                     &phr_mode](const UciCtrlPacket& rsp) {
            return uci_cmd.check_core_get_config_rsp(rsp, &param_id, &val_len,
                                                     &phr_mode);
        };
        if (__send_packet()) {
            if (param_id != PARAM_PHR_MODE_ID) {
                interface.log("UWB: error: get config id %d", param_id);
                return false;
            } else {
                interface.log("UWB: get phr mode %d", phr_mode);
                return true;
            }
        }
        interface.log("UWB: error: get phr mode fail");
        return false;
    }
    bool set_sfd_id(uint8_t sfd_id) {
        if (!__check_rdy()) {
            return false;
        }
        cmd_packer = [this, &sfd_id]() {
            return uci_cmd.core_set_config(PARAM_SFD_ID_ID, 1, &sfd_id);
        };
        check_rsp = [this](const UciCtrlPacket& rsp) {
            return uci_cmd.check_core_set_config_rsp(rsp);
        };
        if (__send_packet()) {
            interface.log("UWB: set sfd id %d", sfd_id);
            return true;
        }
        interface.log("UWB: error: set sfd id fail");
        return false;
    }

    bool get_sfd_id(uint8_t& sfd_id) {
        uint8_t param_id;
        uint8_t val_len;
        if (!__check_rdy()) {
            return false;
        }
        cmd_packer = [this]() {
            return uci_cmd.core_get_config(PARAM_SFD_ID_ID);
        };
        check_rsp = [this, &param_id, &val_len,
                     &sfd_id](const UciCtrlPacket& rsp) {
            return uci_cmd.check_core_get_config_rsp(rsp, &param_id, &val_len,
                                                     &sfd_id);
        };
        if (__send_packet()) {
            if (param_id != PARAM_SFD_ID_ID) {
                interface.log("UWB: error: get config id %d", param_id);
                return false;
            } else {
                interface.log("UWB: get sfd id %d", sfd_id);
                return true;
            }
        }
        interface.log("UWB: error: get sfd id fail");
        return false;
    }

    bool set_tx_power(uint8_t tx_power) {
        if (!__check_rdy()) {
            return false;
        }
        cmd_packer = [this, &tx_power]() {
            return uci_cmd.core_set_config(PARAM_TX_POWER_ID, 1, &tx_power);
        };
        check_rsp = [this](const UciCtrlPacket& rsp) {
            return uci_cmd.check_core_set_config_rsp(rsp);
        };
        if (__send_packet()) {
            interface.log("UWB: set tx power %d", tx_power);
            return true;
        }
        interface.log("UWB: error: set tx power fail");
        return false;
    }

    bool set_hprf() {
        if (!__check_rdy()) {
            return false;
        }
        cmd_packer = [this]() { return uci_cmd.cx_set_hprf(); };
        check_rsp = [this](const UciCtrlPacket& rsp) {
            return uci_cmd.check_cx_set_hprf_rsp(rsp);
        };
        if (__send_packet()) {
            interface.log("UWB: set hprf");
            return true;
        }
        interface.log("UWB: error: set hprf fail");
        return false;
    }
    bool set_nooploop() {
        if (!__check_rdy()) {
            return false;
        }
        cmd_packer = [this]() { return uci_cmd.cx_nooploop(); };
        check_rsp = [this](const UciCtrlPacket& rsp) {
            return uci_cmd.check_cx_nooploop_rsp(rsp);
        };
        if (__send_packet()) {
            interface.log("UWB: set nooploop");
            return true;
        }
        interface.log("UWB: error: set nooploop fail");
        return false;
    }

    bool get_config(uint8_t config_id, uint8_t* config_value, uint8_t length) {}

    /**
     * @brief 获取设备信息
     */
    bool get_dev_info() {
        if (uwbs_sta != READY) {
            interface.log("UWB: error: UWBS not ready");
            return false;
        }

        UciCMD::UWBDeviceInfo dev_info;
        cmd_packer = [this]() { return uci_cmd.core_get_device_info(); };
        check_rsp = [this, &dev_info](const UciCtrlPacket& rsp) {
            return uci_cmd.check_core_get_device_info_rsp(rsp, dev_info);
        };

        if (__send_packet()) {
            interface.log(
                "UWB: uci generic version = 0x%.4X, mac version = 0x%.4X, "
                "phy version = 0x%.4X , uci test version = 0x%.4X",
                dev_info.uci_ver, dev_info.mac_ver, dev_info.phy_ver,
                dev_info.uci_test_ver);

            // uint8_t vendor_len=;
            return true;
        }
        interface.log("UWB: error: get device info fail");
        return false;
    }

    /**
     * @brief 数据透传
     * @param data 发送数据
     * @return 发送成功返回true，失败返回false
     */
    bool data_transmit(const std::vector<uint8_t>& data) {
        if (!__check_rdy()) {
            return false;
        }

        if (data.size() == 0) {
            return true;
        }

        cmd_packer = [this, &data]() { return uci_cmd.cx_app_data_tx(data); };
        check_rsp = [this](const UciCtrlPacket& rsp) {
            return uci_cmd.check_cx_app_data_tx_rsp(rsp);
        };

        if (__send_packet()) {
            // interface.log("UWB: data transmit");
            return true;
        }
        interface.log("UWB: error: data transmit fail");
        return false;
    }
    bool data_transmit_tx_test(std::vector<uint8_t> data, uint16_t pack_size,
                               uint16_t pack_num) {
        if (!__check_rdy()) {
            return false;
        }

        data.reserve(pack_size);
        for (uint16_t i = 0; i < pack_size; i++) {
            data.push_back(i % 0xff);
        }
#define _CNT 2000
        int delay_cnt = _CNT;
        for (uint16_t i = 0; i < pack_num; i++) {
            data[0] = i & 0xff;
            data[1] = i >> 8;
            data_transmit(data);

            while (delay_cnt--) {
                // __NOP();
            }
            delay_cnt = _CNT;
            // interface.delay_ms(delay_cnt);
        }
        return true;
    }

    bool data_transmit_rx_test(std::vector<uint8_t> data, uint16_t pack_size,
                               uint16_t pack_num) {
        if (!__check_rdy()) {
            return false;
        }
        uint16_t loss_pack = 0;
        uint32_t time_ms = 0;
        bool start_flag = false;
        uint16_t recv_cnt = 0;
        uint16_t pack_id = 0;
        uint32_t start_tick = 0;
        while (true) {
            if (get_recv_data(data)) {
                pack_id = data[0] | (data[1] << 8);
                if (!start_flag) {
                    if (pack_id == 0) {
                        start_flag = true;
                        recv_cnt = 1;
                        start_tick = interface.get_system_1ms_ticks();
                    }
                } else {
                    if (pack_id < recv_cnt) {
                        interface.log("UWB: error: pack id = %u, recv_cnt = %u",
                                      pack_id, recv_cnt);
                        return false;
                    }
                    if (pack_id != recv_cnt) {
                        loss_pack += pack_id - recv_cnt;
                        recv_cnt = pack_id;
                    }
                    recv_cnt++;
                    // interface.log("UWB: recv recv_cnt = %d", recv_cnt);
                    if (recv_cnt == pack_num) {
                        time_ms = interface.get_system_1ms_ticks() - start_tick;
                        interface.log(
                            "UWB: data transmit rx test: transmit pack = "
                            "%u, loss pack = %u, "
                            "time = %ums, transmit data = %uB",
                            pack_num, loss_pack, time_ms, pack_num * pack_size);
                        return true;
                    }
                }
            }
        }
        return true;
    }

    /**
     * @brief 设置接收模式
     * @return 设置成功返回true，失败返回false
     */
    bool set_recv_mode() {
        if (!__check_rdy()) {
            return false;
        }

        cmd_packer = [this]() { return uci_cmd.cx_app_data_rx(); };
        check_rsp = [this](const UciCtrlPacket& rsp) {
            return uci_cmd.check_cx_app_data_rx_rsp(rsp);
        };

        if (__send_packet()) {
            interface.log("UWB: set recv mode");
            return true;
        }
        interface.log("UWB: error: set recv mode fail");
        return false;
    }

    /**
     * @brief 获取透传数据
     * @param recv_data 接收数据
     * @return 获取成功返回true，失败返回false
     */
    bool get_recv_data(std::vector<uint8_t>& recv_data) {
        update();
        if (!__check_rdy()) {
            return false;
        }
        if (transparent_data.empty()) {
            return false;
        }
        recv_data.clear();
        recv_data.reserve(transparent_data.size());
        while (transparent_data.empty() == false) {
            recv_data.push_back(transparent_data.front());
            transparent_data.pop();
        }
        return true;
    }

    /**
     * @brief 停止接收
     * @return 停止成功返回true，失败返回false
     */
    bool stop_recv() {
        if (!__check_rdy()) {
            return false;
        }
        cmd_packer = [this]() { return uci_cmd.cx_app_data_stop_rx(); };
        check_rsp = [this](const UciCtrlPacket& rsp) {
            return uci_cmd.check_cx_app_data_stop_rx_rsp(rsp);
        };
        if (__send_packet()) {
            interface.log("UWB: stop recv");
            return true;
        }
        interface.log("UWB: error: stop recv fail");
        return false;
    }

    /**
     * @brief 更新，监听通知和更新状态机
     * @return 无
     */
    void update() {
        __listening_ntf();
        __uwbs_state_machine();
    }

    bool is_init_success() const { return init_success; }

    // bool init() {
    //     __init();
    //     return init_success;
    // }

   private:
    void __delay_ms(uint32_t ms) { interface.delay_ms(ms); }
    bool __check_rdy() {
        if (uwbs_sta == READY) {
            return true;
        }
        interface.log("UWB: error: UWBS not ready");
        return false;
    }
    void __uwbs_state_machine() {
        switch (uwbs_sta) {
            case BOOT: {
                // interface.log("UWB: boot");
                break;
            }
            case READY: {
                // interface.log("UWB: ready");
                break;
            }
            case ACTIVE: {
                break;
            }
            case ERROR: {
                break;
            }
        }
    }

    void __load_recv_data() { interface.get_recv_data(rx_data_queue); }

    bool __rsp_process(uint32_t timeout_ms) {
        uint32_t start_tick = interface.get_system_1ms_ticks();
        while (interface.get_system_1ms_ticks() - start_tick < timeout_ms) {
            __load_recv_data();
            while (rx_data_queue.empty() == false) {
                _data = rx_data_queue.front();
                rx_data_queue.pop();

                if (recv_packet.flow_parse(_data)) {
                    //  Log.r(recv_packet.packet.data(),
                    //   recv_packet.packet.size());
                    if (recv_packet.mt == MT_RSP) {
                        // 接收到响应
                        return true;
                    } else if (recv_packet.mt == MT_NTF) {
                        // 接收到通知
                        __notify_process();
                    }
                }
            }
        }
        interface.log("UWB: error: wait rsp timeout");
        return false;
    }

    void __listening_ntf() {
        __load_recv_data();
        while (rx_data_queue.empty() == false) {
            _data = rx_data_queue.front();
            rx_data_queue.pop();
            if (recv_packet.flow_parse(_data)) {
                if (recv_packet.mt == MT_NTF) {
                    __notify_process();
                } else {
                    interface.log("UWB: error: unexpected rsp packet");
                }
            }
        }
    }

    void __notify_process() {
        if (recv_packet.gid == GID0x00) {
            switch (recv_packet.oid) {
                case CORE_DEVICE_STATUS_NTF: {
                    uint8_t sta = uci_ntf.parse_core_device_status_ntf(
                        recv_packet.packet);
                    if (sta == DEVICE_STATE_READY) {
                        if (uwbs_sta == BOOT) {
                            uwbs_sta = READY;
                            interface.log("UWB: UWBS move to active state");
                        }
                    }
                    break;
                }
                default: {
                    interface.log(
                        "UWB: warning: undealed notify: gid=0x00, oid=0x%.2X",
                        recv_packet.oid);
                    break;
                }
            }
        }
        if (recv_packet.gid == GID0x03) {
            switch (recv_packet.oid) {
                case CX_APP_DATA_TX_NTF: {
                    if (uci_ntf.parse_cx_app_data_tx_ntf(recv_packet.packet) !=
                        STATUS_OK) {
                        interface.log("UWB: error: parse data tx ntf fail");
                    } else {
                        // interface.log("UWB: data transmit");
                    }
                    break;
                }
                case CX_APP_DATA_RX_NTF: {
                    if (!uci_ntf.parse_cx_app_data_rx_ntf(recv_packet.packet)) {
                        interface.log("UWB: error: parse data rx ntf fail");
                    }

                    if (recv_packet.packet.size() < 2) {
                        interface.log(
                            "UWB: error: rx payload size is too small");
                        break;
                    }
                    for (auto it = recv_packet.packet.begin() + 2;
                         it != recv_packet.packet.end(); it++) {
                        transparent_data.push(*it);
                    }
                    // interface.log("UWB: data receive, size=%u",
                    //               rx_payload.size() - 2);
                    break;
                }
                default: {
                    interface.log(
                        "UWB: warning: undealed notify: gid=0x03, oid=0x%.2X",
                        recv_packet.oid);
                    break;
                }
            }
        }
    }

    void __init() {
        uint32_t start_tick;
        uwbs_sta = BOOT;
        init_success = false;
        // 芯片使能引脚初始化
        interface.chip_en_init();
        // 复位引脚初始化
        interface.reset_pin_init();
        // 使能芯片
        interface.chip_disable();
        __delay_ms(100);

        // 初始化通信端口
        interface.commuication_peripheral_init();

        interface.chip_enable();
        __delay_ms(200);

        // 产生复位信号
        // interface.generate_reset_signal();
        // __delay_ms(100);

        // // 关闭复位信号
        // interface.turn_of_reset_signal();
        // __delay_ms(2000);

        // 更新通知与状态机
        start_tick = interface.get_system_1ms_ticks();
        while (uwbs_sta != READY) {    // 检查是否就绪
            update();
            if (interface.get_system_1ms_ticks() - start_tick > 1000) {
                // 超时
                interface.log("UWB: error: init wait ready timeout");
                break;
            }
        }
        if (uwbs_sta == READY) {
            if (reset(1000)) {
                init_success = true;
                interface.log("UWB: UWBS init success");
                return;
            }
        }
        interface.log("UWB: error: UWBS init fail");

        // 复位指令
        // __delay_ms(500);
        // reset(3000);
    }
    bool __send_packet() {
        if ((cmd_packer == nullptr) || (check_rsp == nullptr)) {
            return false;
        }
        bool send_flag = true;
        bool pack_all_payload = false;
        bool ret = false;
        uint8_t index = 0;
        while (1) {
            if (send_flag) {
                send_flag = false;
                pack_all_payload = cmd_packer();
                interface.send(uci_cmd.packet);
                // Log.r(uci_cmd.packet.data(), uci_cmd.packet.size());
            }
            if (__rsp_process(UWB_GENERAL_TIMEOUT_MS)) {
                if (check_rsp(recv_packet)) {
                    send_flag = true;
                    if (pack_all_payload) {
                        ret = true;
                        break;
                    }
                } else {
                    interface.log("UWB: error: rsp check fail");
                    ret = false;
                    break;
                }
            } else {
                ret = false;
                break;
            }
        }
        uci_cmd.reset_packer();
        cmd_packer = nullptr;
        check_rsp = nullptr;
        return ret;
    }
};
#endif