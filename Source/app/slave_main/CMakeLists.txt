# Slave Application CMakeLists.txt

# 定义源文件
set(SLAVE_SOURCES slave_mode.cpp SlaveDevice.cpp)

set(SLAVE_HEADERS SlaveDevice.h)

# 创建Slave应用库
add_library(slave_app STATIC ${SLAVE_SOURCES} ${SLAVE_HEADERS})

# 包含目录
target_include_directories(
  slave_app PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/inc ${CMAKE_CURRENT_SOURCE_DIR}
                   ${CMAKE_SOURCE_DIR}/Source/BSP/inc)

# 链接依赖
target_link_libraries(
  slave_app
  PUBLIC hal_button
         hal_dw1000
         input_adapter
         BSP
         Logger
         TaskManager
         FreeRTOScpp
         freertos_kernel
         CMSIS
         GD32F4xx_standard_peripheral
         adapter_cx310
         AdapterGpio
         AdapterLed
         AdapterCollector
         hal_uid
         hal_hptimer)

# 条件编译：只在SLAVE构建变体中包含slave源文件到主执行文件
if(BUILD_VARIANT MATCHES "SLAVE")
  target_sources(${EXECUTABLE_NAME} PUBLIC ${SLAVE_SOURCES})

  target_include_directories(
    ${EXECUTABLE_NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/inc
                               ${CMAKE_CURRENT_SOURCE_DIR})

  target_link_libraries(${EXECUTABLE_NAME} PRIVATE slave_app)
endif()
