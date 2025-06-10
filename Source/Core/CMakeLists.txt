add_subdirectory(uwb)
add_subdirectory(dw1000)
target_sources(
  ${EXECUTABLE_NAME}
  PRIVATE ./gd32f4xx_it.c
          ./main.cpp
          ./Enet/netconf.cpp
          ./Enet/enet.cpp
          ./Enet/udp_echo.cpp
          ./Enet/ethernetif.cpp
          ./Enet/sys_arch.c
          )

target_include_directories(${EXECUTABLE_NAME} PUBLIC ./ ./Enet ./include)
