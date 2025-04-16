#include <array>
#include <cstdint>
#include <cstdio>

#include "TaskCPP.h"
#include "TaskCpp.h"
#include "bsp_adc.hpp"
#include "bsp_led.hpp"
#include "bsp_log.hpp"
#include "bsp_spi.hpp"
#include "bsp_uid.hpp"
#include "tag_uwb_protocol.hpp"
#include "uci.hpp"

#ifdef __cplusplus
extern "C" {
#endif
#include "FreeRTOS.h"
#include "task.h"

#ifdef __cplusplus
}
#endif

UartConfig usart0Conf(usart0_info);
// UartConfig usart1Conf(usart1_info);
// UartConfig usart2Conf(usart2_info);
UartConfig uart3Conf(uart3_info);
UartConfig uart4Conf(uart4_info);
Uart usart0(usart0Conf);
// Uart usart1(usart1Conf);
// Uart usart2(usart2Conf);
Uart uart3(uart3Conf);
Uart uart4(uart4Conf);

Logger Log(uart3);

class UwbTask : public TaskClassS<1024> {
   public:
    UwbTask() : TaskClassS<1024>("UwbTask", TaskPrio_Mid) {}

    void task() override {
        // Initialize with tag UID (8 bytes)
        std::vector<uint8_t> tag_uid = {0x07, 0x06, 0x05, 0x04,
                                        0x03, 0x02, 0x01, 0x00};
        UWBPacketBuilder builder(tag_uid);

        Uci uci(usart0);
        // uci.mode_set(RX_MODE);
        // uci.mode_set(STANDBY_MODE);

        std::vector<uint8_t> blinkFrame;

        for (;;) {
            // uwb packet test
            blinkFrame = builder.buildTagBlinkFrame();
            uci.data_send(blinkFrame);
            TaskBase::delay(500);
        }
    }
};

class LedBlinkTask : public TaskClassS<1024> {
   public:
    LedBlinkTask() : TaskClassS<1024>("LedBlinkTask", TaskPrio_Mid) {}

    void task() override {
        LED led0(GPIOC, GPIO_PIN_6);

        for (;;) {
            Log.d("LedBlinkTask");
            led0.toggle();
            TaskBase::delay(500);
        }
    }
};

class UsartDMATask : public TaskClassS<1024> {
   public:
    UsartDMATask() : TaskClassS<1024>("UsartDMATask", TaskPrio_Mid) {}

    void task() override {
        std::vector<uint8_t> rx_data;
        for (;;) {
            // 等待 DMA 完成信号
            if (xSemaphoreTake(usart0_info.dmaRxDoneSema, portMAX_DELAY) ==
                pdPASS) {
                rx_data = usart0.getReceivedData();
                if (rx_data.size() > 0) {
                    usart0.send(rx_data.data(), rx_data.size());
                }
            }
        }
    }
};

class SpiTask : public TaskClassS<1024> {
   public:
    SpiTask() : TaskClassS<1024>("SpiTask", TaskPrio_Mid) {}

    static void spiCallback(void *arg) {
        SpiDev<SpiMode::Slave> *dev = (SpiDev<SpiMode::Slave> *)arg;
        if (!dev->rx_queue.empty_ISR()) {
            uint8_t rxData;
            long worken;
            dev->rx_queue.pop_ISR(rxData, worken);
            dev->send(&rxData, 1);
        }
    }

    void task() override {
        // Master
        std::vector<NSS_IO> nss_pins = {
            {GPIOB, GPIO_PIN_13}    // NSS引脚在PB12
        };

        SpiDev<SpiMode::Master> spiMaster(SPI1_C1MOSI_C2MISO_B10SCLK_B12NSS,
                                          nss_pins);

        // Slave
        SpiDev<SpiMode::Slave> spiSlave(SPI3_E6MOSI_E5MISO_E2SCLK_E11NSS);
        // spiSlave.enable();

        std::vector<uint8_t> spiSlaveData = {0x01, 0x02, 0x03, 0x04, 0x05,
                                             0x06, 0x07, 0x08, 0x09};
        // spi master recv buffer
        std::vector<uint8_t> spiMasterData = {0x01, 0x02, 0x03, 0x04,
                                              0x05, 0x06, 0x07, 0x08};
        std::vector<uint8_t> spiRecvData;

        for (;;) {
            spiSlave.send(spiSlaveData, 0);    // 等待发送完成

            if (!spiMaster.recv(spiSlaveData.size())) {
                //    if(!spiMaster.send_recv(spiMasterData,spiSlaveData.size()))
                //    {
                Log.d("spiMaster send failed.");

            } else {
                Log.d("spiMaster send success.");
                for (auto data : spiMaster.rx_buffer) {
                    Log.d("spiMaster recv data: %d", data);
                }
            }

            while (!spiSlave.rx_queue.empty()) {
                uint8_t data;
                spiSlave.rx_queue.pop(data);
                Log.d("spiSlave recv data: %d", data);
            }
            // spiRecvData = spiMaster.rx_buffer;

            TaskBase::delay(500);
        }
    }
};

class LogTask : public TaskClassS<1024> {
   public:
    LogTask() : TaskClassS<1024>("LogTask", TaskPrio_Mid) {}

    void task() override {
        char buffer[LOG_QUEUE_SIZE + 8];
        for (;;) {
            LogMessage logMsg;
            // 从队列中获取日志消息
            if (Log.logQueue.pop(logMsg, portMAX_DELAY)) {
                Log.uart.send(
                    reinterpret_cast<const uint8_t *>(logMsg.message.data()),
                    strlen(logMsg.message.data()));
            }
        }
    }
};

UwbTask uwbTask;
LedBlinkTask ledBlinkTask;
UsartDMATask usartDMATask;
LogTask logTask;
// SpiTask spiTask;

int main(void) {
    nvic_priority_group_set(NVIC_PRIGROUP_PRE4_SUB0);
    vTaskStartScheduler();
    for (;;);
}
