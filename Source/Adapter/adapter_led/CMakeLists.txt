# LED Adapter Module CMakeLists.txt

# LED适配器库
add_library(AdapterLed STATIC 
    LedAdapter.cpp 
    LedAdapter.h
    LedFactory.cpp
    LedFactory.h
)

# 设置目标属性
target_include_directories(AdapterLed 
    PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}
    PRIVATE ../adapter_gpio
)

# 链接依赖库
target_link_libraries(
    AdapterLed
    PUBLIC
    interface
    PRIVATE
    AdapterGpio
    freertos_kernel
    Logger
    hal_hptimer
)

# 设置目标属性
set_target_properties(AdapterLed PROPERTIES 
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED ON
) 