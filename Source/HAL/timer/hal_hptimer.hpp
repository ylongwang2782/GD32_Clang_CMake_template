#pragma once

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief High Precision Timer HAL
 * 
 * This module provides microsecond-level timing precision using GD32F4xx hardware timers.
 * It supplements the FreeRTOS millisecond-level tick with higher precision timing.
 */

/**
 * @brief Initialize the high precision timer
 * 
 * Sets up TIMER1 as a high-precision microsecond counter.
 * Timer runs at 1MHz (1μs resolution) with 32-bit range.
 * 
 * @return true if initialization successful, false otherwise
 */
bool hal_hptimer_init(void);

/**
 * @brief Get current microsecond timestamp
 * 
 * @return Current timestamp in microseconds (32-bit, wraps around after ~71 minutes)
 */
uint32_t hal_hptimer_get_us(void);

/**
 * @brief Get current timestamp in milliseconds with microsecond precision
 * 
 * @return Current timestamp in milliseconds (fractional part discarded)
 */
uint32_t hal_hptimer_get_ms(void);

/**
 * @brief Get high precision timestamp combining FreeRTOS tick and microseconds
 * 
 * @return 64-bit timestamp in microseconds since system start
 */
uint64_t hal_hptimer_get_us64(void);

/**
 * @brief Delay for specified microseconds
 * 
 * @param us Microseconds to delay (blocking)
 */
void hal_hptimer_delay_us(uint32_t us);

/**
 * @brief Check if specified microseconds have elapsed since reference time
 * 
 * @param ref_time Reference time in microseconds
 * @param timeout_us Timeout in microseconds
 * @return true if timeout has elapsed, false otherwise
 */
bool hal_hptimer_is_timeout_us(uint32_t ref_time, uint32_t timeout_us);

/**
 * @brief Get elapsed time in microseconds since reference
 * 
 * @param ref_time Reference time in microseconds
 * @return Elapsed microseconds (handles wrap-around)
 */
uint32_t hal_hptimer_elapsed_us(uint32_t ref_time);

#ifdef __cplusplus
}
#endif 