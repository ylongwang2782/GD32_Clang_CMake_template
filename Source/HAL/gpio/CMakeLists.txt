# HAL GPIO Module

# 创建HAL GPIO库
add_library(hal_gpio INTERFACE hal_gpio.hpp)

# 包含目录
target_include_directories(
  hal_gpio
  INTERFACE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}/Source/Core/CMSIS/GD/GD32F4xx/Include
    ${CMAKE_SOURCE_DIR}/Source/Drivers/GD32F4xx_standard_peripheral/Include)

# 链接依赖
target_link_libraries(hal_gpio INTERFACE CMSIS GD32F4xx_standard_peripheral)
