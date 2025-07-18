#include "TaskManager.hpp"
#include "hal_hptimer.hpp"
#include <cstdio>
#include <cstring>

// 静态成员定义
const char* TaskManager::TAG = "TaskManager";
TaskManager* TaskManager::instance = nullptr;

// 用于FreeRTOS运行时统计的计时器
static uint32_t runTimeStatsTimer = 0;

void configureTimerForRunTimeStats(void) {
    // 初始化运行时统计计时器
    runTimeStatsTimer = 0;
}

unsigned long getRunTimeCounterValue(void) {
    // 使用高精度定时器获取当前时间
    // 为了避免溢出，使用系统时钟的1/10作为运行时统计时钟
    return hal_hptimer_get_ms() / 10;
}

TaskManager::TaskManager(uint32_t statsIntervalMs) 
    : TaskClassS<0>("TaskManager", TaskPrio_Low, TASK_MANAGER_TASK_DEPTH_SIZE)
    , statsInterval(statsIntervalMs)
    , lastStatsTime(0)
    , enableRuntimeStats(true)
    , totalRunTime(0)
    , lastTotalRunTime(0)
{
    instance = this;
    
    // 如果调度器已经运行，需要手动启动任务
    if (xTaskGetSchedulerState() == taskSCHEDULER_RUNNING) {
        this->give();
    }
}

TaskManager::~TaskManager() {
    if (instance == this) {
        instance = nullptr;
    }
}

TaskManager* TaskManager::getInstance() {
    return instance;
}

void TaskManager::setStatsInterval(uint32_t intervalMs) {
    statsInterval = intervalMs;
}

void TaskManager::enableRuntimeStatistics(bool enable) {
    enableRuntimeStats = enable;
}

void TaskManager::triggerTaskList() {
    printTaskList();
}

void TaskManager::triggerRuntimeStats() {
    if (enableRuntimeStats) {
        printRuntimeStats();
    }
}

const char* TaskManager::getTaskStateString(eTaskState state) {
    switch (state) {
        case eReady:        return "Ready";
        case eRunning:      return "Running";
        case eBlocked:      return "Blocked";
        case eSuspended:    return "Suspended";
        case eDeleted:      return "Deleted";
        case eInvalid:      return "Invalid";
        default:            return "Unknown";
    }
}

void TaskManager::printTaskList() {
    Log::w(TAG, "=== Task List ===");
    Log::w(TAG, "Name             State    Priority  Stack HWM");
    Log::w(TAG, "----------------------------------------");
    
    UBaseType_t taskCount = uxTaskGetNumberOfTasks();
    TaskStatus_t* taskStatusArray = new TaskStatus_t[taskCount];
    
    if (taskStatusArray == nullptr) {
        Log::e(TAG, "Failed to allocate memory for task status array");
        return;
    }
    
    // 获取任务状态信息
    UBaseType_t arraySize = uxTaskGetSystemState(taskStatusArray, taskCount, nullptr);
    
    for (UBaseType_t i = 0; i < arraySize; i++) {
        TaskStatus_t* task = &taskStatusArray[i];
        
        // 格式化输出任务信息
        char taskName[configMAX_TASK_NAME_LEN + 1];
        strncpy(taskName, task->pcTaskName, configMAX_TASK_NAME_LEN);
        taskName[configMAX_TASK_NAME_LEN] = '\0';
        
        Log::w(TAG, "%-16s %-8s %8u %10u", 
               taskName,
               getTaskStateString(task->eCurrentState),
               static_cast<unsigned int>(task->uxCurrentPriority),
               static_cast<unsigned int>(task->usStackHighWaterMark));
    }
    
    delete[] taskStatusArray;
    
    // 打印系统堆信息
    size_t freeHeap = xPortGetFreeHeapSize();
    size_t minEverFreeHeap = xPortGetMinimumEverFreeHeapSize();
    
    Log::w(TAG, "----------------------------------------");
    Log::w(TAG, "Free Heap: %u bytes", static_cast<unsigned int>(freeHeap));
    Log::w(TAG, "Min Ever Free Heap: %u bytes", static_cast<unsigned int>(minEverFreeHeap));
    Log::w(TAG, "Total Tasks: %u", static_cast<unsigned int>(taskCount));
    Log::w(TAG, "===================");
}

void TaskManager::printRuntimeStats() {
    if (!enableRuntimeStats) {
        return;
    }
    
    Log::w(TAG, "=== Runtime Statistics ===");
    Log::w(TAG, "Name             Run Time    CPU Usage   Stack HWM");
    Log::w(TAG, "-------------------------------------------------");
    
    UBaseType_t taskCount = uxTaskGetNumberOfTasks();
    TaskStatus_t* taskStatusArray = new TaskStatus_t[taskCount];
    
    if (taskStatusArray == nullptr) {
        Log::e(TAG, "Failed to allocate memory for task status array");
        return;
    }
    
    uint32_t totalRunTime = 0;
    
    // 获取任务状态信息，包括运行时间
    UBaseType_t arraySize = uxTaskGetSystemState(taskStatusArray, taskCount, &totalRunTime);
    
    if (totalRunTime == 0) {
        Log::w(TAG, "Runtime statistics not available");
        delete[] taskStatusArray;
        return;
    }
    
    // 计算时间差
    uint32_t timeDiff = totalRunTime - lastTotalRunTime;
    lastTotalRunTime = totalRunTime;
    
    for (UBaseType_t i = 0; i < arraySize; i++) {
        TaskStatus_t* task = &taskStatusArray[i];
        
        char taskName[configMAX_TASK_NAME_LEN + 1];
        strncpy(taskName, task->pcTaskName, configMAX_TASK_NAME_LEN);
        taskName[configMAX_TASK_NAME_LEN] = '\0';
        
        // 计算CPU使用率
        char cpuUsage[16];
        if (totalRunTime > 0) {
            uint32_t percentage = (task->ulRunTimeCounter * 10000) / totalRunTime;
            snprintf(cpuUsage, sizeof(cpuUsage), "%u.%02u%%", 
                     percentage / 100, percentage % 100);
        } else {
            strcpy(cpuUsage, "0.00%");
        }
        
        Log::w(TAG, "%-16s %10u %10s %10u", 
               taskName,
               static_cast<unsigned int>(task->ulRunTimeCounter),
               cpuUsage,
               static_cast<unsigned int>(task->usStackHighWaterMark));
    }
    
    delete[] taskStatusArray;
    
    Log::w(TAG, "-------------------------------------------------");
    Log::w(TAG, "Total Runtime: %u ticks", static_cast<unsigned int>(totalRunTime));
    Log::w(TAG, "Sample Period: %u ticks", static_cast<unsigned int>(timeDiff));
    Log::w(TAG, "===========================");
}

uint32_t TaskManager::getTotalRunTime() {
    return getRunTimeCounterValue();
}

void TaskManager::formatPercentage(uint32_t value, uint32_t total, char* buffer, size_t bufferSize) {
    if (total == 0) {
        snprintf(buffer, bufferSize, "0.00%%");
        return;
    }
    
    uint32_t percentage = (value * 10000) / total;
    snprintf(buffer, bufferSize, "%u.%02u%%", percentage / 100, percentage % 100);
}

void TaskManager::task() {
    Log::w(TAG, "TaskManager started");
    
    // 启动时打印任务列表
    delay(1000);  // 等待1秒让其他任务启动
    printTaskList();
    
    lastStatsTime = hal_hptimer_get_ms();
    
    for (;;) {
        uint32_t currentTime = hal_hptimer_get_ms();
        
        // 检查是否需要打印运行时统计
        if (statsInterval > 0 && 
            (currentTime - lastStatsTime) >= statsInterval) {
            
            if (enableRuntimeStats) {
                printRuntimeStats();
            }
            
            lastStatsTime = currentTime;
        }
        
        // 任务休眠1秒
        delay(1000);
    }
}

// 静态方法实现
void TaskManager::printTaskListStatic() {
    if (instance) {
        instance->printTaskList();
    }
}

void TaskManager::printRuntimeStatsStatic() {
    if (instance) {
        instance->printRuntimeStats();
    }
} 