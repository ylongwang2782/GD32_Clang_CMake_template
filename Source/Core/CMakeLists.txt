target_sources(${EXECUTABLE_NAME} PRIVATE
    ./freertos.c
    ./gd32f4xx_it.c
    ./main.c
    ./systick.c
)

target_include_directories(${EXECUTABLE_NAME} PUBLIC
    ./
)
