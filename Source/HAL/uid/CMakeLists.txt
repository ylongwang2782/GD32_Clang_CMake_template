# HAL UID Module

# 创建HAL UID库
add_library(hal_uid INTERFACE hal_uid.hpp) 

# 包含目录
target_include_directories(hal_uid INTERFACE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}/Source/Core/CMSIS/GD/GD32F4xx/Include
    ${CMAKE_SOURCE_DIR}/Source/Drivers/GD32F4xx_standard_peripheral/Include
)

# 链接依赖
target_link_libraries(hal_uid INTERFACE
    CMSIS
    GD32F4xx_standard_peripheral
) 