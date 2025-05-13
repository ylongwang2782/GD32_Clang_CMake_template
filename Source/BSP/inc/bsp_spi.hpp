#ifndef BSP_SPI_H
#define BSP_SPI_H
#include <cstdint>
#include <type_traits>
#include <vector>

#include "QueueCpp.h"
#include "TaskCPP.h"
#include "bsp_gpio.hpp"
#include "bsp_log.hpp"
#include "gd32f4xx.h"
#include "gd32f4xx_misc.h"
#include "gd32f4xx_spi.h"
#include "task.h"

#define SPI_TASK_STACK_DEPTH 512
#define SPI_TASK_PRIORITY    TaskPrio_Mid

typedef struct {
    uint32_t spi_periph;
    rcu_periph_enum spi_periph_clock;

    uint32_t mosi_port;
    uint32_t mosi_pin;
    int32_t mosi_func_num;

    uint32_t miso_port;
    uint32_t miso_pin;
    int32_t miso_func_num;

    uint32_t sclk_port;
    uint32_t sclk_pin;
    int32_t sclk_func_num;

    uint32_t nss_port;
    uint32_t nss_pin;
    int32_t nss_func_num;

} Spi_IOConfig;

typedef struct {
    uint16_t rx_buffer_size;
    uint16_t tx_buffer_size;
} SpiBufferInfo;

typedef struct {
    uint32_t prescale;
    uint32_t clock_polarity_phase;
    uint32_t frame_size;
    uint32_t endian;
} Spi_PeriphConfig;

extern const Spi_IOConfig SPI1_C1MOSI_C2MISO_B10SCLK_B12NSS;
extern const Spi_IOConfig SPI1_C1MOSI_C2MISO_C7SCLK_B12NSS;
extern const Spi_IOConfig SPI3_E6MOSI_E5MISO_E2SCLK_E11NSS;

extern const Spi_PeriphConfig SPI_CFG1;

typedef struct {
    uint32_t nss_port;
    uint32_t nss_pin;
} NSS_IO;

enum class SpiMode { Master, Slave };
class SpiDevBase {
   public:
    SpiDevBase(Spi_IOConfig& cfg, SpiMode mode) : __cfg(cfg), __mode(mode) {
        if (!__bsp_is_init) {
            __bsp_is_init = 1;
            for (uint8_t i = 0; i < (uint8_t)SpiEnum::SpiNum; i++) {
                __dev[i] = nullptr;
            }
        }
        switch (cfg.spi_periph) {
            case SPI1:
                __dev[(uint8_t)SpiEnum::Spi1] = this;
                __irqn = SPI1_IRQn;
                break;
            case SPI2:
                __dev[(uint8_t)SpiEnum::Spi2] = this;
                __irqn = SPI2_IRQn;
                break;
            case SPI3:
                __dev[(uint8_t)SpiEnum::Spi3] = this;
                __irqn = SPI3_IRQn;
                break;
            default:
                break;
        }
    };
    ~SpiDevBase() {};

   public:
    enum class SpiEnum {
        Spi1,
        Spi2,
        Spi3,
        // Spi4,
        // Spi5,

        SpiNum
    };
    Queue<uint8_t>* __rx_buffer = nullptr;
    Queue<uint8_t>* __tx_buffer = nullptr;
    Spi_IOConfig __cfg;
    SpiMode __mode;
    uint8_t __irqn;
    uint8_t tx_data;
    long waswoken;
    void* user_callback_arg;
    void (*__user_rx_callback)(void* arg) = nullptr;

    static SpiDevBase* __dev[(uint8_t)SpiEnum::SpiNum];
    static uint8_t __bsp_is_init;

   public:
    void __rx_isr_callback(uint8_t rx_data) {
        if (__mode == SpiMode::Slave) {
            if (__rx_buffer != nullptr) {
                __rx_buffer->add_ISR(rx_data, waswoken);
                portYIELD_FROM_ISR(waswoken);
            }
            if (__tx_buffer != nullptr) {
                if (__tx_buffer->pop_ISR(tx_data, waswoken)) {
                    spi_i2s_data_transmit(__cfg.spi_periph, tx_data);
                }
            }
        }
        if (__user_rx_callback != nullptr) {
            __user_rx_callback(user_callback_arg);
        }
    }
    uint32_t get_spi_periph() const { return __cfg.spi_periph; }

    //    private:
};

class SpiMaster : private SpiDevBase {
   private:
    std::vector<GPIO> __nss;
    GPIO __mosi;
    GPIO __miso;
    GPIO __sclk;
    std::vector<GPIO> __nss_init(std::vector<NSS_IO> nss_io) {
        std::vector<GPIO> nss;
        for (auto i : nss_io) {
            nss.push_back(GPIO((GPIO::Port)i.nss_port, (GPIO::Pin)i.nss_pin,
                               GPIO::Mode::OUTPUT, GPIO::PullUpDown::NONE,
                               GPIO::OType::PP, GPIO::Speed::SPEED_50MHZ));
            nss.back().bit_set();
        }
        return nss;
    }

   public:
    std::vector<uint8_t> rx_buffer;
    /**
     * param[in]  prescale: SPI_PSC_n (n=2,4,8,16,32,64,128,256)

    **/
    SpiMaster(Spi_IOConfig io_cfg, Spi_PeriphConfig spi_cfg,
              std::vector<NSS_IO> nss_io)
        : SpiDevBase(io_cfg, SpiMode::Master),
          __nss(__nss_init(nss_io)),
          __mosi((GPIO::Port)__cfg.mosi_port, (GPIO::Pin)__cfg.mosi_pin,
                 GPIO::Mode::AF, GPIO::PullUpDown::NONE, GPIO::OType::PP,
                 GPIO::Speed::SPEED_50MHZ),
          __miso((GPIO::Port)__cfg.miso_port, (GPIO::Pin)__cfg.miso_pin,
                 GPIO::Mode::AF, GPIO::PullUpDown::NONE, GPIO::OType::PP,
                 GPIO::Speed::SPEED_50MHZ),
          __sclk((GPIO::Port)__cfg.sclk_port, (GPIO::Pin)__cfg.sclk_pin,
                 GPIO::Mode::AF, GPIO::PullUpDown::NONE, GPIO::OType::PP,
                 GPIO::Speed::SPEED_50MHZ) {
        __mosi.af_set(__cfg.mosi_func_num);
        __miso.af_set(__cfg.miso_func_num);
        __sclk.af_set(__cfg.sclk_func_num);

        spi_parameter_struct spi_init_struct;
        rcu_periph_clock_enable(__cfg.spi_periph_clock);
        spi_init_struct.trans_mode = SPI_TRANSMODE_FULLDUPLEX;
        spi_init_struct.device_mode = SPI_MASTER;
        spi_init_struct.frame_size = spi_cfg.frame_size;
        spi_init_struct.clock_polarity_phase = spi_cfg.clock_polarity_phase;
        spi_init_struct.nss = SPI_NSS_SOFT;
        spi_init_struct.prescale = spi_cfg.prescale;
        spi_init_struct.endian = spi_cfg.endian;
        spi_init(__cfg.spi_periph, &spi_init_struct);
        spi_enable(__cfg.spi_periph);
        // spi_init();
    }
    ~SpiMaster() {};

    void nss_high(uint8_t nss_index = 0) { __nss[nss_index].bit_set(); }
    void nss_low(uint8_t nss_index = 0) { __nss[nss_index].bit_reset(); }

    bool send(std::vector<uint8_t> tx_data, uint16_t timeout_ms = 1000,
              uint8_t nss_index = 0) {
        uint32_t timeout_tick = pdMS_TO_TICKS(timeout_ms);
        uint32_t txcount = 0;
        nss_low(nss_index);
        uint32_t tickstart = xTaskGetTickCount();
        while (txcount < tx_data.size()) {
            if (SET == spi_i2s_flag_get(__cfg.spi_periph, SPI_FLAG_TBE)) {
                spi_i2s_data_transmit(__cfg.spi_periph, tx_data[txcount++]);
            }

            if (xTaskGetTickCount() - tickstart > timeout_tick) {
                nss_high(nss_index);
                return false;
            }
        }
        nss_high(nss_index);
        return true;
    }

    bool recv(uint32_t rx_len, uint16_t timeout_ms = 1000,
              uint8_t nss_index = 0) {
        uint32_t timeout_tick = pdMS_TO_TICKS(timeout_ms);
        uint32_t rxcount = 0;
        uint8_t txallowed = 1;
        rx_buffer.clear();
        if (rx_len > rx_buffer.capacity()) {
            rx_buffer.reserve(rx_len);
        }
        nss_low(nss_index);
        uint32_t tickstart = xTaskGetTickCount();
        while (rxcount < rx_len) {
            if (txallowed) {
                txallowed = 0;
                spi_i2s_data_transmit(__cfg.spi_periph, 0xFF);
            }
            if (RESET != spi_i2s_flag_get(__cfg.spi_periph, SPI_FLAG_RBNE)) {
                rx_buffer.push_back(spi_i2s_data_receive(__cfg.spi_periph));
                rxcount++;
                txallowed = 1;
            }
            if (xTaskGetTickCount() - tickstart > timeout_tick) {
                nss_high(nss_index);
                return false;
            }
        }

        nss_high(nss_index);
        return true;
    }
    bool send_recv(std::vector<uint8_t> tx_data, uint32_t rx_len,
                   uint16_t timeout_ms = 1000, uint8_t nss_index = 0) {
        uint32_t tx_size = tx_data.size() > rx_len ? tx_data.size() : rx_len;
        uint32_t txcount = 0;
        uint32_t rxcount = 0;
        uint8_t txallowed = 1;
        uint32_t timeout_tick = pdMS_TO_TICKS(timeout_ms);
        rx_buffer.clear();
        if (rx_len > rx_buffer.capacity()) {
            rx_buffer.reserve(rx_len);
        }

        nss_low(nss_index);
        uint32_t tickstart = xTaskGetTickCount();
        while ((txcount < tx_size) || (rxcount < rx_len)) {
            if (txallowed) {
                txallowed = 0;
                if (txcount < tx_data.size()) {
                    spi_i2s_data_transmit(__cfg.spi_periph, tx_data[txcount]);
                } else {
                    spi_i2s_data_transmit(__cfg.spi_periph, 0xFF);
                }
                txcount++;
            }
            if (RESET != spi_i2s_flag_get(__cfg.spi_periph, SPI_FLAG_RBNE) &&
                rxcount < rx_len) {
                rx_buffer.push_back(spi_i2s_data_receive(__cfg.spi_periph));
                rxcount++;
                txallowed = 1;
            }
            if (rxcount >= rx_len &&
                SET == spi_i2s_flag_get(__cfg.spi_periph, SPI_STAT_TBE)) {
                txallowed = 1;
            }

            if (xTaskGetTickCount() - tickstart > timeout_tick) {
                nss_high(nss_index);
                return false;
            }
        }
        nss_high(nss_index);
        return true;
    }
};

class SpiSlave : private SpiDevBase {
   private:
    GPIO __nss;
    GPIO __mosi;
    GPIO __miso;
    GPIO __sclk;

   public:
    Queue<uint8_t> rx_queue;

   private:
    Queue<uint8_t> tx_queue;
    uint16_t __tx_count = 0;
    long waswoken;

   public:
    SpiSlave(Spi_IOConfig io_cfg, uint16_t tx_buffer_size,
             uint16_t rx_buffer_size, Spi_PeriphConfig spi_cfg)
        : SpiDevBase(io_cfg, SpiMode::Slave),
          __nss((GPIO::Port)__cfg.nss_port, (GPIO::Pin)__cfg.nss_pin,
                GPIO::Mode::AF, GPIO::PullUpDown::NONE, GPIO::OType::PP,
                GPIO::Speed::SPEED_50MHZ),
          __mosi((GPIO::Port)__cfg.mosi_port, (GPIO::Pin)__cfg.mosi_pin,
                 GPIO::Mode::AF, GPIO::PullUpDown::NONE, GPIO::OType::PP,
                 GPIO::Speed::SPEED_50MHZ),
          __miso((GPIO::Port)__cfg.miso_port, (GPIO::Pin)__cfg.miso_pin,
                 GPIO::Mode::AF, GPIO::PullUpDown::NONE, GPIO::OType::PP,
                 GPIO::Speed::SPEED_50MHZ),
          __sclk((GPIO::Port)__cfg.sclk_port, (GPIO::Pin)__cfg.sclk_pin,
                 GPIO::Mode::AF, GPIO::PullUpDown::NONE, GPIO::OType::PP,
                 GPIO::Speed::SPEED_50MHZ),
          rx_queue(tx_buffer_size),
          tx_queue(rx_buffer_size) {
        __nss.af_set(__cfg.nss_func_num);
        __mosi.af_set(__cfg.mosi_func_num);
        __miso.af_set(__cfg.miso_func_num);
        __sclk.af_set(__cfg.sclk_func_num);
        __rx_buffer = &this->rx_queue;
        __tx_buffer = &this->tx_queue;
        spi_parameter_struct spi_init_struct;
        rcu_periph_clock_enable(__cfg.spi_periph_clock);
        spi_init_struct.trans_mode = SPI_TRANSMODE_FULLDUPLEX;
        spi_init_struct.device_mode = SPI_SLAVE;
        spi_init_struct.frame_size = spi_cfg.frame_size;
        spi_init_struct.clock_polarity_phase = spi_cfg.clock_polarity_phase;
        spi_init_struct.nss = SPI_NSS_SOFT;
        spi_init_struct.prescale = spi_cfg.prescale;
        spi_init_struct.endian = spi_cfg.endian;
        spi_init(__cfg.spi_periph, &spi_init_struct);
        spi_enable(__cfg.spi_periph);

        nvic_irq_enable(__irqn, 10, 0);
        spi_i2s_interrupt_enable(__cfg.spi_periph, SPI_I2S_INT_RBNE);
        // spi_i2s_interrupt_enable(__cfg.spi_periph, SPI_I2S_INT_TBE);
    }
    void set_rx_callback(void (*callback)(void* arg), void* arg) {
        __user_rx_callback = callback;
        user_callback_arg = arg;
    }

    bool send(uint8_t* tx_data, uint16_t len, uint16_t timeout_ms = 1000) {
        __load_tx_data(tx_data, len);
        return __check_tx_done(timeout_ms);
    }

    bool send(std::vector<uint8_t> tx_data, uint16_t timeout_ms = 1000) {
        __load_tx_data(tx_data.data(), tx_data.size());
        return __check_tx_done(timeout_ms);
    }

   private:
    void __load_tx_data(uint8_t* tx_data, uint16_t len) {
        __tx_count = 0;
        while (__tx_count < len) {
            if (__tx_count == 0 &&
                SET == spi_i2s_flag_get(__cfg.spi_periph, SPI_FLAG_TBE)) {
                // 直接加载第一个字节
                spi_i2s_data_transmit(__cfg.spi_periph, tx_data[__tx_count++]);

            } else {
                if (xPortIsInsideInterrupt() == pdTRUE) {
                    tx_queue.add_ISR(tx_data[__tx_count], waswoken);
                } else {
                    tx_queue.add(tx_data[__tx_count]);
                }
                __tx_count++;
            }
        }
    }

    bool __check_tx_done(uint16_t timeout_ms) {
        uint32_t timeout_tick = pdMS_TO_TICKS(timeout_ms);
        uint32_t tickstart = xTaskGetTickCount();

        while ((xTaskGetTickCount() - tickstart < timeout_tick) ||
               (timeout_ms == 0)) {
            if (xPortIsInsideInterrupt() == pdTRUE) {
                if (tx_queue.empty_ISR() &&
                    (SET == spi_i2s_flag_get(__cfg.spi_periph, SPI_FLAG_TBE))) {
                    __tx_count = 0;
                    return true;
                } else {
                    if (timeout_ms == 0) {
                        return false;
                    }
                }
            } else {
                if (tx_queue.empty() &&
                    SET == spi_i2s_flag_get(__cfg.spi_periph, SPI_FLAG_TBE)) {
                    __tx_count = 0;
                    return true;
                } else if (timeout_ms == 0) {
                    return false;
                }
            }
        }

        return false;
    }
};

extern "C" {
void SPI1_IRQHandler(void);
void SPI2_IRQHandler(void);
void SPI3_IRQHandler(void);
}
template <SpiMode mode>
class SpiDev : public std::conditional<mode == SpiMode::Master, SpiMaster,
                                       SpiSlave>::type {
   public:
    template <typename Enable = void,
              std::enable_if_t<mode == SpiMode::Master, Enable>* = nullptr>
    SpiDev(Spi_IOConfig io_cfg, std::vector<NSS_IO> nss_io,
           Spi_PeriphConfig spi_cfg = SPI_CFG1)
        : SpiMaster(io_cfg, spi_cfg, nss_io), __cfg(io_cfg) {}

    template <typename Enable = void,
              std::enable_if_t<mode == SpiMode::Slave, Enable>* = nullptr>
    SpiDev(Spi_IOConfig io_cfg, uint16_t tx_buffer_size = 256,
           uint16_t rx_buffer_size = 256, Spi_PeriphConfig spi_cfg = SPI_CFG1)
        : SpiSlave(io_cfg, tx_buffer_size, rx_buffer_size, spi_cfg),
          __cfg(io_cfg) {}
    ~SpiDev() {};

    void enable() { spi_enable(__cfg.spi_periph); }
    void disable() { spi_disable(__cfg.spi_periph); }
    friend void SPI1_IRQHandler(void);
    friend void SPI2_IRQHandler(void);
    friend void SPI3_IRQHandler(void);

   private:
    Spi_IOConfig __cfg;
};

class SpiTask : public TaskClassS<SPI_TASK_STACK_DEPTH> {
   private:
    Logger& Log;

   public:
    SpiTask(Logger& Log)
        : TaskClassS<SPI_TASK_STACK_DEPTH>("SpiTask", TaskPrio_Mid), Log(Log) {}

    static void spiCallback(void* arg) {
        SpiDev<SpiMode::Slave>* dev = (SpiDev<SpiMode::Slave>*)arg;
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

#endif