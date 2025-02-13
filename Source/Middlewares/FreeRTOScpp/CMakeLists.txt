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
