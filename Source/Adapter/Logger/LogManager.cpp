#include "LogManager.h"

// 静态成员变量定义
std::unique_ptr<UartConfig> LogManager::s_uartConfig = nullptr;
std::unique_ptr<Uart> LogManager::s_uart = nullptr;
std::unique_ptr<Logger> LogManager::s_logger = nullptr;
std::unique_ptr<LogTask> LogManager::s_logTask = nullptr;
bool LogManager::s_initialized = false;

bool LogManager::quickInit(UartType uartType, bool enableColor, Logger::Level logLevel) {
    if (s_initialized) {
        return true; // 已经初始化
    }

    UasrtInfo* uartInfo = nullptr;
    
    switch (uartType) {
        case UART_TYPE_UART3:
            uartInfo = &uart3_info;
            break;
        case UART_TYPE_UART7:
            uartInfo = &uart7_info;
            break;
        default:
            return false;
    }

    return init(*uartInfo, enableColor, logLevel);
}

bool LogManager::init(UasrtInfo& uartInfo, bool enableColor, Logger::Level logLevel) {
    if (s_initialized) {
        return true; // 已经初始化
    }

    // 创建UART配置和实例
    s_uartConfig = std::make_unique<UartConfig>(uartInfo);
    if (!s_uartConfig) {
        cleanup();
        return false;
    }
    
    s_uart = std::make_unique<Uart>(*s_uartConfig);
    if (!s_uart) {
        cleanup();
        return false;
    }
    
    // 创建Logger实例
    s_logger = std::make_unique<Logger>(*s_uart);
    if (!s_logger) {
        cleanup();
        return false;
    }
    
    // 设置全局实例
    Logger::setInstance(s_logger.get());
    
    // 配置Logger
    s_logger->setColorEnabled(enableColor);
    s_logger->setLogLevel(logLevel);
    
    // 创建并启动日志任务
    s_logTask = std::make_unique<LogTask>(*s_logger);
    if (!s_logTask) {
        cleanup();
        return false;
    }
    
    s_logTask->give();
    
    s_initialized = true;
    return true;
}

Logger* LogManager::getInstance() {
    return s_logger.get();
}

bool LogManager::isInitialized() {
    return s_initialized;
}

void LogManager::cleanup() {
    s_logTask.reset();
    s_logger.reset();
    s_uart.reset();
    s_uartConfig.reset();
    s_initialized = false;
    
    // 清除全局实例
    Logger::setInstance(nullptr);
} 