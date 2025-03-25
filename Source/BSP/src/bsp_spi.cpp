#include "bsp_spi.hpp"

#include <cstdint>
uint8_t SpiDevBase::__bsp_is_init = 0;
SpiDevBase* SpiDevBase::__dev[(uint8_t)SpiEnum::SpiNum];
const Spi_IOConfig SPI1_C1MOSI_C2MISO_B10SCLK_B12NSS = {
    .spi_periph = SPI1,
    .spi_periph_clock = RCU_SPI1,

    .mosi_port = GPIOC,
    .mosi_pin = GPIO_PIN_1,
    .mosi_func_num = GPIO_AF_7,

    .miso_port = GPIOC,
    .miso_pin = GPIO_PIN_2,
    .miso_func_num = GPIO_AF_5,

    .sclk_port = GPIOB,
    .sclk_pin = GPIO_PIN_10,
    .sclk_func_num = GPIO_AF_5,

    .nss_port = GPIOB,
    .nss_pin = GPIO_PIN_12,
    .nss_func_num = GPIO_AF_5,
};

const Spi_IOConfig SPI1_C1MOSI_C2MISO_C7SCLK_B12NSS = {
    .spi_periph = SPI1,
    .spi_periph_clock = RCU_SPI1,

    .mosi_port = GPIOC,
    .mosi_pin = GPIO_PIN_1,
    .mosi_func_num = GPIO_AF_7,

    .miso_port = GPIOC,
    .miso_pin = GPIO_PIN_2,
    .miso_func_num = GPIO_AF_5,

    .sclk_port = GPIOC,
    .sclk_pin = GPIO_PIN_7,
    .sclk_func_num = GPIO_AF_5,

    .nss_port = GPIOB,
    .nss_pin = GPIO_PIN_12,
    .nss_func_num = GPIO_AF_5,
};

const Spi_IOConfig SPI2_B5MOSI_B4MISO_B3SCLK_A4NSS = {
    .spi_periph = SPI2,
    .spi_periph_clock = RCU_SPI2,
    .mosi_port = GPIOB,
    .mosi_pin = GPIO_PIN_5,
    .mosi_func_num = GPIO_AF_6,

    .miso_port = GPIOB,
    .miso_pin = GPIO_PIN_4,
    .miso_func_num = GPIO_AF_6,

    .sclk_port = GPIOB,
    .sclk_pin = GPIO_PIN_3,
    .sclk_func_num = GPIO_AF_6,

    .nss_port = GPIOA,
    .nss_pin = GPIO_PIN_4,
    .nss_func_num = GPIO_AF_6,
};

const Spi_IOConfig SPI3_E5MOSI_E6MISO_E2SCLK_E11NSS = {
    .spi_periph = SPI3,
    .spi_periph_clock = RCU_SPI3,

    .mosi_port = GPIOE,
    .mosi_pin = GPIO_PIN_5,
    .mosi_func_num = GPIO_AF_5,

    .miso_port = GPIOE,
    .miso_pin = GPIO_PIN_6,
    .miso_func_num = GPIO_AF_5,

    .sclk_port = GPIOE,
    .sclk_pin = GPIO_PIN_2,
    .sclk_func_num = GPIO_AF_5,

    .nss_port = GPIOE,
    .nss_pin = GPIO_PIN_11,
    .nss_func_num = GPIO_AF_5,
};

const Spi_PeriphConfig SPI_CFG1 = {
    .prescale = SPI_PSC_32,
    .clock_polarity_phase = SPI_CK_PL_LOW_PH_1EDGE,
    .frame_size = SPI_FRAMESIZE_8BIT,
    .endian = SPI_ENDIAN_MSB,
};
extern "C" {
void SPI1_IRQHandler(void) {
    if (RESET != spi_i2s_flag_get(SPI1, SPI_FLAG_RBNE)) {
        SpiDevBase::__dev[(uint8_t)SpiDevBase::SpiEnum::Spi1]
            ->__rx_isr_callback(spi_i2s_data_receive(SPI1));
    }
}
void SPI2_IRQHandler(void) {
    if (RESET != spi_i2s_flag_get(SPI2, SPI_FLAG_RBNE)) {
        SpiDevBase::__dev[(uint8_t)SpiDevBase::SpiEnum::Spi2]
            ->__rx_isr_callback(spi_i2s_data_receive(SPI2));
    }
}
void SPI3_IRQHandler(void) {
    if (RESET != spi_i2s_flag_get(SPI3, SPI_FLAG_RBNE)) {
        SpiDevBase::__dev[(uint8_t)SpiDevBase::SpiEnum::Spi3]
            ->__rx_isr_callback(spi_i2s_data_receive(SPI3));
    }
}
}
