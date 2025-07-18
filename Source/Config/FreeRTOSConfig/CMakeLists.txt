# Create freertos_config target for FreeRTOS
add_library(freertos_config INTERFACE)
target_include_directories(freertos_config SYSTEM INTERFACE
    ${CMAKE_CURRENT_SOURCE_DIR}
)
target_compile_definitions(freertos_config INTERFACE
    projCOVERAGE_TEST=0
)