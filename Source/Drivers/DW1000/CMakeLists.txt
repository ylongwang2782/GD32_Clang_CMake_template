# 添加静态库 启动文件与启动函数
add_library(DW1000 INTERFACE)

target_sources(
  DW1000
  INTERFACE decadriver/deca_device.c decadriver/deca_params_init.c
            platform/deca_mutex.c platform/deca_range_tables.c
            platform/deca_sleep.cpp platform/deca_spi.c)

target_include_directories(DW1000 INTERFACE decadriver/ platform/)
