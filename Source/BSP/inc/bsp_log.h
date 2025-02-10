#ifndef _LOG_H_
#define _LOG_H_

#include <stdarg.h>

#include "bsp_uart.h"

extern "C" {
#include "FreeRTOS.h"
#include "queue.h"
#include "task.h"
}

#define USART_LOG      USART1
#define LOG_QUEUE_SIZE 64
enum class LogLevel { VERBOSE, DEBUGL, INFO, WARN, ERROR };

class Logger {
   public:
    void log(LogLevel level, const char *format, va_list args) {
        // 定义日志级别的字符串表示
        static const char *levelStr[] = {"VERBOSE", "DEBUG", "INFO", "WARN",
                                         "ERROR"};

        // 缓冲区大小
        constexpr size_t bufferSize = LOG_QUEUE_SIZE;

        // 缓存区，用于存储格式化后的日志内容
        char buffer[bufferSize];

        // 格式化日志内容
        vsnprintf(buffer, sizeof(buffer), format, args);

        // 添加级别前缀
        char finalMessage[bufferSize + 8];
        snprintf(finalMessage, sizeof(finalMessage), "[%s]:%s\n",
                 levelStr[static_cast<int>(level)], buffer);

        // 输出日志
        output(level, finalMessage);
    }
    void v(const char *format, ...) {
        va_list args;
        va_start(args, format);

        log(LogLevel::VERBOSE, format, args);

        va_end(args);
    }
    void d(const char *format, ...) {
        va_list args;
        va_start(args, format);

        log(LogLevel::DEBUGL, format, args);

        va_end(args);
    }
    void i(const char *format, ...) {
        va_list args;
        va_start(args, format);

        log(LogLevel::INFO, format, args);

        va_end(args);
    }
    void w(const char *format, ...) {
        va_list args;
        va_start(args, format);

        log(LogLevel::WARN, format, args);

        va_end(args);
    }
    void e(const char *format, ...) {
        va_list args;
        va_start(args, format);

        log(LogLevel::ERROR, format, args);

        va_end(args);
    }

    QueueHandle_t logQueue;

   private:
    void output(LogLevel level, const char *message) {
        // 将日志消息添加到队列中
        xQueueSend(logQueue, message, portMAX_DELAY);
    }
};

#endif
