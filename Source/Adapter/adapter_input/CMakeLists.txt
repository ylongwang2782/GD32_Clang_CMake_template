# InputAdapter Module

add_library(input_adapter STATIC InputAdapter.cpp)

target_include_directories(input_adapter PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(input_adapter PUBLIC
    hal_button
    interface
) 