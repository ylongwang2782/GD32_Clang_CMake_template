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

#define SET_SPI2_NSS_HIGH gpio_bit_set(GPIOA, GPIO_PIN_15);
#define SET_SPI2_NSS_LOW  gpio_bit_reset(GPIOA, GPIO_PIN_15);

namespace SPIExample {
#define SPI_CRC_ENABLE 0
// constexpr bool SPI_CRC_ENABLE = true;
constexpr size_t ARRAYSIZE = 10;

// SPI2 GPIO 端口和引脚定义
constexpr uint32_t SPI2_PORT = GPIOB;
constexpr uint32_t SPI2_SCK_PIN = GPIO_PIN_3;
constexpr uint32_t SPI2_MISO_PIN = GPIO_PIN_4;
constexpr uint32_t SPI2_MOSI_PIN = GPIO_PIN_5;
constexpr uint32_t SPI2_NSS_PORT = GPIOA;
constexpr uint32_t SPI2_NSS_PIN = GPIO_PIN_15;

// SPI3 GPIO 端口和引脚定义
constexpr uint32_t SPI3_PORT = GPIOE;
constexpr uint32_t SPI3_SCK_PIN = GPIO_PIN_2;
constexpr uint32_t SPI3_MISO_PIN = GPIO_PIN_4;
constexpr uint32_t SPI3_MOSI_PIN = GPIO_PIN_5;
constexpr uint32_t SPI3_NSS_PIN = GPIO_PIN_6;

// SPI2 DMA 配置

constexpr uint32_t SPI2_TX_DMA = DMA0;
constexpr dma_channel_enum SPI2_TX_DMA_CH = DMA_CH0;
constexpr uint32_t SPI2_RX_DMA = DMA0;
constexpr dma_channel_enum SPI2_RX_DMA_CH = DMA_CH2;
constexpr dma_subperipheral_enum SPI2_DMA_SUBPERI = DMA_SUBPERI0;

// SPI3 DMA 配置
constexpr uint32_t SPI3_TX_DMA = DMA1;
constexpr dma_channel_enum SPI3_TX_DMA_CH = DMA_CH4;
constexpr uint32_t SPI3_RX_DMA = DMA1;
constexpr dma_channel_enum SPI3_RX_DMA_CH = DMA_CH3;
constexpr dma_subperipheral_enum SPI3_DMA_SUBPERI = DMA_SUBPERI5;

class SPIConfig {
   public:
    SPIConfig() {
        configureClocks();
        configureGPIO();
        dma_config();
        configureSPI();
        SET_SPI2_NSS_HIGH
        enableSPI();
        SET_SPI2_NSS_LOW
    }

    void runDma() {
        /* enable DMA channel */
        dma_channel_enable(SPI3_TX_DMA, SPI3_TX_DMA_CH);
        dma_channel_enable(SPI3_RX_DMA, SPI3_RX_DMA_CH);
        dma_channel_enable(SPI2_TX_DMA, SPI2_TX_DMA_CH);
        dma_channel_enable(SPI2_RX_DMA, SPI2_RX_DMA_CH);

        SET_SPI2_NSS_LOW;
        /* enable SPI DMA */
        spi_dma_enable(SPI3, SPI_DMA_TRANSMIT);
        spi_dma_enable(SPI3, SPI_DMA_RECEIVE);
        spi_dma_enable(SPI2, SPI_DMA_RECEIVE);
        spi_dma_enable(SPI2, SPI_DMA_TRANSMIT);

        /* wait DMA transmit complete */
        while (!dma_flag_get(SPI2_TX_DMA, SPI2_TX_DMA_CH, DMA_FLAG_FTF));
        while (!dma_flag_get(SPI2_RX_DMA, SPI2_RX_DMA_CH, DMA_FLAG_FTF));
        while (!dma_flag_get(SPI3_TX_DMA, SPI3_RX_DMA_CH, DMA_FLAG_FTF));
        while (!dma_flag_get(SPI3_RX_DMA, SPI3_TX_DMA_CH, DMA_FLAG_FTF));

        log.d("spi2 receive data:");
        for (size_t i = 0; i < ARRAYSIZE; i++) {
            // log.d("%02X ", spi3_send_array[i]);
            log.d("%02X ", spi3_receive_array[i]);
        }
    }

    void run() {
        // transferData();
        // verifyData();
        runDma();
    }

   private:
    uint8_t spi2_send_array[ARRAYSIZE] = {0xA1, 0xA2, 0xA3, 0xA4, 0xA5,
                                          0xA6, 0xA7, 0xA8, 0xA9, 0xAA};
    uint8_t spi3_send_array[ARRAYSIZE] = {0xB1, 0xB2, 0xB3, 0xB4, 0xB5,
                                          0xB6, 0xB7, 0xB8, 0xB9, 0xBA};
    uint8_t spi2_receive_array[ARRAYSIZE];
    uint8_t spi3_receive_array[ARRAYSIZE];

    void configureClocks() {
        rcu_periph_clock_enable(RCU_GPIOA);
        rcu_periph_clock_enable(RCU_GPIOB);
        rcu_periph_clock_enable(RCU_GPIOE);
        rcu_periph_clock_enable(RCU_DMA0);
        rcu_periph_clock_enable(RCU_DMA1);
        rcu_periph_clock_enable(RCU_SPI2);
        rcu_periph_clock_enable(RCU_SPI3);
    }

    void configureGPIO() {
        /* configure SPI2 GPIO */
        gpio_af_set(SPI2_PORT, GPIO_AF_6,
                    SPI2_SCK_PIN | SPI2_MISO_PIN | SPI2_MOSI_PIN);
        gpio_mode_set(SPI2_PORT, GPIO_MODE_AF, GPIO_PUPD_NONE,
                      SPI2_SCK_PIN | SPI2_MISO_PIN | SPI2_MOSI_PIN);
        gpio_output_options_set(SPI2_PORT, GPIO_OTYPE_PP, GPIO_OSPEED_50MHZ,
                                SPI2_SCK_PIN | SPI2_MISO_PIN | SPI2_MOSI_PIN);

        /* set SPI1_NSS as GPIO*/
        gpio_mode_set(SPI2_NSS_PORT, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE,
                      SPI2_NSS_PIN);
        gpio_output_options_set(SPI2_NSS_PORT, GPIO_OTYPE_PP, GPIO_OSPEED_50MHZ,
                                SPI2_NSS_PIN);

        /* configure SPI3 GPIO */
        gpio_af_set(
            SPI3_PORT, GPIO_AF_5,
            SPI3_SCK_PIN | SPI3_MISO_PIN | SPI3_MOSI_PIN | SPI3_NSS_PIN);
        gpio_mode_set(
            SPI3_PORT, GPIO_MODE_AF, GPIO_PUPD_NONE,
            SPI3_SCK_PIN | SPI3_MISO_PIN | SPI3_MOSI_PIN | SPI3_NSS_PIN);
        gpio_output_options_set(
            SPI3_PORT, GPIO_OTYPE_PP, GPIO_OSPEED_50MHZ,
            SPI3_SCK_PIN | SPI3_MISO_PIN | SPI3_MOSI_PIN | SPI3_NSS_PIN);
    }

    void configureSPI() {
        spi_parameter_struct spi_init_struct = {
            .device_mode = SPI_MASTER,
            .trans_mode = SPI_TRANSMODE_FULLDUPLEX,
            .frame_size = SPI_FRAMESIZE_8BIT,
            .nss = SPI_NSS_SOFT,
            .endian = SPI_ENDIAN_MSB,
            .clock_polarity_phase = SPI_CK_PL_LOW_PH_2EDGE,
            .prescale = SPI_PSC_32};
        spi_init(SPI2, &spi_init_struct);

        spi_init_struct.device_mode = SPI_SLAVE;
        spi_init_struct.nss = SPI_NSS_HARD;
        spi_init(SPI3, &spi_init_struct);

#if SPI_CRC_ENABLE
        spi_crc_polynomial_set(SPI2, 7);
        spi_crc_polynomial_set(SPI3, 7);
        spi_crc_on(SPI2);
        spi_crc_on(SPI3);
#endif /* enable CRC function */
    }

    void enableSPI() {
        spi_enable(SPI2);
        spi_enable(SPI3);
    }

    void dma_config(void) {
        dma_single_data_parameter_struct dma_init_struct;

        /* configure SPI2 transmit DMA */
        dma_deinit(SPI2_TX_DMA, SPI2_TX_DMA_CH);
        dma_init_struct.periph_addr = (uint32_t)&SPI_DATA(SPI2);
        dma_init_struct.memory0_addr = (uint32_t)spi2_send_array;
        dma_init_struct.direction = DMA_MEMORY_TO_PERIPH;
        dma_init_struct.periph_memory_width = DMA_PERIPH_WIDTH_8BIT;
        dma_init_struct.priority = DMA_PRIORITY_LOW;
        dma_init_struct.number = ARRAYSIZE;
        dma_init_struct.periph_inc = DMA_PERIPH_INCREASE_DISABLE;
        dma_init_struct.memory_inc = DMA_MEMORY_INCREASE_ENABLE;
        dma_init_struct.circular_mode = DMA_CIRCULAR_MODE_DISABLE;
        dma_single_data_mode_init(SPI2_TX_DMA, SPI2_TX_DMA_CH,
                                  &dma_init_struct);
        dma_channel_subperipheral_select(SPI2_TX_DMA, SPI2_TX_DMA_CH,
                                         SPI2_DMA_SUBPERI);

        /* configure SPI2 receive DMA */
        dma_deinit(SPI2_RX_DMA, SPI2_RX_DMA_CH);
        dma_init_struct.periph_addr = (uint32_t)&SPI_DATA(SPI2);
        dma_init_struct.memory0_addr = (uint32_t)spi2_receive_array;
        dma_init_struct.direction = DMA_PERIPH_TO_MEMORY;
        dma_init_struct.priority = DMA_PRIORITY_HIGH;
        dma_single_data_mode_init(SPI2_RX_DMA, SPI2_RX_DMA_CH,
                                  &dma_init_struct);
        dma_channel_subperipheral_select(SPI2_RX_DMA, SPI2_RX_DMA_CH,
                                         SPI2_DMA_SUBPERI);

        /* configure SPI3 transmit DMA */
        dma_deinit(SPI3_TX_DMA, SPI3_TX_DMA_CH);
        dma_init_struct.periph_addr = (uint32_t)&SPI_DATA(SPI3);
        dma_init_struct.memory0_addr = (uint32_t)spi3_send_array;
        dma_init_struct.direction = DMA_MEMORY_TO_PERIPH;
        dma_init_struct.priority = DMA_PRIORITY_MEDIUM;
        dma_single_data_mode_init(SPI3_TX_DMA, SPI3_TX_DMA_CH,
                                  &dma_init_struct);
        dma_channel_subperipheral_select(SPI3_TX_DMA, SPI3_TX_DMA_CH,
                                         SPI3_DMA_SUBPERI);

        /* configure SPI3 receive DMA */
        dma_deinit(SPI3_RX_DMA, SPI3_RX_DMA_CH);
        dma_init_struct.periph_addr = (uint32_t)&SPI_DATA(SPI3);
        dma_init_struct.memory0_addr = (uint32_t)spi3_receive_array;
        dma_init_struct.direction = DMA_PERIPH_TO_MEMORY;
        dma_init_struct.priority = DMA_PRIORITY_ULTRA_HIGH;
        dma_single_data_mode_init(SPI3_RX_DMA, SPI3_RX_DMA_CH,
                                  &dma_init_struct);
        dma_channel_subperipheral_select(SPI3_RX_DMA, SPI3_RX_DMA_CH,
                                         SPI3_DMA_SUBPERI);
    }

    void transferData() {
        uint32_t send_n = 0, receive_n = 0;

        // clear spi3 receive array
        for (size_t i = 0; i < ARRAYSIZE; i++) {
            spi3_receive_array[i] = 0;
        }

        while (send_n < ARRAYSIZE) {
            // while (RESET == spi_i2s_flag_get(SPI3, SPI_FLAG_TBE));
            // spi_i2s_data_transmit(SPI3, spi3_send_array[send_n]);
            while (RESET == spi_i2s_flag_get(SPI2, SPI_FLAG_TBE));
            spi_i2s_data_transmit(SPI2, spi2_send_array[send_n++]);
            while (RESET == spi_i2s_flag_get(SPI3, SPI_FLAG_RBNE));
            spi3_receive_array[receive_n++] = spi_i2s_data_receive(SPI3);
            // while (RESET == spi_i2s_flag_get(SPI2, SPI_FLAG_RBNE));
            // spi2_receive_array[receive_n++] = spi_i2s_data_receive(SPI2);
            // #if SPI_CRC_ENABLE
            //             if ((ARRAYSIZE - 1) == receive_n) {
            //                 /* send the CRC value */
            //                 spi_crc_next(SPI2);
            //                 spi_crc_next(SPI3);
            //             }
            // #endif /* enable CRC function */
        }
        // log spi2 receive data
        log.d("spi2 receive data:");
        for (size_t i = 0; i < ARRAYSIZE; i++) {
            // log.d("%02X ", spi3_send_array[i]);
            log.d("%02X ", spi3_receive_array[i]);
        }
    }

    bool memoryCompare(const uint8_t *src, const uint8_t *dst,
                       size_t length) const {
        while (length--) {
            if (*src++ != *dst++) {
                return false;
            }
        }
        return true;
    }

    void verifyData() {
#if SPI_CRC_ENABLE
        /* receive the CRC value */
        while (RESET == spi_i2s_flag_get(SPI2, SPI_FLAG_RBNE)) {
        }
        while (RESET == spi_i2s_flag_get(SPI3, SPI_FLAG_RBNE)) {
        }
        SET_SPI2_NSS_HIGH
        /* check the CRC error status  */
        if (SET != spi_i2s_flag_get(SPI2, SPI_FLAG_CRCERR)) {
            log.d("spi1 crc error");
        } else {
            log.d("spi1 crc ok");
        }

        if (SET != spi_i2s_flag_get(SPI3, SPI_FLAG_CRCERR)) {
            log.d("spi3 crc error");
        } else {
            log.d("spi3 crc ok");
        }
#else
        SET_SPI2_NSS_HIGH
        /* compare receive data with send data */
        if (memoryCompare(spi3_receive_array, spi2_send_array, ARRAYSIZE)) {
            log.d("spi1 send ok");
        } else {
            log.d("spi1 send error");
        }

        if (memoryCompare(spi2_receive_array, spi3_send_array, ARRAYSIZE)) {
            log.d("spi3 send ok");
        } else {
            log.d("spi3 send error");
        }
#endif /* enable CRC function */
    }
};
}    // namespace SPIExample

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

class SpiTask : public TaskClassS<1024> {
   public:
    SpiTask() : TaskClassS<1024>("SpiTask", TaskPrio_Mid) {}

    void task() override {
        SPIExample::SPIConfig spiConfig;
        for (;;) {
            spiConfig.run();
            //  log.d("Spi test.");
            TaskBase::delay(1000);
        }
    }
};

// class PinTestTask : public TaskClassS<1024> {
//    public:
//     PinTestTask() : TaskClassS<1024>("LedBlinkTask", TaskPrio_Mid) {}

//     void task() override {
//         PinTest pinTest;
//         pinTest.init();

//         for (;;) {
//             log.d("Pin test.");
//             // 62 Pins Function Test
//             for (auto &pin : pinTest.pins) {
//                 pin.toggle();
//             }
//             TaskBase::delay(500);
//         }
//     }
// };

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
SpiTask spiTask;

int main(void) {
    nvic_priority_group_set(NVIC_PRIGROUP_PRE4_SUB0);
    vTaskStartScheduler();
    for (;;);
}
