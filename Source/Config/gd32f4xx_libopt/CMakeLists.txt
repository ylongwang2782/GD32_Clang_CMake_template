# gd32f4xx_libopt CMakeLists.txt
add_library(gd32f4xx_libopt INTERFACE)
target_include_directories(gd32f4xx_libopt SYSTEM INTERFACE
    ${CMAKE_CURRENT_SOURCE_DIR}
)
target_compile_definitions(gd32f4xx_libopt INTERFACE
    projCOVERAGE_TEST=0
)

target_link_libraries(gd32f4xx_libopt INTERFACE
    GD32F4xx_standard_peripheral
)