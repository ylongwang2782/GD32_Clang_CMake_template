#ifndef _LOG_H_
#define _LOG_H_

#include <stdarg.h>
// #include "printf.h"
#include "bsp_uart.h"
extern "C" {

#include "FreeRTOS.h"
#include "queue.h"
#include "task.h"
}

#define LOG_QUEUE_SIZE 50

#define LOGF(format, ...)  (void)printf("[LOG]: " format, ##__VA_ARGS__)
#define INFOF(format, ...) (void)printf("[INFO]: " format, ##__VA_ARGS__)
#define WARNF(format, ...) (void)printf("[WARN]: " format, ##__VA_ARGS__)
#define ERRF(format, ...)  (void)printf("[ERR]: " format, ##__VA_ARGS__)

enum class LogLevel { DEBUGL, INFO, WARN, ERROR };

class Logger {
   public:
    void log(LogLevel level, const char *format, ...);
    QueueHandle_t logQueue;

   private:
    void output(LogLevel level, const char *message);
};

#endif
