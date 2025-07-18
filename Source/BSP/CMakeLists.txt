add_library(BSP INTERFACE)

target_sources(BSP INTERFACE
    ./src/bsp_allocate.cpp
)

target_include_directories(BSP INTERFACE
    ./inc
)

# Link with GD32F4xx standard peripheral library
target_link_libraries(BSP INTERFACE
    GD32F4xx_standard_peripheral
    hal_gpio
)
