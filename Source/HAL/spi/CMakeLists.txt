# HAL SPI Library CMakeLists.txt

# Create HAL SPI library
add_library(hal_spi STATIC hal_spi.cpp)

# Set include directories for this library
target_include_directories(hal_spi PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

# Link with required libraries
target_link_libraries(hal_spi PUBLIC GD32F4xx_standard_peripheral FreeRTOScpp
                                     hal_gpio)
