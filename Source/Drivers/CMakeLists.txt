add_subdirectory(CMSIS)
add_subdirectory(GD32F4xx_standard_peripheral)
add_subdirectory(DW1000)

add_library(Drivers INTERFACE)

target_link_libraries(Drivers INTERFACE
    CMSIS
    GD32F4xx_standard_peripheral
    # CX310
    DW1000
)
