# Hal Uart Module

# 创建HAL UART库
add_library(hal_uart STATIC hal_uart.cpp)

# 包含目录
target_include_directories(hal_uart PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

# 链接依赖
target_link_libraries(hal_uart PUBLIC CMSIS GD32F4xx_standard_peripheral
                                      hal_gpio FreeRTOScpp freertos_kernel)
