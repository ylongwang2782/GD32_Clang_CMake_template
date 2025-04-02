#include <array>
#include <cstdint>
#include <vector>

#include "bsp_log.hpp"
#include "bsp_uart.hpp"

#define RX_MODE      0x01 /* 接收模式 */
#define STANDBY_MODE 0x02 /* 待机模式 */

#define UCI_OK  0x00 /* 成功 */
#define UCI_ERR 0x01 /* 失败 */

class Uci {
   private:
    std::vector<uint8_t> current_packet_;

   public:
    Uci(Uart& uart) : uart(uart) {}
    Uart& uart;    // 串口对象的引用

    void data_send(const std::vector<uint8_t>& data) {
        uint8_t header = 0x23;    // 0b0010 (Control Packet) << 4 | 0b0011 (UWB
                                  // Data Session Control Group)
        uint8_t oid = 0x00;       // TX enable OID
        uint16_t size = data.size();
        // Serialize UCI
        current_packet_.clear();
        current_packet_.push_back(header);
        current_packet_.push_back(oid);
        // 修改为大端模式：先存高位字节，再存低位字节
        current_packet_.push_back((size >> 8) & 0xFF);
        current_packet_.push_back(size & 0xFF);
        current_packet_.insert(current_packet_.end(), data.begin(), data.end());

        // 发送UCI
        send(current_packet_);
    }

    // after send data to uci uart, check reply
    bool send_reply_check(const std::vector<uint8_t>& data, uint8_t mode) {
        std::vector<uint8_t> sendReply;

        switch (mode) {
            case RX_MODE:
                sendReply = {0x43, 0x01, 0x00, 0x01, 0x00};
                break;
            case STANDBY_MODE:
                sendReply = {0x43, 0x02, 0x00, 0x01, 0x00};
                break;
            default:
                sendReply = {0x43, 0x00, 0x00, 0x01, 0x00,
                             0x63, 0x00, 0x00, 0x01, 0x00};
        }

        if (data.size() != sendReply.size()) {
            return false;
        }
        return true;
    }

    int mode_set(uint8_t mode) {
        std::vector<uint8_t> rxModeData = {0x23, 0x01, 0x00, 0x00};
        std::vector<uint8_t> standByModeData = {0x23, 0x02, 0x00, 0x00};
        std::vector<uint8_t> response;

        switch (mode) {
            case RX_MODE:
                send(rxModeData);
                response = uart.getReceivedData();
                if (!send_reply_check(response, RX_MODE)) {
                    Log.e("Uci: RX_MODE config failed.");
                    return UCI_ERR;
                }
                return UCI_OK;

            case STANDBY_MODE:
                send(standByModeData);
                response = uart.getReceivedData();
                if (!send_reply_check(response, STANDBY_MODE)) {
                    Log.e("Uci: STANDBY_MODE config failed.");
                    return UCI_ERR;
                }
                return UCI_OK;

            default:
                return UCI_ERR;
        }
    }

   private:
    void send(std::vector<uint8_t>& data) {
        uart.data_send(const_cast<uint8_t*>(data.data()), data.size());
    }
};