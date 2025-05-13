# lwip-2.1.2/CMakeLists.txt

cmake_minimum_required(VERSION 3.13)

add_library(lwip INTERFACE)

set(LWIP_SRC
    ./src/api/api_lib.c
    ./src/api/api_msg.c
    ./src/api/err.c
    ./src/api/if_api.c
    ./src/api/netbuf.c
    ./src/api/netdb.c
    ./src/api/netifapi.c
    ./src/api/sockets.c
    ./src/api/tcpip.c
    ./src/core/altcp.c
    ./src/core/altcp_alloc.c
    ./src/core/altcp_tcp.c
    ./src/core/def.c
    ./src/core/dns.c
    ./src/core/inet_chksum.c
    ./src/core/init.c
    ./src/core/ip.c
    ./src/core/mem.c
    ./src/core/memp.c
    ./src/core/netif.c
    ./src/core/pbuf.c
    ./src/core/raw.c
    ./src/core/stats.c
    ./src/core/sys.c
    ./src/core/tcp.c
    ./src/core/tcp_in.c
    ./src/core/tcp_out.c
    ./src/core/timeouts.c
    ./src/core/udp.c
    ./src/core/ipv4/autoip.c
    ./src/core/ipv4/dhcp.c
    ./src/core/ipv4/etharp.c
    ./src/core/ipv4/icmp.c
    ./src/core/ipv4/igmp.c
    ./src/core/ipv4/ip4.c
    ./src/core/ipv4/ip4_addr.c
    ./src/core/ipv4/ip4_frag.c
    ./src/netif/ethernet.c
    ./port/GD32F4xx/FreeRTOS/ethernetif.c
    ./port/GD32F4xx/FreeRTOS/sys_arch.c)

# 编译成静态库
target_sources(lwip INTERFACE ${LWIP_SRC})

# 包含路径
target_include_directories(
  lwip INTERFACE ./src/include ./src/include/netif ./src/include/lwip
              ./src/include/ipv4 ./port/GD32F4xx/ ./port/GD32F4xx/FreeRTOS)
