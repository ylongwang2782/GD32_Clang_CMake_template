# lwipopts CMakeLists.txt
add_library(lwipopts INTERFACE)
target_include_directories(lwipopts SYSTEM INTERFACE
    ${CMAKE_CURRENT_SOURCE_DIR}
)