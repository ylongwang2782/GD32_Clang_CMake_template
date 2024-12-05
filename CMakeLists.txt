cmake_minimum_required(VERSION 3.22)

# Setup compiler settings
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS ON)

# Define the build type
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Debug")
endif()

# Include toolchain file
include("CMake/arm-none-eabi-gcc.cmake")

set(CMAKE_EXPORT_COMPILE_COMMANDS TRUE)

enable_language(ASM C CXX)

# 设置项目名称和版本号
project(GD32_Clang_Cmake_template VERSION 0.1.0)

add_compile_options(
    -Wno-unused-parameter # 忽略未使用的参数警告
    -Wno-pedantic # 忽略严格模式的警告
    -Wno-unused-variable
)

add_definitions(
    -DGD32F470
)

add_subdirectory(Source)
