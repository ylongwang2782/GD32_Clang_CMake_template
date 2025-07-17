if(CMAKE_C_COMPILER_ID MATCHES "ARMClang")
  add_custom_command(
    TARGET ${EXECUTABLE_NAME}
    POST_BUILD
    COMMAND ${CMAKE_FROMELF} --i32 $<TARGET_FILE:${EXECUTABLE_NAME}> --output
            $<TARGET_FILE_DIR:${EXECUTABLE_NAME}>/${EXECUTABLE_NAME}.hex
    COMMAND ${CMAKE_FROMELF} --bin $<TARGET_FILE:${EXECUTABLE_NAME}> --output
            $<TARGET_FILE_DIR:${EXECUTABLE_NAME}>/${EXECUTABLE_NAME}.bin
    COMMAND ${CMAKE_FROMELF} --text -z $<TARGET_FILE:${EXECUTABLE_NAME}>)

elseif(CMAKE_C_COMPILER_ID MATCHES "GNU")
  add_custom_command(
    TARGET ${EXECUTABLE_NAME}
    POST_BUILD
    # 生成 Intel HEX 文件（调试 + 烧录都能用）
    COMMAND ${CMAKE_OBJCOPY} -O ihex $<TARGET_FILE:${EXECUTABLE_NAME}>
            $<TARGET_FILE_DIR:${EXECUTABLE_NAME}>/${EXECUTABLE_NAME}.hex
    # 生成精简版 BIN 文件（快速烧录，只包含 Flash 段）
    COMMAND
      ${CMAKE_OBJCOPY} -O binary -j .vectors -j .text -j .rodata -j .ARM.extab
      -j .ARM.exidx -j .init_array -j .fini_array -j .data
      $<TARGET_FILE:${EXECUTABLE_NAME}>
      $<TARGET_FILE_DIR:${EXECUTABLE_NAME}>/${EXECUTABLE_NAME}.bin
    COMMENT "Generating HEX and BIN files")

endif()
