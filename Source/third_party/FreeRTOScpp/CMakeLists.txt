add_library(FreeRTOScpp INTERFACE)

target_sources(FreeRTOScpp INTERFACE
    ./CallBack.cpp
    ./Lock.cpp
    ./ReadWrite.cpp
    ./TaskCpp.cpp
)

target_include_directories(FreeRTOScpp INTERFACE
    ./include
)

# Link with FreeRTOS kernel to access FreeRTOS.h
target_link_libraries(FreeRTOScpp INTERFACE
    freertos_kernel
)
