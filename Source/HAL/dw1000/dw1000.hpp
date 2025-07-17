#define DW1000_HPP

#include <stdint.h>

#include <vector>

#include "FreeRTOScpp.h"
#include "Logger.h"
#include "TaskCPP.h"
#include "deca_device_api.h"
#include "deca_regs.h"
#include "gd32f4xx.h"
#include "gd32f4xx_usart.h"

class DW1000 {
   public:
    DW1000() : _is_initialized(false), _rx_enabled(false) {}
    ~DW1000() {}
    
    bool init() {
        __spi3_init();         // 初始化SPI3
        __hardware_reset();    // 复位DW1000
        
        // 设置慢速SPI进行初始化
        __port_set_dw1000_slowrate_spi3();
        
        // 初始化DW1000
        if (dwt_initialise(DWT_LOADNONE) == DWT_ERROR) {
            Log::e("DW1000", "Init Failed");
            return false;
        }
        
        // 设置高速SPI以获得最佳性能
        __port_set_dw1000_fastrate_spi3();
        
        // 配置DW1000
        dwt_configure(&_config);
        
        _is_initialized = true;
        Log::i("DW1000", "Init Success");
        return true;
    }

    bool data_transmit(std::vector<uint8_t>& data) {
        if (!_is_initialized) {
            return false;
        }
        
        if (data.size() > 127) {  // 标准帧长度限制
            return false;
        }
        
        // 停止接收模式
        if (_rx_enabled) {
            stop_recv();
        }
        
        // 写入TX数据到DW1000的TX缓冲区
        dwt_writetxdata(data.size(), data.data(), 0);
        
        // 配置TX帧控制
        dwt_writetxfctrl(data.size(), 0, 0);
        
        // 开始发送
        dwt_starttx(DWT_START_TX_IMMEDIATE);
        
        // 等待发送完成
        while (!(dwt_read32bitreg(SYS_STATUS_ID) & SYS_STATUS_TXFRS)) {
            TaskBase::delay(1);
        }
        
        // 清除发送完成标志
        dwt_write32bitreg(SYS_STATUS_ID, SYS_STATUS_TXFRS);
        
        return true;
    }

    bool get_recv_data(std::vector<uint8_t>& rx_data) {
        if (!_is_initialized || !_rx_enabled) {
            return false;
        }
        
        uint32_t status_reg = dwt_read32bitreg(SYS_STATUS_ID);
        
        // 检查是否接收到好的帧
        if (status_reg & SYS_STATUS_RXFCG) {
            // 读取帧长度
            uint16_t frame_len = dwt_read32bitreg(RX_FINFO_ID) & RX_FINFO_RXFL_MASK_1023;
            
            if (frame_len <= 127) {  // 最大帧长度检查
                rx_data.resize(frame_len);
                dwt_readrxdata(rx_data.data(), frame_len, 0);
                
                // 清除接收完成标志
                dwt_write32bitreg(SYS_STATUS_ID, SYS_STATUS_RXFCG);
                
                return true;
            }
        }
        
        // 检查是否有接收错误
        if (status_reg & SYS_STATUS_ALL_RX_ERR) {
            // 清除错误标志
            dwt_write32bitreg(SYS_STATUS_ID, SYS_STATUS_ALL_RX_ERR);
        }
        
        return false;
    }

    void update() {
        // 定期检查状态，可以在这里处理中断或其他状态更新
        if (_is_initialized && _rx_enabled) {
            uint32_t status_reg = dwt_read32bitreg(SYS_STATUS_ID);
            
            // 如果有接收超时，重新启动接收
            if (status_reg & (SYS_STATUS_RXRFTO | SYS_STATUS_RXPTO)) {
                dwt_write32bitreg(SYS_STATUS_ID, SYS_STATUS_RXRFTO | SYS_STATUS_RXPTO);
                // 重新启动接收
                dwt_rxenable(DWT_START_RX_IMMEDIATE);
            }
        }
    }
    
    bool set_recv_mode() {
        if (!_is_initialized) {
            return false;
        }
        
        // 启动接收模式
        if (dwt_rxenable(DWT_START_RX_IMMEDIATE) == DWT_SUCCESS) {
            _rx_enabled = true;
            return true;
        }
        
        return false;
    }

    bool stop_recv() {
        if (!_is_initialized || !_rx_enabled) {
            return false;
        }
        
        // 通过写入SYS_CTRL寄存器来强制停止接收
        dwt_write32bitreg(SYS_CTRL_ID, SYS_CTRL_TRXOFF);
        
        _rx_enabled = false;
        return true;
    }

   private:
    bool _is_initialized;
    bool _rx_enabled;
    
    // DW1000配置结构体
    dwt_config_t _config = {
        2,               /* Channel number. */
        DWT_PRF_64M,     /* Pulse repetition frequency. */
        DWT_PLEN_1024,   /* Preamble length. Used in TX only. */
        DWT_PAC32,       /* Preamble acquisition chunk size. Used in RX only. */
        9,               /* TX preamble code. Used in TX only. */
        9,               /* RX preamble code. Used in RX only. */
        1,               /* 0 to use standard SFD, 1 to use non-standard SFD. */
        DWT_BR_110K,     /* Data rate. */
        DWT_PHRMODE_STD, /* PHY header mode. */
        (1025 + 64 - 32) /* SFD timeout (preamble length + 1 + SFD length - PAC size). Used in RX only. */
    };
    
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

    void __port_set_dw1000_slowrate_spi3(void) {
        spi_disable(SPI3);

        spi_parameter_struct spi_init_struct;
        spi_struct_para_init(&spi_init_struct);
        spi_init_struct.device_mode = SPI_MASTER;
        spi_init_struct.trans_mode = SPI_TRANSMODE_FULLDUPLEX;
        spi_init_struct.frame_size = SPI_FRAMESIZE_8BIT;
        spi_init_struct.endian = SPI_ENDIAN_MSB;
        spi_init_struct.nss = SPI_NSS_SOFT;
        spi_init_struct.clock_polarity_phase = SPI_CK_PL_LOW_PH_1EDGE;
        spi_init_struct.prescale = SPI_PSC_128;  // 慢速初始化
        spi_init(SPI3, &spi_init_struct);

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
        spi_init_struct.prescale = SPI_PSC_16;  // 高速运行
        spi_init(SPI3, &spi_init_struct);

        spi_enable(SPI3);
    }

    void __hardware_reset() {
        // 初始化复位引脚
        gpio_mode_set(GPIOE, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE,
                      GPIO_PIN_3);    // RST
        gpio_output_options_set(GPIOE, GPIO_OTYPE_PP, GPIO_OSPEED_50MHZ,
                                GPIO_PIN_3);
        gpio_bit_reset(GPIOE, GPIO_PIN_3);    // RST
        TaskBase::delay(10);                  // 等待10ms
        gpio_bit_set(GPIOE, GPIO_PIN_3);      // RST
        TaskBase::delay(100);                 // 等待100ms
    }
};

#endif