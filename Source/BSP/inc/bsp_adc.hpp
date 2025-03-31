#ifndef BSP_ADC_HPP
#define BSP_ADC_HPP

#include <array>
#include <cstddef>

#include "TaskCPP.h"

extern "C" {
#include "FreeRTOS.h"
#include "gd32f4xx.h"
#include "gd32f4xx_dma.h"
#include "queue.h"
#include "semphr.h"
#include "task.h"
}

class AdcSingleConversion {
   public:
    uint16_t value;
    uint32_t gpio_periph;
    uint32_t gpio_pin;
    uint32_t adc_periph;
    uint8_t adc_channel;

    explicit AdcSingleConversion(uint32_t gpio_periph, uint32_t pin,
                                 uint32_t adc_periph)
        : gpio_periph(gpio_periph),
          gpio_pin(pin),
          adc_periph(adc_periph),
          adc_channel(get_adc_channel(pin)) {}

    void init() {
        rcu_config();
        gpio_config();
        adc_config();
    }

    void read() {
        /* ADC routine channel config */
        adc_routine_channel_config(adc_periph, 0U, adc_channel,
                                   ADC_SAMPLETIME_15);
        /* ADC software trigger enable */
        adc_software_trigger_enable(ADC0, ADC_ROUTINE_CHANNEL);

        /* wait the end of conversion flag */
        while (!adc_flag_get(ADC0, ADC_FLAG_EOC));
        /* clear the end of conversion flag */
        adc_flag_clear(ADC0, ADC_FLAG_EOC);
        /* return regular channel sample value */
        value = adc_routine_data_read(ADC0);
    }

   private:
    void gpio_config() {
        gpio_mode_set((uint32_t)gpio_periph, GPIO_MODE_ANALOG, GPIO_PUPD_NONE,
                      gpio_pin);
    }

    void rcu_config(void) {
        /* enable GPIO and ADC clock */
        rcu_periph_clock_enable(get_rcu_periph(gpio_periph));
        rcu_periph_clock_enable(get_rcu_periph(adc_periph));
        adc_clock_config(ADC_ADCCK_PCLK2_DIV8);
    }

    rcu_periph_enum get_rcu_periph(uint32_t periph) {
        // 根据外设返回对应的RCU时钟
        if (periph == GPIOA) return RCU_GPIOA;
        if (periph == GPIOB) return RCU_GPIOB;
        if (periph == GPIOC) return RCU_GPIOC;
        if (periph == ADC0) return RCU_ADC0;
        if (periph == ADC1) return RCU_ADC1;
        // 添加更多外设支持...
        return RCU_GPIOA;    // 默认返回GPIOA
    }

    /*!
        \brief      configure the ADC peripheral
        \param[in]  none
        \param[out] none
        \retval     none
    */
    void adc_config(void) {
        /* reset ADC */
        adc_deinit();
        /* ADC mode config */
        adc_sync_mode_config(ADC_SYNC_MODE_INDEPENDENT);
        /* ADC contineous function disable */
        adc_special_function_config(ADC0, ADC_CONTINUOUS_MODE, DISABLE);
        /* ADC scan mode disable */
        adc_special_function_config(ADC0, ADC_SCAN_MODE, DISABLE);
        /* ADC data alignment config */
        adc_data_alignment_config(ADC0, ADC_DATAALIGN_RIGHT);
        /* ADC channel length config */
        adc_channel_length_config(ADC0, ADC_ROUTINE_CHANNEL, 2U);

        /* ADC trigger config */
        adc_external_trigger_source_config(ADC0, ADC_ROUTINE_CHANNEL,
                                           ADC_EXTTRIG_ROUTINE_T0_CH0);
        adc_external_trigger_config(ADC0, ADC_ROUTINE_CHANNEL,
                                    EXTERNAL_TRIGGER_DISABLE);

        /* ADC Vbat channel enable */
        adc_channel_16_to_18(ADC_VBAT_CHANNEL_SWITCH, ENABLE);

        /* enable ADC interface */
        adc_enable(ADC0);
        TaskBase::delay(1);
        /* ADC calibration and reset calibration */
        adc_calibration_enable(ADC0);
    }

   private:
    uint8_t get_adc_channel(uint32_t pin) {
        // 根据GPIO引脚号返回对应的ADC通道
        switch (pin) {
            case GPIO_PIN_0:
                return 0;
            case GPIO_PIN_1:
                return 1;
            case GPIO_PIN_2:
                return 2;
            case GPIO_PIN_3:
                return 3;
            case GPIO_PIN_4:
                return 4;
            case GPIO_PIN_5:
                return 5;
            case GPIO_PIN_6:
                return 6;
            case GPIO_PIN_7:
                return 7;
            case GPIO_PIN_8:
                return 8;
            case GPIO_PIN_9:
                return 9;
            case GPIO_PIN_10:
                return 10;
            case GPIO_PIN_11:
                return 11;
            case GPIO_PIN_12:
                return 12;
            case GPIO_PIN_13:
                return 13;
            case GPIO_PIN_14:
                return 14;
            case GPIO_PIN_15:
                return 15;
            default:
                return 0;    // 默认通道0
        }
    }
};

class ADCSingleMultiScanDMA {
   private:
    static const uint8_t channel_num = 4;

   public:
    std::array<uint16_t, channel_num> adc_values;
    ADCSingleMultiScanDMA() { init_hardware(); }

    void init_hardware() {
        rcu_config();
        gpio_config();
        dma_config();
        adc_config();
    }

    const std::array<uint16_t, channel_num>& get_values() const {
        return adc_values;
    }

   private:
    void rcu_config() {
        /* enable GPIOC clock */
        rcu_periph_clock_enable(RCU_GPIOA);
        /* enable ADC clock */
        rcu_periph_clock_enable(RCU_ADC0);
        /* enable timer1 clock */
        rcu_periph_clock_enable(RCU_TIMER1);
        rcu_timer_clock_prescaler_config(RCU_TIMER_PSC_MUL4);
        /* enable DMA clock */
        rcu_periph_clock_enable(RCU_DMA1);
        /* config ADC clock */
        adc_clock_config(ADC_ADCCK_PCLK2_DIV8);
    }

    void gpio_config() {
        /* 配置GPIOA的引脚1-4为模拟模式 */
        uint32_t pins = 0;
        for (uint8_t i = 0; i < channel_num; i++) {
            pins |= GPIO_PIN_1 << i;    // 生成引脚位掩码
        }
        gpio_mode_set(GPIOA, GPIO_MODE_ANALOG, GPIO_PUPD_NONE, pins);
    }

    void dma_config() {
        /* ADC_DMA_channel configuration */
        dma_single_data_parameter_struct dma_single_data_parameter;

        /* ADC DMA_channel configuration */
        dma_deinit(DMA1, DMA_CH0);

        /* initialize DMA single data mode */
        dma_single_data_parameter.periph_addr = (uint32_t)(&ADC_RDATA(ADC0));
        dma_single_data_parameter.periph_inc = DMA_PERIPH_INCREASE_DISABLE;
        dma_single_data_parameter.memory0_addr = (uint32_t)(adc_values.data());
        dma_single_data_parameter.memory_inc = DMA_MEMORY_INCREASE_ENABLE;
        dma_single_data_parameter.periph_memory_width = DMA_PERIPH_WIDTH_16BIT;
        dma_single_data_parameter.direction = DMA_PERIPH_TO_MEMORY;
        dma_single_data_parameter.number = channel_num;
        dma_single_data_parameter.priority = DMA_PRIORITY_HIGH;
        dma_single_data_mode_init(DMA1, DMA_CH0, &dma_single_data_parameter);
        dma_channel_subperipheral_select(DMA1, DMA_CH0, DMA_SUBPERI0);

        /* enable DMA circulation mode */
        dma_circulation_enable(DMA1, DMA_CH0);

        /* enable DMA channel */
        dma_channel_enable(DMA1, DMA_CH0);
    }

   private:
    void adc_config() {
        /* ADC mode config */
        adc_sync_mode_config(ADC_SYNC_MODE_INDEPENDENT);
        /* 禁用连续转换模式 */
        adc_special_function_config(ADC0, ADC_CONTINUOUS_MODE, DISABLE);
        /* 启用扫描模式 */
        adc_special_function_config(ADC0, ADC_SCAN_MODE, ENABLE);
        /* ADC data alignment config */
        adc_data_alignment_config(ADC0, ADC_DATAALIGN_RIGHT);

        /* ADC channel length config */
        adc_channel_length_config(ADC0, ADC_ROUTINE_CHANNEL, channel_num);
        /* ADC routine channel config */
        // adc routine channel config according to channel_num
        for (uint8_t i = 0; i < channel_num; i++) {
            adc_routine_channel_config(ADC0, i, ADC_CHANNEL_1 + i,
                                       ADC_SAMPLETIME_15);
        }
        /* ADC trigger config */
        adc_external_trigger_source_config(ADC0, ADC_ROUTINE_CHANNEL,
                                           ADC_EXTTRIG_ROUTINE_T0_CH0);
        adc_external_trigger_config(ADC0, ADC_ROUTINE_CHANNEL,
                                    EXTERNAL_TRIGGER_DISABLE);

        /* ADC DMA function enable */
        adc_dma_request_after_last_enable(ADC0);
        adc_dma_mode_enable(ADC0);

        /* enable ADC interface */
        adc_enable(ADC0);
        /* wait for ADC stability */
        TaskBase::delay(1);
        /* ADC calibration and reset calibration */
        adc_calibration_enable(ADC0);

        /* enable ADC software trigger */
        adc_software_trigger_enable(ADC0, ADC_ROUTINE_CHANNEL);
    }

   public:
    // 添加手动触发函数
    void trigger_scan() {
        /* 手动触发单次转换 */
        adc_software_trigger_enable(ADC0, ADC_ROUTINE_CHANNEL);
    }
};

#endif