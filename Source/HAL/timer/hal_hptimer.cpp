#include "hal_hptimer.hpp"
#include "gd32f4xx.h"

#ifdef __cplusplus
extern "C" {
#endif

#include "FreeRTOS.h"
#include "task.h"

#ifdef __cplusplus
}
#endif

// Use TIMER1 for high precision timing
// TIMER1 is a 32-bit general-purpose timer, perfect for high precision counting
#define HP_TIMER                TIMER1
#define HP_TIMER_RCU            RCU_TIMER1

// Timer configuration
#define HP_TIMER_PRESCALER      119      // APB1 clock (60MHz) / (59+1) = 1MHz (1μs resolution)
#define HP_TIMER_PERIOD         0xFFFFFFFF  // 32-bit timer, maximum period

static volatile bool s_initialized = false;

bool hal_hptimer_init(void)
{
    if (s_initialized) {
        return true;
    }

    // Enable timer clock
    rcu_periph_clock_enable(HP_TIMER_RCU);
    
    // Reset timer
    rcu_periph_reset_enable(RCU_TIMER1RST);
    rcu_periph_reset_disable(RCU_TIMER1RST);

    // Configure timer parameters
    timer_parameter_struct timer_init_struct;
    timer_struct_para_init(&timer_init_struct);
    
    timer_init_struct.prescaler = HP_TIMER_PRESCALER;
    timer_init_struct.alignedmode = TIMER_COUNTER_EDGE;
    timer_init_struct.counterdirection = TIMER_COUNTER_UP;
    timer_init_struct.period = HP_TIMER_PERIOD;
    timer_init_struct.clockdivision = TIMER_CKDIV_DIV1;
    timer_init_struct.repetitioncounter = 0;

    // Initialize timer
    timer_init(HP_TIMER, &timer_init_struct);
    
    // Enable auto-reload shadow register
    timer_auto_reload_shadow_enable(HP_TIMER);
    
    // Start the timer
    timer_enable(HP_TIMER);
    
    s_initialized = true;
    return true;
}

uint32_t hal_hptimer_get_us(void)
{
    if (!s_initialized) {
        return 0;
    }
    
    return timer_counter_read(HP_TIMER);
}

uint32_t hal_hptimer_get_ms(void)
{
    return hal_hptimer_get_us() / 1000;
}

uint64_t hal_hptimer_get_us64(void)
{
    if (!s_initialized) {
        return 0;
    }
    
    // Get FreeRTOS tick count (milliseconds)
    TickType_t tick_count = xTaskGetTickCount();
    
    // Get current microsecond counter
    uint32_t us_counter = hal_hptimer_get_us();
    
    // Combine to create 64-bit timestamp
    // Note: This assumes timer wraps less frequently than FreeRTOS tick updates
    uint64_t total_us = (uint64_t)tick_count * 1000ULL + (us_counter % 1000);
    
    return total_us;
}

void hal_hptimer_delay_us(uint32_t us)
{
    if (!s_initialized || us == 0) {
        return;
    }
    
    uint32_t start_time = hal_hptimer_get_us();
    
    // For very short delays, use busy wait
    if (us < 1000) {
        while (hal_hptimer_elapsed_us(start_time) < us) {
            // Busy wait
        }
    } else {
        // For longer delays, yield to other tasks periodically
        while (hal_hptimer_elapsed_us(start_time) < us) {
            if (hal_hptimer_elapsed_us(start_time) % 100 == 0) {
                taskYIELD();
            }
        }
    }
}

bool hal_hptimer_is_timeout_us(uint32_t ref_time, uint32_t timeout_us)
{
    return hal_hptimer_elapsed_us(ref_time) >= timeout_us;
}

uint32_t hal_hptimer_elapsed_us(uint32_t ref_time)
{
    if (!s_initialized) {
        return 0;
    }
    
    uint32_t current_time = hal_hptimer_get_us();
    
    // Handle wrap-around (32-bit counter wraps after ~71 minutes)
    if (current_time >= ref_time) {
        return current_time - ref_time;
    } else {
        // Wrap-around occurred
        return (0xFFFFFFFF - ref_time) + current_time + 1;
    }
} 