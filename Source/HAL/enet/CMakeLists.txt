# HAL Ethernet Library CMakeLists.txt

# Create HAL Ethernet library
add_library(hal_enet STATIC
    enet_hal.cpp
    ethernetif.cpp
    sys_arch.c
)

# Set include directories for this library
target_include_directories(hal_enet PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)

# Link with required libraries
target_link_libraries(hal_enet PUBLIC
    Config
    lwip_obj
    freertos_kernel
    FreeRTOScpp
    GD32F4xx_standard_peripheral
    Logger
    netcfg
) 