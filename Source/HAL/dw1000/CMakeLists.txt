# HAL DW1000 Library CMakeLists.txt

# Create HAL DW1000 library
add_library(hal_dw1000 STATIC
    ./decadriver/deca_device.c
    ./platform/deca_mutex.c
    ./platform/deca_range_tables.c
    ./platform/deca_sleep.cpp
    ./platform/deca_spi.c
)

# Set include directories for this library
target_include_directories(hal_dw1000 PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/decadriver
    ${CMAKE_CURRENT_SOURCE_DIR}/platform
)

# Link with required libraries
target_link_libraries(hal_dw1000 PUBLIC
    Config
    freertos_kernel
    FreeRTOScpp
    BSP
    GD32F4xx_standard_peripheral
    Logger
) 