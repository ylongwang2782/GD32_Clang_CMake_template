set(COMMON_SOURCES ./gd32f4xx_it.c ./main.cpp syscalls.c freertos.c)
target_compile_definitions(${EXECUTABLE_NAME} PRIVATE USE_CX310)

if(BUILD_VARIANT STREQUAL "MASTER_DEBUG" OR BUILD_VARIANT STREQUAL
                                            "MASTER_RELEASE")

  add_subdirectory(master_main)
  target_sources(${EXECUTABLE_NAME} PRIVATE ${COMMON_SOURCES})

elseif(BUILD_VARIANT STREQUAL "SLAVE_DEBUG" OR BUILD_VARIANT STREQUAL
                                               "SLAVE_RELEASE")
  add_subdirectory(slave_main)
  target_sources(${EXECUTABLE_NAME} PRIVATE ${COMMON_SOURCES})

elseif(BUILD_VARIANT STREQUAL "MASTER_BOARDTEST")
  target_sources(${EXECUTABLE_NAME} PRIVATE ${COMMON_SOURCES}
                                            ./board_test/master/board_test.cpp)
  target_include_directories(${EXECUTABLE_NAME} PUBLIC . ./board_test/master)
elseif(BUILD_VARIANT STREQUAL "SLAVE_BOARDTEST")
  target_sources(
    ${EXECUTABLE_NAME}
    PRIVATE ${COMMON_SOURCES} ./board_test/slave/board_test.cpp
            ./board_test/slave/peripherals.cpp)
  target_include_directories(${EXECUTABLE_NAME} PUBLIC . ./board_test/slave)
endif()
