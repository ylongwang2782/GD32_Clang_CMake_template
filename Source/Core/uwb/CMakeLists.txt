target_sources(${EXECUTABLE_NAME} PRIVATE
    ./uwb/uwb.cpp
)

target_include_directories(${EXECUTABLE_NAME} PUBLIC
    ./uci/
    ./uwb/
)