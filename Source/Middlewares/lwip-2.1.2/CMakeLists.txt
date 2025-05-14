# 进入当前目录
cmake_minimum_required(VERSION 3.19)

# 添加 lwIP 源文件（对象库）
add_library(
  lwip_obj OBJECT
  src/core/init.c
  src/core/mem.c
  src/core/memp.c
  src/core/netif.c
  src/core/pbuf.c
  src/core/raw.c
  src/core/stats.c
  src/core/sys.c
  src/core/tcp.c
  src/core/tcp_in.c
  src/core/tcp_out.c
  src/core/timeouts.c
  src/core/udp.c
  src/api/api_msg.c
  src/api/api_lib.c
  src/api/err.c
  src/api/netbuf.c
  src/api/netdb.c
  src/api/netifapi.c
  src/api/sockets.c
  src/api/tcpip.c
  src/netif/ethernet.c)

# 包含路径
target_include_directories(
  lwip_obj PUBLIC ./src/include ./src/include/netif ./src/include/lwip
                     ./src/include/ipv4 ./port/GD32F4xx)
