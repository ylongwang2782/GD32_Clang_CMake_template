# Adapter Ethernet Library CMakeLists.txt

# Create Adapter Ethernet library
add_library(adapter_enet STATIC
    netconf.cpp
)

# Set include directories for this library
target_include_directories(adapter_enet PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)

# Link with required libraries
target_link_libraries(adapter_enet PUBLIC
    hal_enet
    Config
    lwip_obj
    freertos_kernel
    FreeRTOScpp
    Logger
) 