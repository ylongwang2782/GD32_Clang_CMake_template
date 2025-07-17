#pragma once
#include <cstdint>
#include <cstdio>
#include <vector>

#include "../../../HAL/exti/hal_exti.hpp"
#include "../../../HAL/gpio/hal_gpio.hpp"
#include "../../../HAL/spi/hal_spi.hpp"
#include "hal_uart.hpp"
#include "CX310.hpp"
#include "FreeRTOScpp.h"
#include "ICX310.hpp"
#include "Logger.h"
#include "SemaphoreCPP.h"
#include "TaskCPP.h"
#include "hal_hptimer.hpp"

class CX310_SlaveSpiAdapter : public ICX310 {
   public:
    CX310_SlaveSpiAdapter() {}
    ~CX310_SlaveSpiAdapter() {
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

    Exit<CX310_SlaveSpiAdapter> int_exti = {
        this, &CX310_SlaveSpiAdapter::int_pin_irq_handler, exit_cfg};
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
                
            }
            revc_len = (((uint16_t)rx_buffer[2]) << 8) | rx_buffer[3];
            if (!spi_dev->recv_open_loop(rx_buffer + 4, revc_len, 0)) {
                spi_dev->nss_high();
            }
            spi_dev->nss_high();

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

        uint32_t start_time = get_system_1ms_ticks();
        while (rdy_pin.input_bit_get() == SET) {
            // 等待RDY引脚变为低电平
            if (get_system_1ms_ticks() - start_time > 1000) {
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
        return hal_hptimer_get_ms();
    }

    void delay_ms(uint32_t ms) override { TaskBase::delay(ms); }

    void log(const char* format, ...) override {
        va_list args;
        va_start(args, format);
        char buffer[256];
        vsnprintf(buffer, sizeof(buffer), format, args);
        va_end(args);
        Log::d("UWB", buffer);
    }
};