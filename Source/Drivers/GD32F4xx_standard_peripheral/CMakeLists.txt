# Drivers/GD32F4xx_standard_peripheral/CMakeLists.txt

# 递归获取 Source 中的所有 .c 文件
file(GLOB GD32_SOURCES
    "${CMAKE_CURRENT_SOURCE_DIR}/Source/*.c"
)

# 创建静态库
add_library(GD32F4xx_standard_peripheral STATIC ${GD32_SOURCES})

# 头文件目录（对外公开）
target_include_directories(GD32F4xx_standard_peripheral PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/Include
)

add_dependencies(GD32F4xx_standard_peripheral CMSIS)
target_link_libraries(GD32F4xx_standard_peripheral CMSIS)

target_compile_options(GD32F4xx_standard_peripheral PRIVATE -Os)
