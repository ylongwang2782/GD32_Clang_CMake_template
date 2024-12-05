# 添加静态库 启动文件与启动函数
add_library(CMSIS INTERFACE)

target_sources(CMSIS INTERFACE GD/GD32F4xx/Source/GNU/startup_gd32f450_470.S
                               GD/GD32F4xx/Source/system_gd32f4xx.c)

target_include_directories(CMSIS INTERFACE ./GD/GD32F4xx/Include ./)
