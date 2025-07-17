#pragma once
#ifndef _LOG_MANAGER_H_
#define _LOG_MANAGER_H_

#include "Logger.h"
#include "hal_uart.hpp"
#include <memory>

class LogManager {
public:
    enum UartType {
        UART_TYPE_UART3,
        UART_TYPE_UART7
    };

    static bool quickInit(UartType uartType, 
                         bool enableColor = true, 
                         Logger::Level logLevel = Logger::Level::TRACE);

    static bool init(UasrtInfo& uartInfo, 
                    bool enableColor = true, 
                    Logger::Level logLevel = Logger::Level::TRACE);

    static Logger* getInstance();

    static bool isInitialized();

    static void cleanup();

private:
    static std::unique_ptr<UartConfig> s_uartConfig;
    static std::unique_ptr<Uart> s_uart;
    static std::unique_ptr<Logger> s_logger;
    static std::unique_ptr<LogTask> s_logTask;
    static bool s_initialized;

    LogManager() = delete;
    LogManager(const LogManager&) = delete;
    LogManager& operator=(const LogManager&) = delete;
};

#define LOG_QUICK_INIT_SLAVE()   LogManager::quickInit(LogManager::UART_TYPE_UART3)
#define LOG_QUICK_INIT_MASTER()  LogManager::quickInit(LogManager::UART_TYPE_UART7)

#endif 