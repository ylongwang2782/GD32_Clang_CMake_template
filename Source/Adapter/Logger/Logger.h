#pragma once
#ifndef _LOGGER_H_
#define _LOGGER_H_

#include <stdarg.h>

#include <array>
#include <map>
#include <string>
#include <functional>

#include "QueueCPP.h"
#include "TaskCPP.h"
#include "hal_hptimer.hpp"
#include "hal_uart.hpp"
#include "queue.h"
#include "task.h"


#define LOG_TASK_DEPTH_SIZE 512
#define LOG_TASK_PRIO       TaskPrio_Highest

// 定义日志消息的最大长度
#define LOG_QUEUE_SIZE 256
// 定义日志队列的长度
#define LOG_QUEUE_LENGTH 20

// ANSI颜色代码定义
#define ANSI_COLOR_RESET   "\033[0m"
#define ANSI_COLOR_BLACK   "\033[30m"
#define ANSI_COLOR_RED     "\033[31m"
#define ANSI_COLOR_GREEN   "\033[32m"
#define ANSI_COLOR_YELLOW  "\033[33m"
#define ANSI_COLOR_BLUE    "\033[34m"
#define ANSI_COLOR_MAGENTA "\033[35m"
#define ANSI_COLOR_CYAN    "\033[36m"
#define ANSI_COLOR_WHITE   "\033[37m"

// 高亮颜色
#define ANSI_COLOR_BRIGHT_BLACK   "\033[90m"
#define ANSI_COLOR_BRIGHT_RED     "\033[91m"
#define ANSI_COLOR_BRIGHT_GREEN   "\033[92m"
#define ANSI_COLOR_BRIGHT_YELLOW  "\033[93m"
#define ANSI_COLOR_BRIGHT_BLUE    "\033[94m"
#define ANSI_COLOR_BRIGHT_MAGENTA "\033[95m"
#define ANSI_COLOR_BRIGHT_CYAN    "\033[96m"
#define ANSI_COLOR_BRIGHT_WHITE   "\033[97m"

// 日志消息结构体
struct LogMessage {
    std::array<char, LOG_QUEUE_SIZE> message;
};

// 日志级别枚举
class Logger {
   public:
    enum class Level { RAW, TRACE, DEBUGL, INFO, WARN, ERROR };

    // 同步时间戳回调函数类型
    using SyncTimestampCallback = std::function<uint64_t()>;

    Logger(Uart& uart) : uart(uart), logQueue("LogQueue"), colorEnabled(true), syncTimestampCallback(nullptr) {}

    Uart& uart;    // 串口对象的引用
    FreeRTOScpp::Queue<LogMessage, LOG_QUEUE_LENGTH> logQueue;

    Level currentLevel = Level::RAW;
    bool colorEnabled;                         // 颜色输出控制
    std::map<std::string, Level> tagLevels;    // 存储每个标签的日志等级
    SyncTimestampCallback syncTimestampCallback;  // 同步时间戳回调函数

    void setLogLevel(Level level) { currentLevel = level; }

    // 设置同步时间戳回调函数
    void setSyncTimestampCallback(SyncTimestampCallback callback) {
        syncTimestampCallback = callback;
    }

    // 清除同步时间戳回调函数
    void clearSyncTimestampCallback() {
        syncTimestampCallback = nullptr;
    }

    // 设置特定标签的日志等级
    void setTagLogLevel(const char* tag, Level level) {
        tagLevels[std::string(tag)] = level;
    }

    // 获取特定标签的日志等级
    Level getTagLogLevel(const char* tag) const {
        auto it = tagLevels.find(std::string(tag));
        if (it != tagLevels.end()) {
            return it->second;
        }
        return currentLevel;    // 如果没有设置特定标签级别，返回全局级别
    }

    // 清除特定标签的日志等级设置
    void clearTagLogLevel(const char* tag) {
        tagLevels.erase(std::string(tag));
    }

    // 清除所有标签的日志等级设置
    void clearAllTagLogLevels() { tagLevels.clear(); }

    // 检查是否应该输出日志
    bool shouldLog(Level level, const char* tag) const {
        Level tagLevel = getTagLogLevel(tag);
        return level >= tagLevel;
    }

    void setColorEnabled(bool enabled) { colorEnabled = enabled; }

    bool isColorEnabled() const { return colorEnabled; }

    // 获取日志级别对应的颜色代码
    const char* getLevelColor(Level level) const {
        if (!colorEnabled) return "";

        switch (level) {
            case Level::TRACE:
                return ANSI_COLOR_BRIGHT_BLACK;    // 灰色
            case Level::DEBUGL:
                return ANSI_COLOR_CYAN;    // 青色
            case Level::INFO:
                return ANSI_COLOR_GREEN;    // 绿色
            case Level::WARN:
                return ANSI_COLOR_YELLOW;    // 黄色
            case Level::ERROR:
                return ANSI_COLOR_RED;    // 红色
            case Level::RAW:
                return ANSI_COLOR_MAGENTA;    // 紫色
            default:
                return "";
        }
    }

    void log(Level level, const char* TAG, const char* format, va_list args) {
        // 检查是否应该输出这个标签的日志
        if (!shouldLog(level, TAG)) return;

        // 定义日志级别的字符串表示
        static const char* levelStr[] = {"RAW",  "TRACE", "DEBUG",
                                         "INFO", "WARN",  "ERROR"};

        // 缓冲区大小
        constexpr size_t bufferSize = LOG_QUEUE_SIZE;

        // 缓存区，用于存储格式化后的日志内容
        char buffer[bufferSize];

        // 格式化日志内容
        vsnprintf(buffer, sizeof(buffer), format, args);

        // 获取时间戳 - 优先使用同步时间戳
        uint64_t timestampUs;
        if (syncTimestampCallback) {
            timestampUs = syncTimestampCallback();
        } else {
            timestampUs = hal_hptimer_get_us();
        }
        
        // 转换为秒和毫秒
        uint32_t seconds = timestampUs / 1000000;
        uint32_t milliseconds = (timestampUs % 1000000) / 1000;

        // 获取颜色代码
        const char* colorCode = getLevelColor(level);
        const char* resetCode = colorEnabled ? ANSI_COLOR_RESET : "";

        // 添加时间戳和级别前缀，包含颜色
        char finalMessage[bufferSize + 64];    // 增加缓冲区大小以容纳颜色代码
        if (colorEnabled) {
            snprintf(finalMessage, sizeof(finalMessage),
                     "%s[%lu.%03lu] [%s] [%s] %s%s\r\n", colorCode, 
                     (unsigned long)seconds, (unsigned long)milliseconds,
                     levelStr[static_cast<int>(level)], TAG,
                     buffer, resetCode);
        } else {
            snprintf(finalMessage, sizeof(finalMessage),
                     "[%lu.%03lu] [%s] [%s] %s\r\n", 
                     (unsigned long)seconds, (unsigned long)milliseconds,
                     levelStr[static_cast<int>(level)], TAG, buffer);
        }

        // 输出日志
        output(level, finalMessage);
    }

    void verbose(const char* TAG, const char* format, ...) {
        if (!shouldLog(Level::TRACE, TAG)) return;
        va_list args;
        va_start(args, format);
        log(Level::TRACE, TAG, format, args);
        va_end(args);
    }

    void debug(const char* TAG, const char* format, ...) {
        if (!shouldLog(Level::DEBUGL, TAG)) return;
        va_list args;
        va_start(args, format);
        log(Level::DEBUGL, TAG, format, args);
        va_end(args);
    }

    void info(const char* TAG, const char* format, ...) {
        if (!shouldLog(Level::INFO, TAG)) return;
        va_list args;
        va_start(args, format);
        log(Level::INFO, TAG, format, args);
        va_end(args);
    }

    void warn(const char* TAG, const char* format, ...) {
        if (!shouldLog(Level::WARN, TAG)) return;
        va_list args;
        va_start(args, format);
        log(Level::WARN, TAG, format, args);
        va_end(args);
    }

    void error(const char* TAG, const char* format, ...) {
        if (!shouldLog(Level::ERROR, TAG)) return;
        va_list args;
        va_start(args, format);
        log(Level::ERROR, TAG, format, args);
        va_end(args);
    }

    void raw(uint8_t* data, size_t size) {
        const char* TAG = "";
        if (!shouldLog(Level::RAW, TAG)) return;

        // 每个字节需要两个字符+一个空格，最后一个字节不需要空格
        char buffer[size * 3];
        for (size_t i = 0; i < size; i++) {
            // 最后一个字节不加空格
            if (i == size - 1) {
                snprintf(buffer + i * 3, 3, "%02X", data[i]);
            } else {
                snprintf(buffer + i * 3, 4, "%02X ", data[i]);
            }
        }
        buffer[size * 3 - 1] = '\0';    // 添加字符串终止符
        va_list emptyArgs;
        log(Level::RAW, TAG, buffer, emptyArgs);
    }

    // 静态方法接口
    static void setInstance(Logger* instance);
    static Logger* getInstance() { return s_instance; }

   private:
    void output(Level level, const char* message) {
        LogMessage logMsg;
        std::strncpy(logMsg.message.data(), message, LOG_QUEUE_SIZE - 1);
        logMsg.message[LOG_QUEUE_SIZE - 1] = '\0';

        // 将日志消息放入队列
        if (!logQueue.add(logMsg, portMAX_DELAY)) {
            printf("Failed to add log message to queue!\n");
        }
    }

    static Logger* s_instance;
};

// Log类作为静态接口的别名
class Log {
   public:
    static void t(const char* TAG, const char* format, ...);
    static void d(const char* TAG, const char* format, ...);
    static void i(const char* TAG, const char* format, ...);
    static void w(const char* TAG, const char* format, ...);
    static void e(const char* TAG, const char* format, ...);
    static void r(uint8_t* data, size_t size);
    static void setLogLevel(Logger::Level level);
    static void setTagLogLevel(const char* tag, Logger::Level level);
    static Logger::Level getTagLogLevel(const char* tag);
    static void clearTagLogLevel(const char* tag);
    static void clearAllTagLogLevels();
    static void setColorEnabled(bool enabled);
    static bool isColorEnabled();
    static void setSyncTimestampCallback(Logger::SyncTimestampCallback callback);
    static void clearSyncTimestampCallback();
};

class LogTask : public TaskClassS<LOG_TASK_DEPTH_SIZE> {
   private:
    Logger& Log;

   public:
    LogTask(Logger& Log)
        : TaskClassS<LOG_TASK_DEPTH_SIZE>("LogTask", LOG_TASK_PRIO), Log(Log) {}

    void task() override {
        char buffer[LOG_QUEUE_SIZE + 8];
        for (;;) {
            LogMessage logMsg;
            // 从队列中获取日志消息
            if (Log.logQueue.pop(logMsg, portMAX_DELAY)) {
                Log.uart.data_send(
                    reinterpret_cast<const uint8_t*>(logMsg.message.data()),
                    strlen(logMsg.message.data()));
            }
        }
    }
};

#endif