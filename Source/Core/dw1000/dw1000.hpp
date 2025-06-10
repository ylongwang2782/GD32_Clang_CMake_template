#ifndef DW1000_HPP
#define DW1000_HPP

#include <stdint.h>

#include <vector>

#include "FreeRTOScpp.h"
#include "TaskCPP.h"
#include "bsp_log.hpp"
#include "bsp_spi.hpp"
#include "deca_device_api.h"
#include "deca_regs.h"
#include "gd32f4xx.h"
#include "gd32f4xx_usart.h"

extern Logger Log;
// #define DW_DEBUG usart_data_transmit(UART3, 'A');

#define DW1000_RX_MAX_LEN 1000
#define DW1000_SPI        SPI3
#define DW_DBG            usart_data_transmit(UART3, 'A');

class DW1000 {
   public:
    DW1000() { Log.d("BOOT", "DW1000 instance created"); }
    ~DW1000() {}
    bool init() {
        // dw1000_spi = DW1000_SPI;
        // ��ʼ����λ����
        gpio_mode_set(GPIOE, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE,
                      GPIO_PIN_3);    // RST
        gpio_output_options_set(GPIOE, GPIO_OTYPE_PP, GPIO_OSPEED_50MHZ,
                                GPIO_PIN_3);
        __spi3_init();         // ��ʼ��SPI3
        __hardware_reset();    // ��λDW1000

        // spi_dev = new
        // SpiDev<SpiMode::Master>(SPI3_E6MOSI_E5MISO_E2SCLK_E11NSS,
        //                                       nss_io, spi_cfg);
        static dwt_config_t config = {
            5,            /* Channel number. */
            DWT_PRF_64M,  /* Pulse repetition frequency. */
            DWT_PLEN_128, /* Preamble length. Used in TX only. */
            DWT_PAC8,   /* Preamble acquisition chunk size. Used in RX only. */
            9,          /* TX preamble code. Used in TX only. */
            9,          /* RX preamble code. Used in RX only. */
            1,          /* 0 to use standard SFD, 1 to use non-standard SFD. */
            DWT_BR_6M8, /* Data rate. */
            DWT_PHRMODE_STD, /* PHY header mode. */
            129 /* SFD timeout (preamble length + 1 + SFD length - PAC size).
                   Used in RX only. */
        };
        if (dwt_initialise(DWT_LOADUCODE) == DWT_ERROR) {
            // Log.v("BOOT", "dw1000 initialization");
            // TaskBase::delay(10);
            return false;    // dw1000 init failed
        }

        __port_set_dw1000_fastrate_spi3();    // spi freq: 1.875MHz ->15MHz
        // spi_dev->set_speed(SPI_PSC_4);
        dwt_configure(&config);

        dwt_setlnapamode(DWT_PA_ENABLE);
        dwt_setfinegraintxseq(0);
        dwt_setgpiodirection(DWT_GxM5, DWT_GxP5);
        dwt_setgpiodirection(DWT_GxM6, DWT_GxP6);

        is_init = true;
        return is_init;
    }

    bool data_transmit(std::vector<uint8_t>& data) {
        if (data.size() == 0) return false;
        data.push_back(0x00);    // ���CRCУ��λ
        data.push_back(0x00);

        // ��������
        int ret = __dw1000_send_frame((uint8_t*)data.data(), data.size());
        if (ret != 0) {
            return false;    // ����ʧ��
        }
        return true;    // ���ͳɹ�
    }

    bool get_recv_data(std::vector<uint8_t>& rx_data) {
        if (rx_len > 0) {
            rx_data.resize(rx_len);    // �����������ݳ���
            std::copy(rx_buffer, rx_buffer + rx_len, rx_data.begin());
            rx_len = 0;    // ���ý��ճ���
            return true;    // �ɹ���ȡ��������
        }
        return false;    // û�н��յ�����
    }

    void update() {
        if (is_init) {
            __load_recv_data();    // ���ؽ�������
        }
    }
    bool set_recv_mode() {
        // ���ý���ģʽ
        // PA RX mode
        dwt_setgpiovalue(DWT_GxM5, 0);
        dwt_setgpiovalue(DWT_GxM6, DWT_GxP6);

        // ������ʼ����
        int ret = dwt_rxenable(DWT_START_RX_IMMEDIATE);
        if (ret != 0) {
            return false;    // ����ģʽ����ʧ��
        }
        return true;    // ����ģʽ���óɹ�
    }

    bool stop_recv() {
        // ֹͣ����
        dwt_forcetrxoff();
        return true;
    }

   private:
    enum RecvStatus : uint8_t {
        __Idle = 0,     // ����״̬
        __Receiving,    // ������״̬
        __Received,     // �ѽ���״̬
    };
    RecvStatus recv_status = RecvStatus::__Idle;
    uint8_t rx_buffer[DW1000_RX_MAX_LEN];    // ���ջ�����
    uint16_t rx_len = 0;                     // �������ݳ���
    bool is_init = false;                    // ��ʼ����־

    // SpiDev<SpiMode::Master>* spi_dev;
    // std::vector<NSS_IO> nss_io = {{GPIOE, GPIO_PIN_4}};
    // Spi_PeriphConfig spi_cfg = {
    //     .prescale = SPI_PSC_32,
    //     .clock_polarity_phase = SPI_CK_PL_LOW_PH_1EDGE,
    //     .frame_size = SPI_FRAMESIZE_8BIT,
    //     .endian = SPI_ENDIAN_MSB,
    // };

    /**
     * @brief ����һ֡����
     *
     * @param[out] buffer �������ݻ�����ָ��
     * @param[out] length ���յ���ʵ�����ݳ���ָ��
     * @param[in] max_frame_len ������յ����֡����
     *
     * @retval 0 ���ճɹ�
     * @retval -1 ���������Ч��buffer��lengthΪ�գ�
     * @retval -2 �����ݽ���
     * @retval -3 ���ȳ��ޣ����ճ��� > max_frame_len��
     * @retval -4 ���ճ�ʱ
     * @retval -5 ���մ���
     * @retval -6 δ֪״̬
     *
     * @note ����ǰ��������״̬��־λ�����պ��Զ�����lengthֵ
     */
    int __dw1000_receive_frame(uint8_t* buffer, uint16_t* length,
                               uint32_t status) {
        if (buffer == NULL || length == NULL) return -1;

        // ����ر�־λ
        // dwt_write32bitreg(SYS_STATUS_ID, SYS_STATUS_ALL_RX_GOOD |
        //                                      SYS_STATUS_ALL_RX_ERR |
        //                                      SYS_STATUS_ALL_RX_TO);

        if (status & SYS_STATUS_RXFCG) {
            // ��ȡ��ǰ֡����
            uint16_t frame_len =
                dwt_read32bitreg(RX_FINFO_ID) & RX_FINFO_RXFL_MASK_1023;
            if (frame_len > DW1000_RX_MAX_LEN) {
                dwt_write32bitreg(SYS_STATUS_ID, SYS_STATUS_RXFCG);
                return -3;    // ���ȳ���
            }

            dwt_readrxdata(buffer, frame_len, 0);
            dwt_write32bitreg(SYS_STATUS_ID, SYS_STATUS_RXFCG);
            *length = frame_len;

            return 0;    // ���ճɹ�
        } else if (status & SYS_STATUS_ALL_RX_TO) {
            dwt_write32bitreg(SYS_STATUS_ID, SYS_STATUS_ALL_RX_TO);
            return -4;    // ��ʱ
        } else if (status & SYS_STATUS_ALL_RX_ERR) {
            dwt_write32bitreg(SYS_STATUS_ID, SYS_STATUS_ALL_RX_ERR);
            return -5;    // ���մ���
        }
        return -6;    // δ֪����
    }

    void __load_recv_data() {
        switch (recv_status) {
            case RecvStatus::__Idle: {
                // ����ر�־λ
                dwt_write32bitreg(SYS_STATUS_ID, SYS_STATUS_ALL_RX_GOOD |
                                                     SYS_STATUS_ALL_RX_ERR |
                                                     SYS_STATUS_ALL_RX_TO);
                if(rx_len > 0) {
                    // �����δ��������ݣ�ֱ�ӽ������״̬
                    Log.w("DW1000", "Previous data not processed, entering receiving state.");
                }
                recv_status = RecvStatus::__Receiving;
                break;
            }
            case RecvStatus::__Receiving: {
                uint32_t status = dwt_read32bitreg(SYS_STATUS_ID);
                if (!(status & (SYS_STATUS_RXFCG | SYS_STATUS_ALL_RX_ERR |
                                SYS_STATUS_ALL_RX_TO))) {
                } else {
                    // ����״̬�����Զ�ȡ����
                    uint16_t length = 0;
                    int ret =
                        __dw1000_receive_frame(rx_buffer, &length, status);
                    if (ret == 0) {
                        // ���ճɹ�������״̬

                        rx_len = length - 2;    // ���½��ճ���
                    } else {
                        // ����ʧ�ܣ�����״̬
                        rx_len = 0;
                        // Log.e("DW1000", "Receive error: %d", ret);
                    }
                    recv_status = RecvStatus::__Idle;
                }
                break;
            }

            case RecvStatus::__Received: {
                // �ѽ���״̬����������
                break;
            }
        }
    }

    /**
     * @brief ����һ֡����
     *
     * @param[in] tx_data ����������ָ��
     * @param[in] length ���������ݳ���
     *
     * @retval 0 ���ͳɹ�
     * @retval -1 ���������Ч��tx_dataΪ�ջ�lengthΪ0��
     *
     * @note ����ǰ�����������ɱ�־�����ͺ��Զ���λ״̬
     */
    int __dw1000_send_frame(uint8_t* tx_data, uint16_t length) {
        if (length == 0 || tx_data == NULL) return -1;

        // ���������ɱ�־
        dwt_write32bitreg(SYS_STATUS_ID, SYS_STATUS_TXFRS);

        // д�뷢�����ݲ����ÿ��ƼĴ���
        dwt_writetxdata(length, tx_data, 0);
        dwt_writetxfctrl(length, 0, 0);

        // PA TX mode
        dwt_setgpiovalue(DWT_GxM5, DWT_GxP5);
        dwt_setgpiovalue(DWT_GxM6, 0);

        dwt_starttx(DWT_START_TX_IMMEDIATE);

        // �ȴ��������
        while (!(dwt_read32bitreg(SYS_STATUS_ID) & SYS_STATUS_TXFRS)) {
        }
        DW_DBG

        // ���������ɱ�־
        dwt_write32bitreg(SYS_STATUS_ID, SYS_STATUS_TXFRS);

        return 0;    // �ɹ�
    }

    void __spi3_init() {
        dw1000_spi = SPI3;
        dw1000_spi_nss_pin = GPIO_PIN_4;    // PE4 NSS
        dw1000_spi_nss_port = GPIOE;

        rcu_periph_clock_enable(RCU_GPIOE);
        rcu_periph_clock_enable(RCU_SPI3);
        spi_parameter_struct spi_init_struct;

        // PE2 CLK  PE4 NSS  PE5 MISO   PE6 MOSI
        gpio_af_set(GPIOE, GPIO_AF_5, GPIO_PIN_2 | GPIO_PIN_5 | GPIO_PIN_6);
        gpio_mode_set(GPIOE, GPIO_MODE_AF, GPIO_PUPD_NONE,
                      GPIO_PIN_2 | GPIO_PIN_5 | GPIO_PIN_6);
        gpio_output_options_set(GPIOE, GPIO_OTYPE_PP, GPIO_OSPEED_50MHZ,
                                GPIO_PIN_2 | GPIO_PIN_5 | GPIO_PIN_6);

        gpio_mode_set(GPIOE, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE,
                      GPIO_PIN_4);    // NSS
        gpio_output_options_set(GPIOE, GPIO_OTYPE_PP, GPIO_OSPEED_50MHZ,
                                GPIO_PIN_4);

        gpio_mode_set(GPIOE, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE,
                      GPIO_PIN_3);    // RST
        gpio_output_options_set(GPIOE, GPIO_OTYPE_PP, GPIO_OSPEED_50MHZ,
                                GPIO_PIN_3);

        gpio_bit_set(GPIOE, GPIO_PIN_4);

        spi_init_struct.trans_mode = SPI_TRANSMODE_FULLDUPLEX;
        spi_init_struct.device_mode = SPI_MASTER;
        spi_init_struct.frame_size = SPI_FRAMESIZE_8BIT;
        spi_init_struct.clock_polarity_phase = SPI_CK_PL_LOW_PH_1EDGE;
        spi_init_struct.nss = SPI_NSS_SOFT;
        spi_init_struct.prescale = SPI_PSC_128;
        spi_init_struct.endian = SPI_ENDIAN_MSB;
        spi_init(SPI3, &spi_init_struct);

        spi_nss_output_disable(SPI3);
        spi_enable(SPI3);
    }

    void __port_set_dw1000_fastrate_spi3(void) {
        spi_disable(SPI3);

        spi_parameter_struct spi_init_struct;
        spi_struct_para_init(&spi_init_struct);
        spi_init_struct.device_mode = SPI_MASTER;
        spi_init_struct.trans_mode = SPI_TRANSMODE_FULLDUPLEX;
        spi_init_struct.frame_size = SPI_FRAMESIZE_8BIT;
        spi_init_struct.endian = SPI_ENDIAN_MSB;
        spi_init_struct.nss = SPI_NSS_SOFT;
        spi_init_struct.clock_polarity_phase = SPI_CK_PL_LOW_PH_1EDGE;
        spi_init_struct.prescale = SPI_PSC_16;
        spi_init(SPI3, &spi_init_struct);

        spi_enable(SPI3);
    }

    void __hardware_reset() {
        gpio_bit_reset(GPIOE, GPIO_PIN_3);    // RST
        TaskBase::delay(10);                  // �ȴ�10ms
        gpio_bit_set(GPIOE, GPIO_PIN_3);      // RST
        TaskBase::delay(100);                 // �ȴ�10ms
    }
};

#endif