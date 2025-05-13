cmake_minimum_required(VERSION 3.19)

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

if(BUILD_VARIANT STREQUAL "DEBUG")
project(GD32_CPP_OS_CMAKE VERSION 1.0)
elseif(BUILD_VARIANT STREQUAL "RELEASE")
project(GD32_CPP_OS_CMAKE VERSION 1.0)
else()
  message(FATAL_ERROR "Unknown BUILD_VARIANT value: ${BUILD_VARIANT}")
endif()

add_compile_options(
  -Wno-unused-parameter # 忽略未使用的参数警告
  -Wno-pedantic # 忽略严格模式的警告
  -Wno-unused-variable)

add_link_options(-Wl,--unresolved-symbols=ignore-in-object-files)

add_definitions(-DGD32F470 -DGCC)

add_subdirectory(Source)
