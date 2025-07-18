# netcfg CMakeLists.txt
add_library(netcfg INTERFACE)
target_include_directories(netcfg INTERFACE
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(netcfg INTERFACE
    GD32F4xx_standard_peripheral
)