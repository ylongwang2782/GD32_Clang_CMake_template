#include "main.h"
#include "gd32f4xx.h"

int main(void)
{
    /* configure systick */
    systick_config();

    /* enable the LEDs GPIO clock */
    rcu_periph_clock_enable(RCU_GPIOC);

    /* configure LED2 GPIO port */
    gpio_mode_set(GPIOC, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, GPIO_PIN_6);
    gpio_output_options_set(GPIOC, GPIO_OTYPE_PP, GPIO_OSPEED_50MHZ, GPIO_PIN_6);
    /* reset LED2 GPIO pin */
    gpio_bit_reset(GPIOC, GPIO_PIN_6);

    while(1) {
        /* turn on LED2 */
        gpio_bit_set(GPIOC, GPIO_PIN_6);
        delay_1ms(500);

        /* turn off LED2 */
        gpio_bit_reset(GPIOC, GPIO_PIN_6);
        delay_1ms(500);
    }
}
