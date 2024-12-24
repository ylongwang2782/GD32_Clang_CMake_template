#include "bsp_log.h"

void vAssertCalled(const char *file, int line) {
    taskDISABLE_INTERRUPTS();
    printf("Assert failed in file %s at line %d\n", file, line);
    for (;;);
}

void Logger::log(LogLevel level, const char *format, ...) {
    // 定义日志级别的字符串表示
    static const char *levelStr[] = {"VERBOSE", "DEBUG", "INFO", "WARN",
                                     "ERROR"};

    // 缓冲区大小
    constexpr size_t bufferSize = LOG_QUEUE_SIZE;

    // 缓存区，用于存储格式化后的日志内容
    char buffer[bufferSize];

    // 格式化日志内容
    va_list args;
    va_start(args, format);
    vsnprintf(buffer, sizeof(buffer), format, args);
    va_end(args);

    // 添加级别前缀
    char finalMessage[bufferSize + 8];
    snprintf(finalMessage, sizeof(finalMessage), "[%s]:%s\n",
             levelStr[static_cast<int>(level)], buffer);

    // 输出日志
    output(level, finalMessage);
}

void Logger::v(const char *format, ...) { log(LogLevel::VERBOSE, format); }

void Logger::d(const char *format, ...) { log(LogLevel::DEBUGL, format); }

void Logger::i(const char *format, ...) { log(LogLevel::INFO, format); }

void Logger::w(const char *format, ...) { log(LogLevel::WARN, format); }

void Logger::e(const char *format, ...) { log(LogLevel::ERROR, format); }

void Logger::output(LogLevel level, const char *message) {
    // 将日志消息添加到队列中
    xQueueSend(logQueue, message, portMAX_DELAY);
}

#ifdef ARM
extern "C" {
int fputc(int ch, FILE *f) {
    usart_data_transmit(USART1, (uint8_t)ch);
    while (RESET == usart_flag_get(USART1, USART_FLAG_TBE)) {
    }
    return ch;
}
}
#endif

#ifdef GCC
extern "C" {
int _write(int fd, char *pBuffer, int size) {
    for (int i = 0; i < size; i++) {
        while (RESET == usart_flag_get(USART1, USART_FLAG_TBE));
        usart_data_transmit(USART1, (uint8_t)pBuffer[i]);
    }
    return size;
}
}
#endif