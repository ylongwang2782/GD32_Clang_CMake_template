# High Precision Timer (HPTimer) - GD32F4xx

## Overview

This module provides microsecond-level timing precision using GD32F4xx hardware timers, supplementing the FreeRTOS millisecond-level tick with 1000x better resolution.

## Features

- **1μs resolution** - 1000x better than FreeRTOS tick (1ms)
- **32-bit range** - ~71 minutes before wrap-around
- **Hardware-based** - Uses TIMER1 for accurate timing
- **FreeRTOS compatible** - Works alongside existing FreeRTOS timing
- **Wrap-around safe** - Handles timer overflow correctly
- **Low overhead** - Minimal CPU usage

## Technical Details

### Hardware Configuration
- **Timer Used**: TIMER1 (32-bit General-Purpose Timer)
- **Clock Source**: APB1 (60MHz)
- **Prescaler**: 59 (60MHz / 60 = 1MHz = 1μs resolution)
- **Counter**: 32-bit up-counter
- **Period**: 0xFFFFFFFF (maximum range)

### Clock Calculation
```
APB1 Clock = 240MHz / 4 = 60MHz
Timer Clock = 60MHz / (59 + 1) = 1MHz
Resolution = 1/1MHz = 1μs
Max Time = 2^32 μs = 4,294,967,296 μs ≈ 71.58 minutes
```

## API Reference

### Initialization
```c
bool hal_hptimer_init(void);
```
- Initializes TIMER1 for 1μs resolution
- Must be called during system initialization
- Returns `true` on success, `false` on failure

### Time Measurement
```c
uint32_t hal_hptimer_get_us(void);      // Get microsecond timestamp
uint32_t hal_hptimer_get_ms(void);      // Get millisecond timestamp
uint64_t hal_hptimer_get_us64(void);    // Get 64-bit microsecond timestamp
```

### Delays
```c
void hal_hptimer_delay_us(uint32_t us); // Microsecond delay
```

### Timeout Handling
```c
bool hal_hptimer_is_timeout_us(uint32_t ref_time, uint32_t timeout_us);
uint32_t hal_hptimer_elapsed_us(uint32_t ref_time);
```

## Integration Guide

### Step 1: Include Header
```c
// For C files
#include "hptimer.h"

// For C++ files  
#include "hal_hptimer.hpp"
```

### Step 2: Initialize During Startup
```c
// In your main initialization or SlaveDevice::initialize()
if (!hal_hptimer_init()) {
    // Handle initialization failure
    return false;
}
```

### Step 3: Replace Existing Timing Code

#### Before (FreeRTOS tick):
```c
uint32_t startTime = xTaskGetTickCount();
// ... do work ...
if (xTaskGetTickCount() - startTime > pdMS_TO_TICKS(timeout_ms)) {
    // Timeout handling
}
```

#### After (High precision):
```c
uint32_t startTime = hal_hptimer_get_us();
// ... do work ...
if (hal_hptimer_is_timeout_us(startTime, timeout_ms * 1000)) {
    // Timeout handling
}
```

## Usage Examples

### 1. Execution Time Measurement
```c
uint32_t start_time = hal_hptimer_get_us();
// Your code here
uint32_t execution_time = hal_hptimer_elapsed_us(start_time);
printf("Function took %lu μs\n", execution_time);
```

### 2. High Precision Timeout
```c
uint32_t start_time = hal_hptimer_get_us();
uint32_t timeout_us = 5000; // 5ms timeout

while (!condition_met) {
    if (hal_hptimer_is_timeout_us(start_time, timeout_us)) {
        return false; // Timeout
    }
    // Continue waiting
}
```

### 3. Periodic Task (2kHz)
```c
static uint32_t last_run = 0;
const uint32_t period_us = 500; // 500μs = 2kHz

uint32_t current_time = hal_hptimer_get_us();
if (last_run == 0 || hal_hptimer_elapsed_us(last_run) >= period_us) {
    // Execute periodic task
    last_run = current_time;
}
```

### 4. Microsecond Delays
```c
hal_hptimer_delay_us(100);  // 100μs delay
hal_hptimer_delay_us(1500); // 1.5ms delay
```

## Performance Characteristics

### Resolution Comparison
| Method | Resolution | Range | Overhead |
|--------|------------|-------|----------|
| FreeRTOS tick | 1ms | ~49 days | Low |
| HPTimer | 1μs | ~71 minutes | Very Low |
| SysTick | ~4.17ns | 16.7ms | None |

### Memory Usage
- **RAM**: ~8 bytes (static variables)
- **Flash**: ~1KB (code)
- **Hardware**: 1 timer (TIMER1)

## Limitations and Considerations

### 1. Wrap-around Handling
- Timer wraps every ~71 minutes
- All timing functions handle wrap-around correctly
- For periods > 71 minutes, use `hal_hptimer_get_us64()`

### 2. Interrupt Priority
- Timer runs in free-running mode (no interrupts)
- No impact on interrupt latency
- Safe to use in ISRs

### 3. Clock Dependency
- Depends on APB1 clock stability
- Clock changes will affect timing accuracy
- Assumes 240MHz system clock

### 4. Thread Safety
- Reading timer is atomic (32-bit ARM)
- No locks required for basic operations
- Safe for concurrent access

## Troubleshooting

### Common Issues

1. **Timer not initialized**
   - Ensure `hal_hptimer_init()` is called during startup
   - Check return value for initialization success

2. **Timing inaccuracy**
   - Verify system clock is 240MHz
   - Check APB1 clock configuration
   - Ensure timer is not disabled elsewhere

3. **Wrap-around issues**
   - Use `hal_hptimer_elapsed_us()` for time differences
   - Consider `hal_hptimer_get_us64()` for long periods

### Debug Information
```c
// Check if timer is running
if (hal_hptimer_get_us() == 0) {
    // Timer may not be initialized
}

// Verify timer increment
uint32_t t1 = hal_hptimer_get_us();
hal_hptimer_delay_us(1000);
uint32_t t2 = hal_hptimer_get_us();
// t2 - t1 should be approximately 1000
```

## Integration with Your Project

### For SlaveDevice.cpp Line 399
Replace:
```cpp
uint32_t startTime = xTaskGetTickCount();
```

With:
```cpp
uint32_t startTime = hal_hptimer_get_us();
```

And update timeout checks accordingly.

### Benefits for Your Application
1. **Precise timing measurements** - Monitor performance with μs accuracy
2. **Better timeout handling** - Sub-millisecond timeout precision
3. **Improved scheduling** - More accurate periodic tasks
4. **Performance profiling** - Identify bottlenecks with high precision
5. **Protocol timing** - Meet strict timing requirements

## Future Enhancements

1. **Interrupt-based timeouts** - Add callback support
2. **Multiple timers** - Support for additional precision timers
3. **Calibration** - Runtime clock calibration
4. **Power management** - Sleep mode compatibility
5. **Statistics** - Timing statistics collection

## Conclusion

The High Precision Timer provides a significant improvement over FreeRTOS tick timing, offering 1000x better resolution while maintaining compatibility with existing code. It's ideal for applications requiring precise timing measurements, accurate timeouts, or high-frequency periodic tasks. 