target_sources(
  ${EXECUTABLE_NAME}
  PRIVATE ./gd32f4xx_it.c
          ./main.cpp
          ./Enet/netconf.cpp
          ./Enet/gd32f4xx_enet_eval.cpp
          ./Enet/hello_gigadevice.cpp
          ./Enet/udp_echo.cpp
          ./Enet/tcp_client.cpp
          ./Enet/ethernetif.c
          ./Enet/sys_arch.c
          )

target_include_directories(${EXECUTABLE_NAME} PUBLIC ./ ./Enet ./include)
