cmake_minimum_required(VERSION 3.22)

# 使用字符串连接操作生成可执行文件的名称
set(EXECUTABLE_NAME "${PROJECT_NAME}_${PROJECT_VERSION}")

# 添加编译选项
add_compile_options(
  -Wno-unused-parameter # 忽略未使用的参数警告
  -Wno-pedantic # 忽略严格模式的警告
)

# 添加可执行文件目标
add_executable(${EXECUTABLE_NAME})

# 设置目标的私有包含目录
include_directories(Config # 配置文件目录
)

# 设置目标的私有编译定义，cpp需要提前定义__cpluscplus
target_compile_definitions(${EXECUTABLE_NAME} PRIVATE GD32F470 __cpluscplus)

add_subdirectory(BSP)
add_subdirectory(Core)
add_subdirectory(Drivers)
add_subdirectory(Middlewares)
add_subdirectory(Retarget)

# 链接目标与其他库
target_link_libraries(${EXECUTABLE_NAME} BSP Retarget Drivers freertos_kernel)

# ===============================================================================

# 设置可执行文件的生成位置
set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin/${CMAKE_BUILD_TYPE}/)

# 指定map文件生成路径
target_link_options(
  ${EXECUTABLE_NAME} PRIVATE
  "-Wl,-Map=$<TARGET_FILE_DIR:${EXECUTABLE_NAME}>/${EXECUTABLE_NAME}.elf.map")

# 添加编译后命令-生成bin和Hex格式固件、打印可执行文件尺寸信息
include(${PROJECT_SOURCE_DIR}/CMake/create_bin.cmake)

# 设置cmake install指令，烧录程序
include(${PROJECT_SOURCE_DIR}/CMake/flash_by_install.cmake)
