# Gpio Module CMakeLists.txt

# Gpio library - GPIO适配器模块
add_library(AdapterGpio STATIC 
    GpioFactory.cpp 
    GpioFactory.h
)

# 根据配置选择性添加硬件GPIO
if(GPIO_USE_HARDWARE)
    target_sources(AdapterGpio PRIVATE
        HardwareGpio.cpp
        HardwareGpio.h
    )
endif()

if(GPIO_USE_VIRTUAL)
    target_sources(AdapterGpio PRIVATE
        VirtualGpio.cpp
        VirtualGpio.h
    )
endif()

# 链接依赖库
target_link_libraries(
  AdapterGpio
  PUBLIC
  interface
  PRIVATE
  Config
  hal_gpio
  CMSIS
  GD32F4xx_standard_peripheral
  Logger
)

# 根据配置添加编译定义
if(GPIO_USE_HARDWARE)
  target_compile_definitions(AdapterGpio PUBLIC GPIO_USE_HARDWARE=1)
endif()

if(GPIO_USE_VIRTUAL)
  target_compile_definitions(AdapterGpio PUBLIC GPIO_USE_VIRTUAL=1)
endif()

# 设置目标属性
set_target_properties(AdapterGpio PROPERTIES CXX_STANDARD 17
                                             CXX_STANDARD_REQUIRED ON)
