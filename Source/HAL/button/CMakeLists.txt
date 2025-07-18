# HAL Button Module

set(HAL_BUTTON_SOURCES
    button.cpp
)

set(HAL_BUTTON_HEADERS
    button.h
)

# 创建HAL Button库
add_library(hal_button STATIC ${HAL_BUTTON_SOURCES} ${HAL_BUTTON_HEADERS})

# 包含目录
target_include_directories(hal_button PUBLIC
)

# 链接依赖
target_link_libraries(hal_button PRIVATE
    CMSIS
    GD32F4xx_standard_peripheral
    hal_gpio
    interface
) 