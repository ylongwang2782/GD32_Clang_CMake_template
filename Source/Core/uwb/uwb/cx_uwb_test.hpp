#ifndef CX_UWB_TEST_HPP
#define CX_UWB_TEST_HPP
#include <cstdint>
#include <vector>

#include "FreeRTOScpp.h"
#include "SemaphoreCPP.h"
#include "TaskCPP.h"
#include "bsp_exti.hpp"
#include "bsp_gpio.hpp"
#include "bsp_log.hpp"
#include "bsp_spi.hpp"
// #include "bsp_uid.hpp"
#include "tag_uwb_protocol.hpp"
#include "uwb.hpp"

extern Logger Log;
extern Uart uartLog;

class SlaveUwbSpiInterface : public CxUwbInterface {
   public:
    SlaveUwbSpiInterface() {}
    ~SlaveUwbSpiInterface() {
        delete en_pin;
        delete rst_pin;
        delete spi_dev;
    }

   private:
    Spi_PeriphConfig spi_cfg = {
        .prescale = SPI_PSC_8,
        .clock_polarity_phase = SPI_CK_PL_LOW_PH_1EDGE,
        .frame_size = SPI_FRAMESIZE_8BIT,
        .endian = SPI_ENDIAN_MSB,
    };

    SpiDev<SpiMode::Master>* spi_dev;
    std::vector<NSS_IO> nss_io = {{GPIOE, GPIO_PIN_4}};

    GPIO* en_pin;
    GPIO* rst_pin;
    GPIO rdy_pin = {GPIO::Port::B, GPIO::Pin::PIN_7, GPIO::Mode::INPUT,
                    GPIO::PullUpDown::NONE};
    GPIO int_pin = {GPIO::Port::B, GPIO::Pin::PIN_6, GPIO::Mode::INPUT,
                    GPIO::PullUpDown::NONE};

    ExitCfg exit_cfg = {.gpio_periph = GPIOB,
                        .gpio_pin = GPIO_PIN_6,
                        .pull_up_down = GPIO_PUPD_NONE,
                        .exti_trig = EXTI_TRIG_FALLING,
                        .priority = 6};

    Exit<SlaveUwbSpiInterface> int_exti = {
        this, &SlaveUwbSpiInterface::int_pin_irq_handler, exit_cfg};
    uint8_t rx_buffer[1024];
    uint16_t revc_len;
    BinarySemaphore rx_semaphore = {"rx_semaphore"};
    long waswoken = 0;
    bool irq_enable = false;

    void int_pin_irq_handler() {
        if (!irq_enable) {
            return;
        }
        if (int_pin.input_bit_get() == RESET) {
            rx_semaphore.give_ISR(waswoken);
        }
    }

   public:
    void reset_pin_init() override {
        rst_pin = new GPIO(GPIO::Port::E, GPIO::Pin::PIN_3, GPIO::Mode::OUTPUT);
        rst_pin->bit_set();
    }
    void generate_reset_signal() override { rst_pin->bit_reset(); }
    void turn_of_reset_signal() override { rst_pin->bit_set(); }
    bool send(std::vector<uint8_t>& tx_data) override {
        bool ret = false;

        spi_dev->nss_low();

        uint32_t start_time = xTaskGetTickCount();
        while (rdy_pin.input_bit_get() == SET) {
            // 等待RDY引脚变为低电平
            if (xTaskGetTickCount() - start_time > 1000) {
                // 超时处理，返回错误或其他适当的操作
                spi_dev->nss_high();
                return false;
            }
        }
        ret = spi_dev->send(tx_data);
        spi_dev->nss_high();
        return ret;
    }

    bool get_recv_data(std::queue<uint8_t>& rx_data) override {
        if (rx_semaphore.take(0)) {
            spi_dev->nss_low();
            int a = 100;
            while (a--) {
                __NOP();
            }
            if (!spi_dev->recv_open_loop(rx_buffer, 4, 0)) {
                spi_dev->nss_high();
                return false;
            }
            revc_len = (((uint16_t)rx_buffer[2]) << 8) | rx_buffer[3];
            if (!spi_dev->recv_open_loop(rx_buffer + 4, revc_len, 0)) {
                spi_dev->nss_high();
                return false;
            }
            spi_dev->nss_high();

            for (int i = 0; i < revc_len + 4; i++) {
                rx_data.push(rx_buffer[i]);
            }
            memset(rx_buffer, 0, sizeof(rx_buffer));
            return true;
        }
        return false;
    }

    void commuication_peripheral_init() override {
        spi_dev = new SpiDev<SpiMode::Master>(SPI3_E6MOSI_E5MISO_E2SCLK_E11NSS,
                                              nss_io, spi_cfg);
        // std::vector<uint8_t> tx_data = {0x00, 0x01, 0x02, 0x03};
        // spi_dev->send_open_loop(tx_data);
        irq_enable = true;
    }
    void chip_en_init() override {
        en_pin = new GPIO(GPIO::Port::F, GPIO::Pin::PIN_2, GPIO::Mode::OUTPUT);
        en_pin->bit_set();
    }
    void chip_enable() override { en_pin->bit_set(); }
    void chip_disable() override { en_pin->bit_reset(); }

    uint32_t get_system_1ms_ticks() override {
        return xTaskGetTickCount() * portTICK_PERIOD_MS;
    }

    void delay_ms(uint32_t ms) override { TaskBase::delay(ms); }

    void log(const char* format, ...) override {
        va_list args;
        va_start(args, format);
        char buffer[256];
        vsnprintf(buffer, sizeof(buffer), format, args);
        va_end(args);
        Log.d("UWB", buffer);
    }
};

class UwbSpiInterface : public CxUwbInterface {
   public:
    UwbSpiInterface() {}
    ~UwbSpiInterface() {
        delete en_pin;
        delete rst_pin;
        delete spi_dev;
    }

   private:
    Spi_PeriphConfig spi_cfg = {
        .prescale = SPI_PSC_8,
        .clock_polarity_phase = SPI_CK_PL_LOW_PH_1EDGE,
        .frame_size = SPI_FRAMESIZE_8BIT,
        .endian = SPI_ENDIAN_MSB,
    };

    SpiDev<SpiMode::Master>* spi_dev;
    std::vector<NSS_IO> nss_io = {{GPIOE, GPIO_PIN_4}};

    GPIO* en_pin;
    GPIO* rst_pin;
    GPIO rdy_pin = {GPIO::Port::B, GPIO::Pin::PIN_7, GPIO::Mode::INPUT,
                    GPIO::PullUpDown::NONE};
    GPIO int_pin = {GPIO::Port::B, GPIO::Pin::PIN_6, GPIO::Mode::INPUT,
                    GPIO::PullUpDown::NONE};

    ExitCfg exit_cfg = {.gpio_periph = GPIOB,
                        .gpio_pin = GPIO_PIN_6,
                        .pull_up_down = GPIO_PUPD_NONE,
                        .exti_trig = EXTI_TRIG_FALLING,
                        .priority = 6};

    Exit<UwbSpiInterface> int_exti = {
        this, &UwbSpiInterface::int_pin_irq_handler, exit_cfg};
    uint8_t rx_buffer[1024];
    uint16_t revc_len;
    BinarySemaphore rx_semaphore = {"rx_semaphore"};
    long waswoken = 0;
    bool irq_enable = false;

    void int_pin_irq_handler() {
        if (!irq_enable) {
            return;
        }
        if (int_pin.input_bit_get() == RESET) {
            spi_dev->nss_low();
            int a = 100;
            while (a--) {
                __NOP();
            }
            if (!spi_dev->recv_open_loop(rx_buffer, 4, 0)) {
                spi_dev->nss_high();
                return;
            }
            revc_len = (((uint16_t)rx_buffer[2]) << 8) | rx_buffer[3];
            if (!spi_dev->recv_open_loop(rx_buffer + 4, revc_len, 0)) {
                spi_dev->nss_high();
                return;
            }
            spi_dev->nss_high();
            rx_semaphore.give_ISR(waswoken);
        }
    }

   public:
    void reset_pin_init() override {
        rst_pin = new GPIO(GPIO::Port::A, GPIO::Pin::PIN_3, GPIO::Mode::OUTPUT);
        rst_pin->bit_set();
    }
    void generate_reset_signal() override { rst_pin->bit_reset(); }
    void turn_of_reset_signal() override { rst_pin->bit_set(); }
    bool send(std::vector<uint8_t>& tx_data) override {
        bool ret = false;

        spi_dev->nss_low();

        uint32_t start_time = xTaskGetTickCount();
        while (rdy_pin.input_bit_get() == SET) {
            // 等待RDY引脚变为低电平
            if (xTaskGetTickCount() - start_time > 1000) {
                // 超时处理，返回错误或其他适当的操作
                spi_dev->nss_high();
                return false;
            }
        }
        // Log.r(tx_data.data(), tx_data.size());
        ret = spi_dev->send(tx_data);
        spi_dev->nss_high();
        return ret;
    }

    bool get_recv_data(std::queue<uint8_t>& rx_data) override {
        if (rx_semaphore.take(0)) {
            for (int i = 0; i < revc_len + 4; i++) {
                rx_data.push(rx_buffer[i]);
            }
            // Log.r(rx_buffer, 4 + revc_len);
            memset(rx_buffer, 0, sizeof(rx_buffer));
            return true;
        }
        return false;
    }

    void commuication_peripheral_init() override {
        spi_dev = new SpiDev<SpiMode::Master>(SPI3_E6MOSI_E5MISO_E2SCLK_E11NSS,
                                              nss_io, spi_cfg);
        // std::vector<uint8_t> tx_data = {0x00, 0x01, 0x02, 0x03};
        // spi_dev->send_open_loop(tx_data);
        irq_enable = true;
    }
    void chip_en_init() override {
        en_pin = new GPIO(GPIO::Port::A, GPIO::Pin::PIN_5, GPIO::Mode::OUTPUT);
        en_pin->bit_set();
    }
    void chip_enable() override { en_pin->bit_set(); }
    void chip_disable() override { en_pin->bit_reset(); }

    uint32_t get_system_1ms_ticks() override {
        return xTaskGetTickCount() * portTICK_PERIOD_MS;
    }

    void delay_ms(uint32_t ms) override { TaskBase::delay(ms); }

    void log(const char* format, ...) override {
        va_list args;
        va_start(args, format);
        char buffer[256];
        vsnprintf(buffer, sizeof(buffer), format, args);
        va_end(args);
        // Log.d("UWB", buffer);
    }
};

class UwbTestTask : public TaskClassS<1024> {
   public:
    UwbTestTask() : TaskClassS<1024>("UwbTestTask", TaskPrio_Mid) {}

   private:
#include <stddef.h>
#include <stdint.h>

    void task() override {
        GPIO KEY(GPIO::Port::A, GPIO::Pin::PIN_0, GPIO::Mode::INPUT,
                 GPIO::PullUpDown::NONE);

        uint8_t key_state = SET;

        UWB<UwbSpiInterface> uwb;
        uint8_t ret = 0;
        // uwb.init();
        // uwb.set_nooploop();
        // uwb.set_channel(PARAM_CHANNEL_NUMBER_5);     // channel 5
        // uwb.set_prf_mode(PARAM_PRF_NOMINAL_64_M);    // PRF mode 3
        // uwb.set_preamble_length(
        //     PARAM_PREAMBLE_LEN_HPRF_128);    // preamble length 8
        // uwb.set_preamble_index(9);           // preamble index 9
        // uwb.set_psdu_data_rate(
        //     PARAM_PSDU_DATA_RATE_6_81);                 // PSDU data rate 4 5
        // uwb.set_phr_mode(3);    // PHR mode 4
        // uwb.set_sfd_id(1);                              // SFD ID 5
        // uwb.set_tx_power(5);                            // TX power 5
        // uwb.get_sfd_id(ret);
        // uwb.get_phr_mode(ret);
        // uint64_t uid = 0x0011AABBCCDDEEFF;    // 模拟 UID

        Log.i("UWB", "UWB Test Task Started");
        for (;;) {
            if (KEY.input_bit_get() == RESET && key_state == SET) {
                key_state = RESET;
                if (uwb.init()) {
                    Log.i("UWB", "Device OK");
                } else {
                    Log.e("UWB", "Device ERROR");
                }
            }
            if (KEY.input_bit_get() == SET && key_state == RESET) {
                key_state = SET;
            }
            uwb.update();
            TaskBase::delay(2);
        }
    }
};

class UwbTransmitTask : public TaskClassS<1024> {
   public:
    UwbTransmitTask() : TaskClassS<1024>("UwbTransmitTask", TaskPrio_Mid) {}

   private:
    uint16_t crc16_ccitt(const uint8_t* data, size_t length,
                         uint16_t initial = 0xFFFF) {
        uint16_t crc = initial;
        for (size_t i = 0; i < length; ++i) {
            crc = (crc >> 8) | (crc << 8);
            crc ^= data[i];
            crc ^= (crc & 0xFF) >> 4;
            crc ^= (crc << 8) << 4;
            crc ^= ((crc & 0xFF) << 4) << 1;
        }
        return crc;
    }
    void task() override {
        UWB<UwbSpiInterface> uwb;
        GPIO PA8(GPIO::Port::A, GPIO::Pin::PIN_8, GPIO::Mode::INPUT,
                 GPIO::PullUpDown::PULLUP, GPIO::OType::PP,
                 GPIO::Speed::SPEED_50MHZ);
        GPIO KEY(GPIO::Port::A, GPIO::Pin::PIN_0, GPIO::Mode::INPUT,
                 GPIO::PullUpDown::NONE);
        uint8_t ret = 0;
        uwb.init();
        // 数传高速
        // uwb.set_hprf();
        // uwb.set_channel(PARAM_CHANNEL_NUMBER_9);     // channel 5
        // uwb.set_prf_mode(PARAM_PRF_NOMINAL_64_M);    // PRF mode 3
        // uwb.set_preamble_length(
        //     PARAM_PREAMBLE_LEN_BPRF_64);    // preamble length 8
        // uwb.set_preamble_index(25);         // preamble index 9
        // uwb.set_psdu_data_rate(
        //     PARAM_PSDU_DATA_RATE_31_2);                 // PSDU data rate 4
        // uwb.set_phr_mode(PARAM_PHYDATARATE_DRHM_HR);    // PHR mode 4
        // uwb.set_sfd_id(2);                              // SFD ID 3

        // 稳定参数
        uwb.set_hprf();
        uwb.set_channel(PARAM_CHANNEL_NUMBER_9);     // channel 5
        uwb.set_prf_mode(PARAM_PRF_NOMINAL_64_M);    // PRF mode 3
        uwb.set_preamble_length(
            PARAM_PREAMBLE_LEN_BPRF_64);    // preamble length 1
        uwb.set_preamble_index(9);          // preamble index 9
        uwb.set_psdu_data_rate(PARAM_PSDU_DATA_RATE_7_8);    // PSDU data rate 4
        uwb.set_phr_mode(PARAM_PHYDATARATE_DRHM_HR);         // PHR mode 4
        uwb.set_sfd_id(2);                                   // SFD ID 3                             // SFD ID 3

        uwb.set_recv_mode();

        Log.i("UWB", "UWB Transmit Task Started");

        std::vector<uint8_t> usr_data(1000);
        for (int i = 0; i < 1000; i++) {
            usr_data[i] = i % 256;    // 填充数据
        }
        uint32_t start_time = xTaskGetTickCount();
        uint16_t packet = 0;
        for (;;) {
            // uwb.update();
            if (PA8.input_bit_get() == RESET) {
                if (uwb.get_recv_data(usr_data)) {
                    Log.d("UWB", "Received Data.  size:%u, crc: %04X",
                          usr_data.size(),
                          crc16_ccitt(usr_data.data(), usr_data.size()));
                    start_time = xTaskGetTickCount();
                    packet++;
                } else {
                    // Log.w("UWB", "No Data Received");
                }
                if (xTaskGetTickCount() - start_time > 1000) {
                    Log.d("ManagerDataTransferTask", "recv packet = %d",
                          packet);
                    packet = 0;
                    start_time = xTaskGetTickCount();
                }
                uwb.update();
                TaskBase::delay(2);
            } else {
                if (KEY.input_bit_get() == RESET) {
                    for (uint8_t i = 0; i < 100; i++) {
                        if (uwb.data_transmit(usr_data)) {
                            Log.d(
                                "UWB", "Transmit Data.  crc: %04X",
                                crc16_ccitt(usr_data.data(), usr_data.size()));
                        }
                        uwb.update();
                        TaskBase::delay(50);
                    }
                }

                TaskBase::delay(2);
            }
        }
    }
};

class UwbCaptureTask : public TaskClassS<1024> {
   public:
    UwbCaptureTask() : TaskClassS<1024>("UwbCaptureTask", TaskPrio_Mid) {}

   private:
    void task() override {
        UWB<UwbSpiInterface> uwb;
        uwb.init();
        uwb.set_hprf();
        uwb.set_channel(PARAM_CHANNEL_NUMBER_9);     // channel 5
        uwb.set_prf_mode(PARAM_PRF_NOMINAL_64_M);    // PRF mode 3
        uwb.set_preamble_length(
            PARAM_PREAMBLE_LEN_BPRF_64);    // preamble length 1
        uwb.set_preamble_index(9);          // preamble index 9
        uwb.set_psdu_data_rate(PARAM_PSDU_DATA_RATE_7_8);    // PSDU data rate 4
        uwb.set_phr_mode(PARAM_PHYDATARATE_DRHM_HR);         // PHR mode 4
        uwb.set_sfd_id(2);                                   // SFD ID 3

        uwb.set_recv_mode();
        std::vector<uint8_t> data(1024);
        for (;;) {
            if (uwb.get_recv_data(data)) {
                // Log.d("UWB", "Received Data.  size:%u", data.size());
                // Log.r(data.data(), data.size());
                uartLog.send(data.data(), data.size());
            }
            data = uartLog.getReceivedData();
            if (data.size() > 0) {
                // Log.d("UWB", "Received Data from USART.  size:%u",
                // data.size()); Log.r(data.data(), data.size());
                // uartLog.send(data.data(), data.size());
                uwb.data_transmit(data);
            }
            uwb.update();
            TaskBase::delay(2);
            // Log.d("UWB", "Capture Task Running");
        }
    }
};

class SlaveBoardUwbTestTask : public TaskClassS<1024> {
   public:
    SlaveBoardUwbTestTask()
        : TaskClassS<1024>("UwbCaptureTask", TaskPrio_Mid) {}

   private:
    void task() override {
        UWB<SlaveUwbSpiInterface> uwb;
        uwb.init();
        uwb.set_hprf();
        uwb.set_channel(PARAM_CHANNEL_NUMBER_9);     // channel 5
        uwb.set_prf_mode(PARAM_PRF_NOMINAL_64_M);    // PRF mode 3
        uwb.set_preamble_length(
            PARAM_PREAMBLE_LEN_BPRF_64);    // preamble length 1
        uwb.set_preamble_index(9);          // preamble index 9
        uwb.set_psdu_data_rate(PARAM_PSDU_DATA_RATE_7_8);    // PSDU data rate 4
        uwb.set_phr_mode(PARAM_PHYDATARATE_DRHM_HR);         // PHR mode 4
        uwb.set_sfd_id(2);                                   // SFD ID 3

        uwb.set_recv_mode();
        std::vector<uint8_t> data(1024);
        Log.i("UWB", "Slave Board UWB Test Task Started");
        for (;;) {
            TaskBase::delay(2);
            if (uwb.get_recv_data(data)) {
                // Log.d("UWB", "Received Data.  size:%u", data.size());
                // Log.r(data.data(), data.size());
                uartLog.send(data.data(), data.size());
            }
            data = uartLog.getReceivedData();
            if (data.size() > 0) {
                Log.d("UWB", "Received Data from USART.  size:%u",
                      data.size());
                Log.r(data.data(), data.size());
                uwb.data_transmit(data);
            }
            uwb.update();
        }
    }
};

#endif