add_library(Logger STATIC
    Logger.cpp
    LogManager.cpp
)

target_include_directories(Logger PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)

# 依赖的库
target_link_libraries(Logger PUBLIC
    FreeRTOScpp
    freertos_kernel
    hal_uart
    hal_hptimer
) 