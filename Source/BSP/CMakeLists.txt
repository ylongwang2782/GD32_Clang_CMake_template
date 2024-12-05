add_library(BSP INTERFACE)

target_sources(BSP INTERFACE
    ./src/bsp_uart.cpp
    ./src/bsp_log.cpp
)

target_include_directories(BSP INTERFACE
    ./inc
)
