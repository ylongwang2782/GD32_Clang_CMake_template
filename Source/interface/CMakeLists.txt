# interface CMakeLists.txt

# 创建interface库
add_library(interface INTERFACE) 

# 包含目录
target_include_directories(interface INTERFACE
    ${CMAKE_CURRENT_SOURCE_DIR}
)