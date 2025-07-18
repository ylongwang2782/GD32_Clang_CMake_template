# Adapter CX310 Library CMakeLists.txt

# Create Adapter CX310 library
add_library(adapter_cx310 INTERFACE)

# Set include directories for this library
target_include_directories(
  adapter_cx310
  INTERFACE ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_SOURCE_DIR}/Source/HAL/spi
         ${CMAKE_SOURCE_DIR}/Source/HAL/exti
         ${CMAKE_SOURCE_DIR}/Source/HAL/gpio)

target_link_libraries(adapter_cx310 INTERFACE hal_exti hal_spi Logger hal_hptimer)
