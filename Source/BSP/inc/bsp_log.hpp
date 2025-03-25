#ifndef _LOG_H_
#define _LOG_H_

#include <stdarg.h>

#include <array>

#include "QueueCPP.h"
#include "bsp_uart.hpp"


extern "C" {
#include "FreeRTOS.h"
#include "queue.h"
#include "task.h"
}

#define USART_LOG      USART1
#define LOG_QUEUE_SIZE 128

// 定义日志消息的最大长度
#define LOG_MESSAGE_MAX_LENGTH 128

// 定义日志队列的长度
#define LOG_QUEUE_LENGTH 10

// 日志消息结构体
struct LogMessage {
    std::array<char, LOG_MESSAGE_MAX_LENGTH> message;
};

// 日志级别枚举
enum class Level { VERBOSE, DEBUGL, INFO, WARN, ERROR, RAW };

class Logger {
   public:
    Logger(Uart &uart) : uart(uart), logQueue("LogQueue") {}

    Uart &uart;    // 串口对象的引用
    FreeRTOScpp::Queue<LogMessage, LOG_QUEUE_LENGTH> logQueue;

    void log(Level level, const char *format, va_list args) {
        // 定义日志级别的字符串表示
        static const char *levelStr[] = {"VERBOSE", "DEBUG", "INFO", "WARN",
                                         "ERROR"};

        // 缓冲区大小
        constexpr size_t bufferSize = LOG_QUEUE_SIZE;

        // 缓存区，用于存储格式化后的日志内容
        char buffer[bufferSize];

        // 格式化日志内容
        vsnprintf(buffer, sizeof(buffer), format, args);

        if (level != Level::RAW) {
            // 添加级别前缀
            char finalMessage[bufferSize + 8];
            snprintf(finalMessage, sizeof(finalMessage), "[%s]:%s\n",
                     levelStr[static_cast<int>(level)], buffer);
            // 输出日志
            output(level, finalMessage);
        } else {
            char finalMessage[bufferSize];
            snprintf(finalMessage, sizeof(finalMessage), "%s\n", buffer);
            // 输出日志
            output(level, finalMessage);
        }
    }
    void v(const char *format, ...) {
        va_list args;
        va_start(args, format);

        log(Level::VERBOSE, format, args);

        va_end(args);
    }
    void d(const char *format, ...) {
        va_list args;
        va_start(args, format);

        log(Level::DEBUGL, format, args);

        va_end(args);
    }
    void i(const char *format, ...) {
        va_list args;
        va_start(args, format);

        log(Level::INFO, format, args);

        va_end(args);
    }
    void w(const char *format, ...) {
        va_list args;
        va_start(args, format);

        log(Level::WARN, format, args);

        va_end(args);
    }
    void e(const char *format, ...) {
        va_list args;
        va_start(args, format);

        log(Level::ERROR, format, args);

        va_end(args);
    }

   private:
    void output(Level level, const char *message) {
        LogMessage logMsg;
        std::strncpy(logMsg.message.data(), message,
                     LOG_MESSAGE_MAX_LENGTH - 1);
        logMsg.message[LOG_MESSAGE_MAX_LENGTH - 1] = '\0';

        // 将日志消息放入队列
        if (!logQueue.add(logMsg, portMAX_DELAY)) {
            printf("Failed to add log message to queue!\n");
        }
    }
};

#endif
