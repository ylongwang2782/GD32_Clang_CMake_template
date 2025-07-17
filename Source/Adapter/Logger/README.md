# 日志系统优化

## 问题描述

原始的日志初始化流程需要多个步骤：

```cpp
// 原始的繁琐初始化流程
UartConfig uart3Conf(uart3_info);
Uart rs232(uart3Conf);
Logger logger(rs232);

Logger::setInstance(&logger);
logger.setColorEnabled(true);
LogTask logTask(logger);
logTask.give();
Log::d(TAG, "LogTask initialized");
```

## 优化方案

新增了 `LogManager` 类来简化日志系统的初始化流程。

### 快速初始化

对于标准的主机和从机配置，只需要一行代码：

```cpp
// 从机快速初始化（使用UART3）
if (!LOG_QUICK_INIT_SLAVE()) {
    // 错误处理
}

// 主机快速初始化（使用UART7）
if (!LOG_QUICK_INIT_MASTER()) {
    // 错误处理
}
```

### 自定义初始化

如果需要使用自定义的UART配置：

```cpp
// 使用自定义UART配置
if (!LogManager::init(custom_uart_info, true, Logger::Level::DEBUG)) {
    // 错误处理
}
```

### 完整示例

优化后的从机初始化代码：

```cpp
#include "LogManager.h"

static void Slave_Task(void *pvParameters) {
    static constexpr const char TAG[] = "BOOT";

    // 优化后的日志初始化 - 只需要一行代码！
    if (!LOG_QUICK_INIT_SLAVE()) {
        // 错误处理
        while(1) {
            vTaskDelay(pdMS_TO_TICKS(1000));
        }
    }
    
    Log::d(TAG, "LogTask initialized");
    
    // 其他初始化代码...
}
```

## API 参考

### LogManager 类

#### 静态方法

- `bool quickInit(UartType uartType, bool enableColor = true, Logger::Level logLevel = Logger::Level::TRACE)`
  - 快速初始化日志系统
  - 参数：
    - `uartType`: UART类型（UART_TYPE_UART3 或 UART_TYPE_UART7）
    - `enableColor`: 是否启用颜色输出
    - `logLevel`: 日志级别
  - 返回值：初始化是否成功

- `bool init(UasrtInfo& uartInfo, bool enableColor = true, Logger::Level logLevel = Logger::Level::TRACE)`
  - 使用自定义UART配置初始化
  - 参数：
    - `uartInfo`: UART配置信息
    - `enableColor`: 是否启用颜色输出
    - `logLevel`: 日志级别
  - 返回值：初始化是否成功

- `Logger* getInstance()`
  - 获取Logger实例指针
  - 返回值：Logger实例指针，未初始化时返回nullptr

- `bool isInitialized()`
  - 检查日志系统是否已初始化
  - 返回值：是否已初始化

- `void cleanup()`
  - 清理日志系统资源

#### 便捷宏

- `LOG_QUICK_INIT_SLAVE()`: 等价于 `LogManager::quickInit(LogManager::UART_TYPE_UART3)`
- `LOG_QUICK_INIT_MASTER()`: 等价于 `LogManager::quickInit(LogManager::UART_TYPE_UART7)`

## 优势

1. **简化初始化**：从8行代码减少到1行代码
2. **减少错误**：自动处理所有必要的初始化步骤
3. **统一管理**：集中管理日志系统的生命周期
4. **易于维护**：所有日志相关的配置集中在一个地方
5. **向后兼容**：不影响现有的Log::d()等静态方法调用

## 注意事项

1. LogManager 使用单例模式，只能初始化一次
2. 如果需要重新初始化，需要先调用 `cleanup()` 方法
3. 初始化失败时会自动清理已分配的资源
4. 建议在系统启动时尽早初始化日志系统 