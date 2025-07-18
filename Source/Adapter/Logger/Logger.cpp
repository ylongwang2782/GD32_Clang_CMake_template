#include "Logger.h"

// 静态实例指针
Logger* Logger::s_instance = nullptr;

void Logger::setInstance(Logger* instance) { s_instance = instance; }

// Log类静态方法实现
void Log::t(const char* TAG, const char* format, ...) {
    Logger* instance = Logger::getInstance();
    if (instance) {
        va_list args;
        va_start(args, format);
        instance->log(Logger::Level::TRACE, TAG, format, args);
        va_end(args);
    }
}

void Log::d(const char* TAG, const char* format, ...) {
    Logger* instance = Logger::getInstance();
    if (instance) {
        va_list args;
        va_start(args, format);
        instance->log(Logger::Level::DEBUGL, TAG, format, args);
        va_end(args);
    }
}

void Log::i(const char* TAG, const char* format, ...) {
    Logger* instance = Logger::getInstance();
    if (instance) {
        va_list args;
        va_start(args, format);
        instance->log(Logger::Level::INFO, TAG, format, args);
        va_end(args);
    }
}

void Log::w(const char* TAG, const char* format, ...) {
    Logger* instance = Logger::getInstance();
    if (instance) {
        va_list args;
        va_start(args, format);
        instance->log(Logger::Level::WARN, TAG, format, args);
        va_end(args);
    }
}

void Log::e(const char* TAG, const char* format, ...) {
    Logger* instance = Logger::getInstance();
    if (instance) {
        va_list args;
        va_start(args, format);
        instance->log(Logger::Level::ERROR, TAG, format, args);
        va_end(args);
    }
}

void Log::r(uint8_t* data, size_t size) {
    Logger* instance = Logger::getInstance();
    if (instance) {
        instance->raw(data, size);
    }
}

void Log::setLogLevel(Logger::Level level) {
    Logger* instance = Logger::getInstance();
    if (instance) {
        instance->setLogLevel(level);
    }
}

void Log::setTagLogLevel(const char* tag, Logger::Level level) {
    Logger* instance = Logger::getInstance();
    if (instance) {
        instance->setTagLogLevel(tag, level);
    }
}

Logger::Level Log::getTagLogLevel(const char* tag) {
    Logger* instance = Logger::getInstance();
    if (instance) {
        return instance->getTagLogLevel(tag);
    }
    return Logger::Level::TRACE;    // 默认返回最低级别
}

void Log::clearTagLogLevel(const char* tag) {
    Logger* instance = Logger::getInstance();
    if (instance) {
        instance->clearTagLogLevel(tag);
    }
}

void Log::clearAllTagLogLevels() {
    Logger* instance = Logger::getInstance();
    if (instance) {
        instance->clearAllTagLogLevels();
    }
}

void Log::setColorEnabled(bool enabled) {
    Logger* instance = Logger::getInstance();
    if (instance) {
        instance->setColorEnabled(enabled);
    }
}

bool Log::isColorEnabled() {
    Logger* instance = Logger::getInstance();
    if (instance) {
        return instance->isColorEnabled();
    }
    return false;
}

void Log::setSyncTimestampCallback(Logger::SyncTimestampCallback callback) {
    Logger* instance = Logger::getInstance();
    if (instance) {
        instance->setSyncTimestampCallback(callback);
    }
}

void Log::clearSyncTimestampCallback() {
    Logger* instance = Logger::getInstance();
    if (instance) {
        instance->clearSyncTimestampCallback();
    }
}

// 原有的vAssertCalled函数
void vAssertCalled(const char* file, int line) {
    taskDISABLE_INTERRUPTS();
    printf("Assert failed in file %s at line %d\n", file, line);
    for (;;);
}

#ifdef ARM
extern "C" {
int fputc(int ch, FILE* f) {
    usart_data_transmit(USART1, (uint8_t)ch);
    while (RESET == usart_flag_get(USART1, USART_FLAG_TBE)) {
    }
    return ch;
}
}
#endif

#ifdef GCC
extern "C" {
int _write(int fd, char* pBuffer, int size) {
    for (int i = 0; i < size; i++) {
        while (RESET == usart_flag_get(UART3, USART_FLAG_TBE));
        usart_data_transmit(UART3, (uint8_t)pBuffer[i]);
    }
    return size;
}
}
#endif