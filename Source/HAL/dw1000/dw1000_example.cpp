/**
 * @file dw1000_example.cpp
 * @brief DW1000 UWB模块使用示例
 * 
 * 本示例展示如何使用DW1000类进行超宽带通信
 * 包括初始化、发送数据、接收数据的基本流程
 */

#include "dw1000.hpp"
#include "TaskCPP.h"
#include "Logger.h"
#include <vector>

// 简单的发送测试
void dw1000_tx_example() {
    DW1000 dw1000;
    
    // 初始化DW1000
    if (!dw1000.init()) {
        Log::e("DW1000_TX", "Failed to initialize DW1000");
        return;
    }
    
    // 准备要发送的数据
    std::vector<uint8_t> tx_data = {'H', 'e', 'l', 'l', 'o', ' ', 'U', 'W', 'B'};
    
    while (true) {
        // 发送数据
        if (dw1000.data_transmit(tx_data)) {
            Log::i("DW1000_TX", "Data sent successfully");
        } else {
            Log::e("DW1000_TX", "Failed to send data");
        }
        
        // 等待1秒后再次发送
        TaskBase::delay(1000);
        
        // 增加序列号（模拟不同的数据）
        if (tx_data.size() > 0) {
            tx_data[tx_data.size() - 1]++;
        }
    }
}

// 简单的接收测试
void dw1000_rx_example() {
    DW1000 dw1000;
    
    // 初始化DW1000
    if (!dw1000.init()) {
        Log::e("DW1000_RX", "Failed to initialize DW1000");
        return;
    }
    
    // 启动接收模式
    if (!dw1000.set_recv_mode()) {
        Log::e("DW1000_RX", "Failed to set receive mode");
        return;
    }
    
    Log::i("DW1000_RX", "Receiver started, waiting for data...");
    
    std::vector<uint8_t> rx_data;
    
    while (true) {
        // 检查是否有数据接收
        if (dw1000.get_recv_data(rx_data)) {
            Log::i("DW1000_RX", "Received %d bytes", rx_data.size());
            
            // 打印接收到的数据（作为字符串）
            if (rx_data.size() > 0) {
                std::string received_str(rx_data.begin(), rx_data.end());
                Log::i("DW1000_RX", "Data: %s", received_str.c_str());
            }
        }
        
        // 定期更新状态
        dw1000.update();
        
        // 短暂延时
        TaskBase::delay(10);
    }
}

// 双向通信示例
void dw1000_duplex_example() {
    DW1000 dw1000;
    
    if (!dw1000.init()) {
        Log::e("DW1000_DUPLEX", "Failed to initialize DW1000");
        return;
    }
    
    std::vector<uint8_t> tx_data = {'R', 'E', 'S', 'P', 'O', 'N', 'S', 'E'};
    std::vector<uint8_t> rx_data;
    uint32_t last_tx_time = 0;
    
    // 首先启动接收模式
    dw1000.set_recv_mode();
    
    while (true) {
        // 检查接收
        if (dw1000.get_recv_data(rx_data)) {
            Log::i("DW1000_DUPLEX", "Received message, sending response");
            
            // 收到数据后发送响应
            if (dw1000.data_transmit(tx_data)) {
                Log::i("DW1000_DUPLEX", "Response sent");
            }
            
            // 重新启动接收
            dw1000.set_recv_mode();
        }
        
        // 定期发送心跳（每5秒）
        uint32_t current_time = TaskBase::getTickCount();
        if (current_time - last_tx_time > 5000) {
            std::vector<uint8_t> heartbeat = {'H', 'E', 'A', 'R', 'T', 'B', 'E', 'A', 'T'};
            if (dw1000.data_transmit(heartbeat)) {
                Log::i("DW1000_DUPLEX", "Heartbeat sent");
            }
            dw1000.set_recv_mode();  // 发送后重新启动接收
            last_tx_time = current_time;
        }
        
        dw1000.update();
        TaskBase::delay(10);
    }
} 