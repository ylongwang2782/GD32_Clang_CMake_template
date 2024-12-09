#ifndef _LOG_H_
#define _LOG_H_

#include <stdarg.h>
// #include "printf.h"
#include "bsp_uart.h"

// #define printf(format, ...) printf_(format, ##__VA_ARGS__)
#define LOGF(format, ...) (void)printf("[LOG]: " format, ##__VA_ARGS__)
#define INFOF(format, ...) (void)printf("[INFO]: " format, ##__VA_ARGS__)
#define DBGF(format, ...) (void)printf("[DEBUG]: " format, ##__VA_ARGS__)
#define WARNF(format, ...) (void)printf("[WARN]: " format, ##__VA_ARGS__)
#define ERRF(format, ...) (void)printf("[ERR]: " format, ##__VA_ARGS__)

#endif
