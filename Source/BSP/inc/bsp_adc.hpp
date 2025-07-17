#ifndef BSP_ADC_HPP
#define BSP_ADC_HPP

#include <cstddef>
extern "C" {
#include "FreeRTOS.h"
#include "gd32f4xx.h"
#include "gd32f4xx_dma.h"
#include "queue.h"
#include "semphr.h"
#include "task.h"
}

class Adc {
   public:
    void init() {
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
        adc_channel_length_config(ADC0, ADC_ROUTINE_CHANNEL, 1U);

        /* ADC trigger config */
        adc_external_trigger_source_config(ADC0, ADC_ROUTINE_CHANNEL,
                                           ADC_EXTTRIG_ROUTINE_T0_CH0);
        adc_external_trigger_config(ADC0, ADC_ROUTINE_CHANNEL,
                                    EXTERNAL_TRIGGER_DISABLE);

        /* enable ADC interface */
        adc_enable(ADC0);

        // TaskBase::delay(1);

        /* ADC calibration and reset calibration */
        adc_calibration_enable(ADC0);
    }


    uint16_t read(uint8_t channel) {
        /* ADC routine channel config */
        adc_routine_channel_config(ADC0, 0U, channel, ADC_SAMPLETIME_56);
        /* ADC software trigger enable */
        adc_software_trigger_enable(ADC0, ADC_ROUTINE_CHANNEL);

        /* wait the end of conversion flag */
        while (!adc_flag_get(ADC0, ADC_FLAG_EOC));
        /* clear the end of conversion flag */
        adc_flag_clear(ADC0, ADC_FLAG_EOC);
        /* return regular channel sample value */
        return (adc_routine_data_read(ADC0));
    }
};

#endif