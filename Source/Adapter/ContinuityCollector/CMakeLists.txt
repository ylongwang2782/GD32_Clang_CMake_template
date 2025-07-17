# Collector Module CMakeLists.txt

# Collector library - 数据采集器模块
add_library(AdapterCollector STATIC ContinuityCollector.cpp
                                    ContinuityCollector.h)

# 设置目标属性
target_include_directories(AdapterCollector PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}
                                                   ../adapter_gpio)

# 链接依赖库
target_link_libraries(
  AdapterCollector
  PUBLIC AdapterGpio
  interface
  freertos_kernel
  Logger
  hal_hptimer
  )

# 设置目标属性
set_target_properties(AdapterCollector PROPERTIES CXX_STANDARD 17
                                            CXX_STANDARD_REQUIRED ON)
