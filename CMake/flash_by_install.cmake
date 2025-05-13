# 定义自定义构建目标：下载到目标单片机
set(OPENOCD_EXECUTABLE "openocd")
set(PYOCD_EXECUTABLE "pyocd")

install(
  CODE CODE
  "MESSAGE(\"Flash......\")"
  CODE "execute_process(COMMAND ${OPENOCD_EXECUTABLE} -f ${PROJECT_SOURCE_DIR}/Scripts/OpenOCD/openocd_daplink.cfg -c \"program ${EXECUTABLE_OUTPUT_PATH}/${EXECUTABLE_NAME}.elf verify reset\" -c shutdown)"
)

# install(CODE CODE "MESSAGE(\"Flash via the following command:\")" CODE
# "MESSAGE(\"${PYOCD_EXECUTABLE} flash -q --erase sector --target STM32F429VETx
# --pack ${PROJECT_SOURCE_DIR}/Utilities/STM32F429VETx_v2.17.1.pack
# ${EXECUTABLE_OUTPUT_PATH}/${EXECUTABLE_NAME}.elf\")" CODE
# "execute_process(COMMAND ${PYOCD_EXECUTABLE} flash -q --erase sector --target
# STM32F429VETx --pack
# ${PROJECT_SOURCE_DIR}/Utilities/STM32F429VETx_v2.17.1.pack
# ${EXECUTABLE_OUTPUT_PATH}/${EXECUTABLE_NAME}.elf  )" )
