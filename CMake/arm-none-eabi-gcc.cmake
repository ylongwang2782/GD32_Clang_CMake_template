set(CMAKE_SYSTEM_NAME Generic)
set(CMAKE_SYSTEM_PROCESSOR arm)

# 强制使用指定的C/C++编译器
set(CMAKE_C_COMPILER_FORCED TRUE)
set(CMAKE_CXX_COMPILER_FORCED TRUE)
set(CMAKE_C_COMPILER_ID GNU)
set(CMAKE_CXX_COMPILER_ID GNU)

# 查找工具链
set(CMAKE_ASM_COMPILER arm-none-eabi-gcc)
set(CMAKE_C_COMPILER arm-none-eabi-gcc)
set(CMAKE_CXX_COMPILER arm-none-eabi-g++)

set(CMAKE_EXECUTABLE_SUFFIX_ASM ".elf")
set(CMAKE_EXECUTABLE_SUFFIX_C ".elf")
set(CMAKE_EXECUTABLE_SUFFIX_CXX ".elf")

set(CMAKE_TRY_COMPILE_TARGET_TYPE STATIC_LIBRARY)

# MCU specific flags
set(TARGET_FLAGS "-mcpu=cortex-m4 -mfpu=fpv4-sp-d16 -mfloat-abi=hard ")

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${TARGET_FLAGS}")
set(CMAKE_C_FLAGS # "${CMAKE_C_FLAGS} -Wall -Wextra -Wpedantic -fdata-sections
                  # -ffunction-sections"
    "${CMAKE_C_FLAGS} -Wpedantic -fdata-sections -ffunction-sections")

if(CMAKE_BUILD_TYPE MATCHES Debug)
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -O0 -g3")
endif()

if(CMAKE_BUILD_TYPE MATCHES Release)
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Os -g0")
endif()

set(CMAKE_ASM_FLAGS "${CMAKE_C_FLAGS} -x assembler-with-cpp -MMD -MP")
set(CMAKE_CXX_FLAGS # "${CMAKE_C_FLAGS} -fno-rtti -fno-exceptions
                    # -fno-threadsafe-statics")
    "${CMAKE_C_FLAGS} -fno-exceptions -fno-threadsafe-statics")

set(CMAKE_C_LINK_FLAGS "${TARGET_FLAGS}")
set(CMAKE_C_LINK_FLAGS
    "${CMAKE_C_LINK_FLAGS} -T \"${CMAKE_SOURCE_DIR}/gd32f470zi_flash.ld\"")
set(CMAKE_C_LINK_FLAGS "${CMAKE_C_LINK_FLAGS} --specs=nano.specs")
set(CMAKE_C_LINK_FLAGS
    "${CMAKE_C_LINK_FLAGS} -Wl,-Map=${CMAKE_PROJECT_NAME}.map -Wl,--gc-sections -Wl,--as-needed"
)
set(CMAKE_C_LINK_FLAGS
    "${CMAKE_C_LINK_FLAGS} -Wl,--start-group -lc -lm -Wl,--end-group")
set(CMAKE_C_LINK_FLAGS "${CMAKE_C_LINK_FLAGS} -Wl,--print-memory-usage")

set(CMAKE_CXX_LINK_FLAGS
    "${CMAKE_C_LINK_FLAGS} -Wl,--start-group -lstdc++ -lsupc++ -Wl,--end-group")

# 添加编译指令 add_compile_options(
# "$<$<CONFIG:Debug>:-Og;-DDEBUG;-g;-funwind-tables>"
# "$<$<CONFIG:Release>:-O3;-DNDEBUG>" "$<$<CONFIG:MinSizeRel>:-Os;-DNDEBUG>"
# "$<$<CONFIG:RelWithDebInfo>:-Os;-g;-DDEBUG>")
