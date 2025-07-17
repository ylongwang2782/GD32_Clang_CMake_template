# HAL HP Timer Library CMakeLists.txt

# Create HAL HP Timer library
add_library(hal_hptimer STATIC hal_hptimer.cpp)

# Set include directories for this library
target_include_directories(hal_hptimer PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

# Link with required libraries
target_link_libraries(hal_hptimer PUBLIC GD32F4xx_standard_peripheral
                                         FreeRTOScpp)
