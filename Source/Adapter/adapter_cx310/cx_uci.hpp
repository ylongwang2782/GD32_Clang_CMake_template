#ifndef CX_UCI_HPP_
#define CX_UCI_HPP_
#include <algorithm>    // 添加这个头文件以使用std::fill
#include <cstdint>
#include <cstring>
#include <vector>

#include "cx_uci_def.hpp"

class UciCtrlPcketBase {
   public:
    UciCtrlPcketBase() = default;
    virtual ~UciCtrlPcketBase() = default;

   public:
    uint8_t mt;                     // Message Type
    uint8_t gid;                    // Group ID
    uint8_t oid;                    // Object ID
    std::vector<uint8_t> packet;    // Packet
};

class UciCtrlPacket : public UciCtrlPcketBase {
   public:
    UciCtrlPacket() = default;
    ~UciCtrlPacket() = default;

   public:
    bool sending = false;    // 发送标志位

   private:
    enum ParserSta : uint8_t {
        PARSE_START = 0,
        MT_PBF_GID_BYTE,
        OID_BYTE,
        RFU_BYTE,
        PAYLOAD_LEN_BYTE,
        PAYLOAD_BYTES,
    };
    uint8_t pbf;                        // Packet Boundary Flag
    uint16_t current_packet_len = 0;    // uci数据包长度
    uint16_t payload_offset = 0;
    uint16_t currunt_payload_len = 0;
    bool is_last_packet = true;
    ParserSta parser_sta = PARSE_START;
    uint16_t pkt_recv_len = 0;
    bool recv_packet = false;

   public:
    void reset() {
        sending = false;
        current_packet_len = 0;
        payload_offset = 0;
        currunt_payload_len = 0;
        is_last_packet = true;
        parser_sta = PARSE_START;
        pkt_recv_len = 0;
        recv_packet = false;
    }
    bool build_packet(const std::vector<uint8_t>& total_payload) {
        size_t residual_len =
            total_payload.size() - payload_offset;    // 剩余待发送的字节长度
        sending = true;

        if (residual_len > MAX_PAYLOAD_LEN) {
            currunt_payload_len = MAX_PAYLOAD_LEN;
            pbf = PBF_SEGMENT;         // Packet Boundary Flag
            is_last_packet = false;    // 不是最后一个数据包

        } else {
            currunt_payload_len = residual_len;
            pbf = PBF_COMPLETE;       // Packet Boundary Flag
            is_last_packet = true;    // 是最后一个数据包
        }

        current_packet_len =
            currunt_payload_len + UCI_CTRL_PKT_HDR_SIZE;    // 加上头4个字节

        // 清空output数组，长度为current_packet_len
        packet.resize(current_packet_len);

        // 构建uci control packet header
        __build_header(packet);

        // 复制payload到output，从output的第4个字节开始复制，长度为currunt_payload_len
        if (currunt_payload_len > 0) {
            std::copy(
                total_payload.begin() + payload_offset,
                total_payload.begin() + payload_offset + currunt_payload_len,
                packet.begin() + UCI_CTRL_PKT_HDR_SIZE);
        }

        // 更新payload_offset，以便下一次调用时可以正确地复制payload
        payload_offset += currunt_payload_len;

        // 如果是最后一个数据包，重置payload_offset
        if (is_last_packet) {
            payload_offset = 0;
            sending = false;
        }

        return is_last_packet;
    }

    bool flow_parse(uint8_t& data) {
        switch (parser_sta) {
            case PARSE_START: {
                recv_packet = false;
                packet.clear();
                parser_sta = MT_PBF_GID_BYTE;
            }
            case MT_PBF_GID_BYTE: {
                mt = (data >> 5) & 0x07;
                pbf = (data >> 4) & 0x01;
                gid = data & 0x0F;
                if ((mt != MT_CMD) && (mt != MT_RSP) && (mt != MT_NTF)) {
                    parser_sta = PARSE_START;
                    break;
                }
                if (pbf == PBF_COMPLETE) {
                    is_last_packet = true;
                } else {
                    is_last_packet = false;
                }
                parser_sta = OID_BYTE;
                break;
            }
            case OID_BYTE: {
                oid = data & 0x3F;
                currunt_payload_len = 0;
                parser_sta = RFU_BYTE;
                break;
            }
            case RFU_BYTE: {
                currunt_payload_len |= ((uint16_t)data << 8);
                parser_sta = PAYLOAD_LEN_BYTE;
                break;
            }
            case PAYLOAD_LEN_BYTE: {
                currunt_payload_len |= data;
                pkt_recv_len = 0;
                if (currunt_payload_len > 0) {
                    parser_sta = PAYLOAD_BYTES;
                } else {
                    recv_packet = true;
                    parser_sta = PARSE_START;
                }
                break;
            }
            case PAYLOAD_BYTES: {
                packet.push_back(data);
                pkt_recv_len++;
                if (pkt_recv_len == currunt_payload_len) {
                    recv_packet = true;
                    if (is_last_packet) {
                        // 是最后一个数据包，重置状态
                        parser_sta = PARSE_START;
                    } else {
                        // 不是最后一个数据包，等待下一个数据包的开始
                        parser_sta = MT_PBF_GID_BYTE;
                    }
                }
                break;
            }
        }
        return (is_last_packet & recv_packet);
    }

   private:
    void __build_header(std::vector<uint8_t>& output) {
        output[0] = (mt & 0x07) << 5;
        output[0] |= ((pbf & 0x01) << 4);
        output[0] |= (gid & 0x0F);
        output[1] = oid & 0x3F;
        output[2] = currunt_payload_len >> 8;
        output[3] = currunt_payload_len & 0xFF;
    }

    void __parse_header(std::vector<uint8_t>& input) {
        mt = input[0] >> 5;
        pbf = (input[0] >> 4) & 0x01;
        gid = input[0] & 0x0F;
        oid = input[1] & 0x3F;
        currunt_payload_len = input[3];

        if (pbf == PBF_COMPLETE) {
            is_last_packet = true;
        } else {
            is_last_packet = false;
        }
    }
};

class UciCMD : private UciCtrlPacket {
   public:
    UciCMD() : packet(UciCtrlPacket::packet) {}

#pragma pack(1)
    typedef struct {
        uint8_t status;
        uint16_t uci_ver;
        uint16_t mac_ver;
        uint16_t phy_ver;
        uint16_t uci_test_ver;
        uint8_t vendor_info_len;
    } UWBDeviceInfo;

    // typedef struct {
    //     uint8_t type_id;
    //     uint8_t len;
    //     uint8_t value[2];
    // } UWBParameter;
#pragma pack()

   public:
    std::vector<uint8_t>& packet;

   private:
    std::vector<uint8_t> payload;

   public:
    uint16_t payload_len() { return payload.size(); }
    void reset_packer() { reset(); }

    /* -----------------<Device Reset CMD>----------------- */
    bool core_device_reset() {
        payload.resize(1);
        payload.clear();
        mt = MT_CMD;
        gid = GID0x00;
        oid = CORE_DEVICE_RESET_CMD;
        payload.push_back(0x00);
        return build_packet(payload);
    }

    bool check_core_device_reset_rsp(const UciCtrlPacket& rsp) {
        if (rsp.mt != MT_RSP) {
            return false;
        }
        if (rsp.gid != GID0x00) {
            return false;
        }
        if (rsp.oid != CORE_DEVICE_RESET_CMD) {
            return false;
        }
        if (rsp.packet[0] != STATUS_OK) {
            return false;
        }
        return true;
    }

    /* ---------------<Get Device Info CMD>--------------- */
    bool core_get_device_info() {
        payload.resize(0);
        payload.clear();
        mt = MT_CMD;
        gid = GID0x00;
        oid = CORE_GET_DEVICE_INFO_CMD;
        return build_packet(payload);
    }

    bool check_core_get_device_info_rsp(const UciCtrlPacket& rsp,
                                        UWBDeviceInfo& info) {
        if (rsp.mt != MT_RSP) {
            return false;
        }
        if (rsp.gid != GID0x00) {
            return false;
        }
        if (rsp.oid != CORE_GET_DEVICE_INFO_CMD) {
            return false;
        }
        memcpy(&info, rsp.packet.data(), sizeof(UWBDeviceInfo));
        if (info.status != STATUS_OK) {
            return false;
        }
        return true;
    }

    /* ---------------<Get Caps Info CMD>---------------- */
    bool core_get_caps_info() {
        payload.resize(0);
        payload.clear();
        mt = MT_CMD;
        gid = GID0x00;
        oid = CORE_GET_CAPS_INFO_CMD;
        return build_packet(payload);
    }

    /* -----------------<Set Config CMD>----------------- */
    // bool core_set_config(uint8_t param_id, uint8_t val_len) {
    //     payload.resize(2);
    //     payload.clear();
    //     mt = MT_CMD;
    //     gid = GID0x00;
    //     oid = CORE_SET_CONFIG_CMD;
    //     payload[0] = param_id;
    //     payload[1] = val_len;    // length
    //     return build_packet(payload);
    // }

    bool core_set_config(uint8_t param_id, uint8_t val_len,
                         uint8_t* param_val) {
        payload.clear();
        payload.resize(3);
        mt = MT_CMD;
        gid = GID0x03;
        oid = CX_SET_CONFIG_CMD;
        payload[0] = 0x01;    // 0x01表示设置参数
        payload[1] = param_id;
        payload[2] = val_len;    // length
        payload.insert(payload.end(), param_val, param_val + val_len);
        return build_packet(payload);
    }

    bool check_core_set_config_rsp(const UciCtrlPacket& rsp) {
        if (rsp.mt != MT_RSP) {
            return false;
        }
        if (rsp.gid != GID0x03) {
            return false;
        }
        if (rsp.oid != CX_SET_CONFIG_CMD) {
            return false;
        }
        if (rsp.packet[0] != STATUS_OK) {
            return false;
        }
        return true;
    }

    /* -----------------<Get Config CMD>----------------- */
    bool core_get_config(uint8_t param_id) {
        payload.clear();
        payload.resize(2);
        mt = MT_CMD;
        gid = GID0x03;
        oid = CX_GET_CONFIG_CMD;
        payload[0] = 0x01;    // 获取一个设备
        payload[1] = param_id;
        return build_packet(payload);
    }

    bool check_core_get_config_rsp(const UciCtrlPacket& rsp, uint8_t* param_id,
                                   uint8_t* val_len, uint8_t* param_val) {
        if (rsp.mt != MT_RSP) {
            return false;
        }
        if (rsp.gid != GID0x03) {
            return false;
        }
        if (rsp.oid != CX_GET_CONFIG_CMD) {
            return false;
        }
        if (rsp.packet[0] != STATUS_OK) {
            return false;
        }
        if (rsp.packet[1] != 0x01) {
            return false;
        }
        *param_id = rsp.packet[2];
        *val_len = rsp.packet[3];
        memcpy(param_val, rsp.packet.data() + 4, *val_len);
        return true;
    }

    /* ---------------<Data Timestamp CMD>--------------- */
    bool cx_app_data_tx(const std::vector<uint8_t>& data) {
        if (data.size() > CX_APP_DATA_TX_MAX_PAYLOAD_LEN) {
            return false;
        }
        payload.resize(data.size());
        payload.assign(data.begin(), data.end());
        mt = MT_CMD;
        gid = GID0x03;
        oid = CX_APP_DATA_TX_CMD;
        return build_packet(payload);
    }

    bool check_cx_app_data_tx_rsp(const UciCtrlPacket& rsp) {
        if (rsp.mt != MT_RSP) {
            return false;
        }
        if (rsp.gid != GID0x03) {
            return false;
        }
        if (rsp.oid != CX_APP_DATA_TX_CMD) {
            return false;
        }
        if (rsp.packet[0] != STATUS_OK) {
            return false;
        }
        return true;
    }

    /* ---------------<Data Receive CMD>---------------- */
    bool cx_app_data_rx() {
        payload.resize(0);
        payload.clear();
        mt = MT_CMD;
        gid = GID0x03;
        oid = CX_APP_DATA_RX_CMD;
        return build_packet(payload);
    }

    bool check_cx_app_data_rx_rsp(const UciCtrlPacket& rsp) {
        if (rsp.mt != MT_RSP) {
            return false;
        }
        if (rsp.gid != GID0x03) {
            return false;
        }
        if (rsp.oid != CX_APP_DATA_RX_CMD) {
            return false;
        }
        if (rsp.packet[0] != STATUS_OK) {
            return false;
        }
        return true;
    }

    /* ---------------<Data Stop Receive CMD>---------------- */
    bool cx_app_data_stop_rx() {
        payload.resize(0);
        payload.clear();
        mt = MT_CMD;
        gid = GID0x03;
        oid = CX_APP_DATA_STOP_RX_CMD;
        return build_packet(payload);
    }
    bool check_cx_app_data_stop_rx_rsp(const UciCtrlPacket& rsp) {
        if (rsp.mt != MT_RSP) {
            return false;
        }
        if (rsp.gid != GID0x03) {
            return false;
        }
        if (rsp.oid != CX_APP_DATA_STOP_RX_CMD) {
            return false;
        }
        if (rsp.packet[0] != STATUS_OK) {
            return false;
        }
        return true;
    }

    bool cx_set_hprf() {
        payload.clear();
        mt = MT_CMD;
        gid = GID0x03;
        oid = 0x03;
        payload.resize(7);
        payload = {0x02, 0xB5, 0x01, 0x03, 0x14, 0x01, 0x19};
        return build_packet(payload);
    }

    bool check_cx_set_hprf_rsp(const UciCtrlPacket& rsp) { return true; }

    bool cx_nooploop() {
        payload.clear();
        mt = MT_CMD;
        gid = GID0x03;
        oid = 0x03;    // 假设0x03是CX_NOOPLOOP_CMD的OID
        payload.resize(25);
        payload = {
            0x08, 0x04, 0x01, 0x05, 0x1F, 0x01, 0x03, 0xB3, 0x01,
            0x01, 0x14, 0x01, 0x09, 0x16, 0x01, 0x02, 0xB5, 0x01,
            0x01, 0x15, 0x01, 0x00, 0xB2, 0x01, 0x05};    // 假设没有额外的参数
        return build_packet(payload);
    }

    bool check_cx_nooploop_rsp(const UciCtrlPacket& rsp) { return true; }
};

class UciNTF : private UciCtrlPacket {
   public:
    UciNTF() = default;

    uint8_t parse_core_device_status_ntf(std::vector<uint8_t>& payload) {
        return payload[0];
    }

    uint8_t parse_cx_app_data_tx_ntf(std::vector<uint8_t>& payload) {
        return payload[0];
    }
    bool parse_cx_app_data_rx_ntf(std::vector<uint8_t>& payload) {
        uint16_t data_len = payload[0] | (payload[1] << 8);
        if (data_len != payload.size() - 2) {
            return false;
        }
        return true;
    }
};

#endif    // UCI_DEF_HPP_