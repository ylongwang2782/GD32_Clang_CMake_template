# HAL EXTI Library CMakeLists.txt

# Create HAL EXTI library
add_library(hal_exti STATIC hal_exti.cpp)

# Set include directories for this library
target_include_directories(hal_exti PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

# Link with required libraries
target_link_libraries(hal_exti PUBLIC GD32F4xx_standard_peripheral)
