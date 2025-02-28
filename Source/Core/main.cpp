#include <array>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <vector>

#include "TaskCPP.h"
#include "TaskCpp.h"
#include "bsp_led.hpp"
#include "bsp_log.hpp"
#include "bsp_uid.hpp"

#ifdef __cplusplus
extern "C" {
#endif
#include "FreeRTOS.h"
#include "task.h"

#ifdef __cplusplus
}
#endif

UartConfig usart1Conf(usart1_info);
UartConfig usart2Conf(usart2_info);
UartConfig uart3Conf(uart3_info);
Uart usart1(usart1Conf);
Uart usart2(usart2Conf);
Uart uart3(uart3Conf);

Logger log(usart1);

class PinTest {
   public:
    struct PinConfig {
        GPIO::Port port;
        GPIO::Pin pin;
    };

    std::vector<GPIO> pins;

    const std::array<PinConfig, 62> pinConfigs = {{
        {GPIO::Port::A, GPIO::Pin::PIN_3},  {GPIO::Port::A, GPIO::Pin::PIN_4},
        {GPIO::Port::A, GPIO::Pin::PIN_5},  {GPIO::Port::A, GPIO::Pin::PIN_6},
        {GPIO::Port::A, GPIO::Pin::PIN_7},  {GPIO::Port::C, GPIO::Pin::PIN_4},
        {GPIO::Port::C, GPIO::Pin::PIN_5},  {GPIO::Port::B, GPIO::Pin::PIN_0},
        {GPIO::Port::B, GPIO::Pin::PIN_1},  {GPIO::Port::F, GPIO::Pin::PIN_11},
        {GPIO::Port::F, GPIO::Pin::PIN_12}, {GPIO::Port::F, GPIO::Pin::PIN_13},
        {GPIO::Port::F, GPIO::Pin::PIN_14}, {GPIO::Port::F, GPIO::Pin::PIN_15},
        {GPIO::Port::G, GPIO::Pin::PIN_0},  {GPIO::Port::G, GPIO::Pin::PIN_1},
        {GPIO::Port::E, GPIO::Pin::PIN_7},  {GPIO::Port::E, GPIO::Pin::PIN_8},
        {GPIO::Port::E, GPIO::Pin::PIN_9},  {GPIO::Port::E, GPIO::Pin::PIN_10},
        {GPIO::Port::E, GPIO::Pin::PIN_11}, {GPIO::Port::E, GPIO::Pin::PIN_12},
        {GPIO::Port::E, GPIO::Pin::PIN_13}, {GPIO::Port::E, GPIO::Pin::PIN_14},
        {GPIO::Port::E, GPIO::Pin::PIN_15}, {GPIO::Port::B, GPIO::Pin::PIN_10},
        {GPIO::Port::B, GPIO::Pin::PIN_11}, {GPIO::Port::B, GPIO::Pin::PIN_12},
        {GPIO::Port::B, GPIO::Pin::PIN_13}, {GPIO::Port::B, GPIO::Pin::PIN_14},
        {GPIO::Port::B, GPIO::Pin::PIN_15}, {GPIO::Port::D, GPIO::Pin::PIN_8},
        {GPIO::Port::D, GPIO::Pin::PIN_9},  {GPIO::Port::D, GPIO::Pin::PIN_10},
        {GPIO::Port::D, GPIO::Pin::PIN_11}, {GPIO::Port::D, GPIO::Pin::PIN_12},
        {GPIO::Port::D, GPIO::Pin::PIN_13}, {GPIO::Port::D, GPIO::Pin::PIN_14},
        {GPIO::Port::D, GPIO::Pin::PIN_15}, {GPIO::Port::G, GPIO::Pin::PIN_2},
        {GPIO::Port::G, GPIO::Pin::PIN_3},  {GPIO::Port::G, GPIO::Pin::PIN_4},
        {GPIO::Port::G, GPIO::Pin::PIN_5},  {GPIO::Port::G, GPIO::Pin::PIN_6},
        {GPIO::Port::G, GPIO::Pin::PIN_7},  {GPIO::Port::G, GPIO::Pin::PIN_8},
        {GPIO::Port::C, GPIO::Pin::PIN_6},  {GPIO::Port::C, GPIO::Pin::PIN_7},
        {GPIO::Port::C, GPIO::Pin::PIN_8},  {GPIO::Port::C, GPIO::Pin::PIN_9},
        {GPIO::Port::A, GPIO::Pin::PIN_8},  {GPIO::Port::A, GPIO::Pin::PIN_9},
        {GPIO::Port::A, GPIO::Pin::PIN_10}, {GPIO::Port::A, GPIO::Pin::PIN_11},
        {GPIO::Port::A, GPIO::Pin::PIN_12}, {GPIO::Port::A, GPIO::Pin::PIN_15},
        {GPIO::Port::C, GPIO::Pin::PIN_10}, {GPIO::Port::C, GPIO::Pin::PIN_11},
        {GPIO::Port::D, GPIO::Pin::PIN_0},  {GPIO::Port::D, GPIO::Pin::PIN_1},
        {GPIO::Port::D, GPIO::Pin::PIN_3},  {GPIO::Port::D, GPIO::Pin::PIN_4},
    }};

    PinTest() {}

    void init() {
        for (const auto &config : pinConfigs) {
            pins.emplace_back(config.port, config.pin, GPIO::Mode::OUTPUT);
        }
    }
};

class PinTestTask : public TaskClassS<1024> {
   public:
    PinTestTask() : TaskClassS<1024>("LedBlinkTask", TaskPrio_Mid) {}

    void task() override {
        PinTest pinTest;
        pinTest.init();

        for (;;) {
            log.d("Pin test.");
            // 62 Pins Function Test
            for (auto &pin : pinTest.pins) {
                pin.toggle();
            }
            TaskBase::delay(500);
        }
    }
};

class LedBlinkTask : public TaskClassS<1024> {
   public:
    LedBlinkTask() : TaskClassS<1024>("LedBlinkTask", TaskPrio_Mid) {}

    void task() override {
        LED led0(GPIO::Port::C, GPIO::Pin::PIN_13);

        for (;;) {
            led0.toggle();
            log.d("Led toggle.");
            TaskBase::delay(500);
        }
    }
};

class UsartDMATask : public TaskClassS<1024> {
   public:
    UsartDMATask() : TaskClassS<1024>("UsartDMATask", TaskPrio_Mid) {}

    void task() override {
        for (;;) {
            // 等待 DMA 完成信号
            if (xSemaphoreTake(usart1_info.dmaRxDoneSema, portMAX_DELAY) ==
                pdPASS) {
                log.d("Usart recv.");
                uint8_t buffer[DMA_RX_BUFFER_SIZE];
                uint16_t len =
                    usart1.getReceivedData(buffer, DMA_RX_BUFFER_SIZE);
                if (len > 0) {
                    // dma tx the data recv
                    usart1.send(buffer, len);
                }
            }
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
            if (log.logQueue.pop(logMsg, portMAX_DELAY)) {
                log.uart.send(
                    reinterpret_cast<const uint8_t *>(logMsg.message.data()),
                    strlen(logMsg.message.data()));
            }
        }
    }
};

// PinTestTask pinTestTask;
LedBlinkTask ledBlinkTask;
UsartDMATask usartDMATask;
LogTask logTask;

int main(void) {
    nvic_priority_group_set(NVIC_PRIGROUP_PRE4_SUB0);
    vTaskStartScheduler();
    for (;;);
}
