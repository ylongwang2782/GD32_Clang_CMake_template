add_subdirectory(FreeRTOS)
add_subdirectory(FreeRTOScpp)
add_subdirectory(lwip-2.1.2)

target_include_directories(${EXECUTABLE_NAME} PUBLIC
    ./json/single_include/nlohmann/
)
