#include <cstdint>
#include <iomanip>
#include <iostream>
#include <vector>

#include "bsp_log.hpp"

extern Uart usart0;

class UWBPacketBuilder {
   public:
    // Constructor with tag UID (8 bytes)
    UWBPacketBuilder(const std::vector<uint8_t>& tag_uid) : tag_uid_(tag_uid) {
        if (tag_uid.size() != 8) {
            // throw std::invalid_argument("Tag UID must be 8 bytes");
        }
        tx_counter_ = 0;
    }

    // Build uplink transmission packet from user data
    std::vector<uint8_t> buildTag2BackendPacket(
        const std::vector<uint8_t>& user_data) {
        // Reset state for new packet
        current_packet_.clear();
        tx_counter_++;

        // Build each layer of the protocol stack
        buildTag2BackendPayload(user_data);
        buildTag2BackendMsg();
        buildTag2BackendSharedData();
        buildTag2BackendPacket();
        buildFrame();
        buildUCI();

        return current_packet_;
    }

    std::vector<uint8_t> sendTagBlinkPacket() {
        // Reset state for new packet
        current_packet_.clear();
        tx_counter_++;

        // Build each layer of the protocol stack
        buildTagBlinkMsgPayload();
        buildTagBlinkMsg();
        buildTagBlinkSharedData();
        buildTagBlinkPacket();
        buildFrame();
        buildUCI();

        return current_packet_;
    }

    // Get the current transmission count
    uint32_t getTransmissionCount() const { return tx_counter_; }

    // Helper function to print hex data
    static void printHex(const std::vector<uint8_t>& data) {
        for (const auto& byte : data) {
            std::cout << std::hex << std::setw(2) << std::setfill('0')
                      << static_cast<int>(byte) << " ";
        }
        std::cout << std::dec << "\n";
    }

   private:
    std::vector<uint8_t> tag_uid_;
    uint32_t tx_counter_;
    std::vector<uint8_t> current_packet_;

    // Calculate checksum for Frame header (sof + payload_size)
    static uint8_t calculateHeaderChecksum(uint8_t sof, uint16_t payload_size) {
        return (sof + (payload_size & 0xFF) + (payload_size >> 8)) & 0xFF;
    }

    // Calculate checksum for entire Frame (sum of all bytes except checksum
    // itself)
    static uint16_t calculateFrameChecksum(
        const std::vector<uint8_t>& frame_data) {
        uint16_t sum = 0;
        for (const auto& byte : frame_data) {
            sum += byte;
        }
        return sum & 0xFFFF;
    }

    // 3.1.1. MSG Payload组包
    void buildTag2BackendPayload(const std::vector<uint8_t>& user_data) {
        struct {
            uint8_t tx_cnt;
            struct {
                uint32_t reserved : 8;         // 保留位
                uint32_t type : 10;            // 消息类型
                uint32_t reserved2 : 2;        // 保留位
                uint32_t payload_size : 12;    // payload 数据长度
            } type_info;
        } payload_header;

        payload_header.tx_cnt = tx_counter_;
        payload_header.type_info.reserved = 0;
        payload_header.type_info.type = 0;    // uplink user transparent frame
        payload_header.type_info.reserved2 = 0;
        payload_header.type_info.payload_size = user_data.size();

        uint32_t type_info_value =
            (payload_header.type_info.payload_size & 0xFFF) |    // 低 12 位
            ((payload_header.type_info.reserved2 & 0x3) << 12) |    // 12-13 位
            ((payload_header.type_info.type & 0x3FF) << 14) |       // 14-23 位
            ((payload_header.type_info.reserved & 0xFF) << 24);     // 高 8 位

        current_packet_.clear();
        current_packet_.push_back(payload_header.tx_cnt);
        for (int i = 3; i >= 0; --i) {
            current_packet_.push_back(
                static_cast<uint8_t>((type_info_value >> (i * 8)) & 0xFF));
        }
        current_packet_.insert(current_packet_.end(), user_data.begin(),
                               user_data.end());

        // usart0.data_send(current_packet_.data(), current_packet_.size());
    }

    void buildTagBlinkMsgPayload() {
        struct {
            uint32_t cnt;
            uint8_t is_moving;
        } payload_header;

        payload_header.cnt = tx_counter_;
        payload_header.is_moving = 1;

        current_packet_.clear();
        for (int i = 3; i >= 0; --i) {
            current_packet_.push_back(
                static_cast<uint8_t>((payload_header.cnt >> (i * 8)) & 0xFF));
        }
        current_packet_.push_back(payload_header.is_moving);

        // usart0.data_send(current_packet_.data(), current_packet_.size());
    }

    // 3.1.2. MSG组包
    void buildTag2BackendMsg() {
        std::vector<uint8_t> msg_payload = current_packet_;
        uint16_t msg_id_size = (43 << 10) | (msg_payload.size() &
                                             0x3FF);    // MSG_ID_TAG2USER = 43

        // Serialize MSG
        current_packet_.clear();
        // 修改为大端模式：先存高位字节，再存低位字节
        current_packet_.push_back((msg_id_size >> 8) & 0xFF);    // 高位字节
        current_packet_.push_back(msg_id_size & 0xFF);           // 低位字节
        current_packet_.insert(current_packet_.end(), msg_payload.begin(),
                               msg_payload.end());
        // usart0.data_send(current_packet_.data(), current_packet_.size());
    }

    void buildTagBlinkMsg() {
        std::vector<uint8_t> msg_payload = current_packet_;
        // 固定MSG ID格式: 0x4801 (8 << 10 | 1)
        uint16_t msg_id_size =
            0x4801;    // MSG_ID_TAG_BLINK = 8, payload_size=5

        // Serialize MSG
        current_packet_.clear();
        // 大端模式：先存高位字节(0x48)，再存低位字节(0x01)
        current_packet_.push_back(0x48);    // 高位字节
        current_packet_.push_back(0x01);    // 低位字节
        current_packet_.insert(current_packet_.end(), msg_payload.begin(),
                               msg_payload.end());
        // usart0.data_send(current_packet_.data(), current_packet_.size());
    }

    // 3.1.3. Shared Data组包
    void buildTag2BackendSharedData() {
        std::vector<uint8_t> msg_data = current_packet_;

        // Serialize Shared Data
        current_packet_ = tag_uid_;
        // 修改为大端模式：从最高字节开始存储
        for (int i = 3; i >= 0; --i) {
            current_packet_.push_back((tx_counter_ >> (i * 8)) & 0xFF);
        }
        // Append the MSG data
        current_packet_.insert(current_packet_.end(), msg_data.begin(),
                               msg_data.end());
    }

    void buildTagBlinkSharedData() {
        std::vector<uint8_t> msg_data = current_packet_;

        // Serialize Shared Data
        current_packet_ = tag_uid_;
        // Append the MSG data
        current_packet_.insert(current_packet_.end(), msg_data.begin(),
                               msg_data.end());

        // usart0.data_send(current_packet_.data(), current_packet_.size());
    }

    // 3.1.4. Packet组包
    void buildTag2BackendPacket() {
        std::vector<uint8_t> shared_data_msg = current_packet_;

        // The first 12 bytes are Shared Data (8 byte UID + 4 byte counter)
        size_t shared_size = 12;
        // The rest is MSG data
        size_t msg_size = shared_data_msg.size() - shared_size;

        uint8_t packet_id = 2;    // PACKET_ID_TAG2BACKEND = 2
        uint16_t size_info = ((shared_size & 0x3F) << 10) | (msg_size & 0x3FF);

        // Serialize Packet
        current_packet_.clear();
        current_packet_.push_back(packet_id);
        // 修改为大端模式：先存高位字节，再存低位字节
        current_packet_.push_back((size_info >> 8) & 0xFF);
        current_packet_.push_back(size_info & 0xFF);
        current_packet_.insert(current_packet_.end(), shared_data_msg.begin(),
                               shared_data_msg.end());
    }

    void buildTagBlinkPacket() {
        std::vector<uint8_t> shared_data_msg = current_packet_;

        // The first 8 bytes are Shared Data (8 byte UID)
        size_t shared_size = 8;
        // The rest is MSG data
        size_t msg_size = shared_data_msg.size() - shared_size;

        uint8_t packet_id = 0x06;       // PACKET_ID_TAG2UWB = 0x06
        uint16_t size_info = 0xC801;    // 固定值 C8 01

        // Serialize Packet
        current_packet_.clear();
        current_packet_.push_back(packet_id);
        // 大端模式：先存高位字节，再存低位字节
        current_packet_.push_back((size_info >> 8) & 0xFF);    // 0xC8
        current_packet_.push_back(size_info & 0xFF);           // 0x01
        current_packet_.insert(current_packet_.end(), shared_data_msg.begin(),
                               shared_data_msg.end());
    }

    // 3.1.5. Frame组包
    void buildFrame() {
        std::vector<uint8_t> packet_data = current_packet_;

        uint8_t sof = 0xAA;
        uint16_t payload_size = packet_data.size();
        uint8_t header_checksum = calculateHeaderChecksum(sof, payload_size);

        // Build frame data before checksum
        current_packet_.clear();
        current_packet_.push_back(sof);
        // 小端模式：先存高位字节，再存低位字节
        current_packet_.push_back((payload_size) & 0xFF);
        current_packet_.push_back(payload_size >> 8 & 0xFF);
        current_packet_.push_back(header_checksum);
        current_packet_.insert(current_packet_.end(), packet_data.begin(),
                               packet_data.end());

        // Calculate and append checksum
        uint16_t checksum = calculateFrameChecksum(current_packet_);
        // 小端模式：先存高位字节，再存低位字节
        current_packet_.push_back((checksum) & 0xFF);
        current_packet_.push_back(checksum >> 8 & 0xFF);
    }

    // 3.1.6. UCI组包
    void buildUCI() {
        std::vector<uint8_t> frame_data = current_packet_;

        uint8_t header = 0x23;    // 0b0010 (Control Packet) << 4 | 0b0011 (UWB
                                  // Data Session Control Group)
        uint8_t oid = 0x00;       // TX enable OID
        uint16_t size = frame_data.size();

        // Serialize UCI
        current_packet_.clear();
        current_packet_.push_back(header);
        current_packet_.push_back(oid);
        // 修改为大端模式：先存高位字节，再存低位字节
        current_packet_.push_back((size >> 8) & 0xFF);
        current_packet_.push_back(size & 0xFF);
        current_packet_.insert(current_packet_.end(), frame_data.begin(),
                               frame_data.end());
        usart0.data_send(current_packet_.data(), current_packet_.size());
    }
};
