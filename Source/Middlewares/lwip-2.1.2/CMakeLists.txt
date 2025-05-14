# 进入当前目录
cmake_minimum_required(VERSION 3.19)

# 添加 lwIP 源文件（对象库）
add_library(
  lwip_obj OBJECT

   src/api/api_lib.c
   src/api/api_msg.c
   src/api/err.c
   src/api/if_api.c
   src/api/netbuf.c
   src/api/netdb.c
   src/api/netifapi.c
   src/api/sockets.c
   src/api/tcpip.c

   src/core/ipv4/autoip.c
   src/core/ipv4/dhcp.c
   src/core/ipv4/etharp.c
   src/core/ipv4/icmp.c
   src/core/ipv4/igmp.c
   src/core/ipv4/ip4.c
   src/core/ipv4/ip4_addr.c
   src/core/ipv4/ip4_frag.c

   src/core/altcp.c
   src/core/altcp_alloc.c
   src/core/altcp_tcp.c

   src/core/def.c
   src/core/dns.c
   src/core/inet_chksum.c
   src/core/init.c
   src/core/ip.c
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

  src/netif/ethernet.c)

target_include_directories(
  lwip_obj PUBLIC ./src/include ./src/include/netif ./src/include/lwip
                     ./src/include/ipv4 ./port/GD32F4xx)
