cmake_minimum_required(VERSION 3.10)

# 创建任务管理器库
add_library(TaskManager STATIC
    TaskManager.cpp
    TaskManager.hpp
)

# 设置目标属性
target_include_directories(TaskManager PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)

# 链接依赖
target_link_libraries(TaskManager
    Logger
    FreeRTOScpp
    hal_hptimer
)

# 设置编译选项
set_target_properties(TaskManager PROPERTIES
    CXX_STANDARD 14
    CXX_STANDARD_REQUIRED ON
)

# 添加编译定义
target_compile_definitions(TaskManager PRIVATE
    $<$<CONFIG:Debug>:DEBUG>
    $<$<CONFIG:Release>:NDEBUG>
)