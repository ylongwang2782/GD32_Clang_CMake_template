# TaskManager - FreeRTOS任务管理器

## 功能描述

TaskManager是一个用于监控FreeRTOS任务运行状态的管理器，提供以下功能：

1. **启动时任务列表**：系统启动后自动打印所有任务的基本信息
2. **定时运行时统计**：定期打印各任务的CPU使用率和运行时间统计
3. **内存监控**：显示系统堆内存使用情况
4. **手动触发**：提供静态方法支持手动触发统计信息输出

## 使用方法

### 1. 基本使用

```cpp
#include "TaskManager.h"

// 创建任务管理器实例，每10秒打印一次运行时统计
TaskManager taskManager(10000);
```

### 2. 配置选项

```cpp
// 创建任务管理器，禁用定时统计
TaskManager taskManager(0);

// 运行时启用/禁用统计
taskManager.enableRuntimeStatistics(true);

// 设置统计间隔为5秒
taskManager.setStatsInterval(5000);
```

### 3. 手动触发

```cpp
// 手动打印任务列表
TaskManager::printTaskListStatic();

// 手动打印运行时统计
TaskManager::printRuntimeStatsStatic();

// 或者通过实例方法
taskManager.triggerTaskList();
taskManager.triggerRuntimeStats();
```

## 输出示例

### 任务列表输出
```
[INFO] [TaskManager] === Task List ===
[INFO] [TaskManager] Name             State    Priority  Stack HWM
[INFO] [TaskManager] ----------------------------------------
[INFO] [TaskManager] MasterTask       Running         2        234
[INFO] [TaskManager] TaskManager      Blocked         1        456
[INFO] [TaskManager] SlaveDataTransfer Running         4        123
[INFO] [TaskManager] IDLE             Ready           0        234
[INFO] [TaskManager] ----------------------------------------
[INFO] [TaskManager] Free Heap: 45678 bytes
[INFO] [TaskManager] Min Ever Free Heap: 43210 bytes
[INFO] [TaskManager] Total Tasks: 4
[INFO] [TaskManager] ===================
```

### 运行时统计输出
```
[INFO] [TaskManager] === Runtime Statistics ===
[INFO] [TaskManager] Name             Run Time    CPU Usage
[INFO] [TaskManager] ----------------------------------------
[INFO] [TaskManager] MasterTask           1234      12.34%
[INFO] [TaskManager] TaskManager           567       5.67%
[INFO] [TaskManager] SlaveDataTransfer    2345      23.45%
[INFO] [TaskManager] IDLE                 5854      58.54%
[INFO] [TaskManager] ----------------------------------------
[INFO] [TaskManager] Total Runtime: 10000 ticks
[INFO] [TaskManager] Sample Period: 1000 ticks
[INFO] [TaskManager] ===========================
```

## 系统要求

1. **FreeRTOS配置**：
   - `configGENERATE_RUN_TIME_STATS` 必须设置为 1
   - `configUSE_TRACE_FACILITY` 必须设置为 1
   - `configUSE_STATS_FORMATTING_FUNCTIONS` 必须设置为 1

2. **依赖项**：
   - Logger系统
   - FreeRTOScpp封装
   - hal_hptimer（高精度定时器）

## 注意事项

1. **内存使用**：TaskManager会动态分配内存来存储任务状态信息，使用完毕后会自动释放
2. **性能影响**：运行时统计会占用一定的CPU资源，建议统计间隔不要太短
3. **线程安全**：TaskManager使用FreeRTOS的API，是线程安全的
4. **资源优化**：为了节省资源，避免使用字符串流等重型操作，全部使用C风格的格式化输出

## 集成示例

### 主机端集成
```cpp
// master_main.cpp
#include "TaskManager.h"

static void MasterTask(void* pvParameters) {
    // 初始化日志系统
    LogManager::quickInit(LogManager::UART_TYPE_UART7, true, Logger::Level::TRACE);
    
    // 创建任务管理器
    TaskManager taskManager(10000);  // 每10秒打印统计
    
    // 其他初始化...
    MasterServer masterServer;
    masterServer.run();
}
```

### 从机端集成
```cpp
// slave_mode.cpp
#include "TaskManager.h"

static void Slave_Task(void *pvParameters) {
    // 初始化日志系统
    LogManager::quickInit(LogManager::UART_TYPE_UART3, true, Logger::Level::ERROR);
    
    // 创建任务管理器
    TaskManager taskManager(15000);  // 每15秒打印统计
    
    // 其他初始化...
    SlaveDevice slaveDevice;
    slaveDevice.run();
}
``` 