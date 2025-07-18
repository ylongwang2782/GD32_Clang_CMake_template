# Configuration CMakeLists.txt
add_subdirectory(FreeRTOSConfig)
add_subdirectory(gd32f4xx_libopt)
add_subdirectory(lwipopts)
add_subdirectory(netcfg)

add_library(Config INTERFACE)

target_include_directories(Config INTERFACE
    ${CMAKE_CURRENT_SOURCE_DIR}
)