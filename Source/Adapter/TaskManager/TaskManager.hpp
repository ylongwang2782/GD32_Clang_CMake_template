#pragma once
#ifndef _TASK_MANAGER_H_
#define _TASK_MANAGER_H_

#include "FreeRTOS.h"
#include "task.h"
#include "TaskCPP.h"
#include "Logger.h"

#define TASK_MANAGER_TASK_DEPTH_SIZE 1024
#define TASK_MANAGER_TASK_PRIO       TaskPrio_Low

// 任务统计信息结构
struct TaskStats {
    char taskName[configMAX_TASK_NAME_LEN];
    UBaseType_t taskNumber;
    eTaskState taskState;
    UBaseType_t currentPriority;
    UBaseType_t basePriority;
    uint32_t runTimeCounter;
    uint32_t stackHighWaterMark;
};

/**
 * @brief 任务管理器类
 * 
 * 提供FreeRTOS任务监控功能：
 * 1. 启动时打印任务列表
 * 2. 定期打印任务运行时统计信息
 */
class TaskManager : public FreeRTOScpp::TaskClassS<0> {
private:
    static const char* TAG;
    static TaskManager* instance;
    
    uint32_t statsInterval;     // 统计信息打印间隔(ms)
    uint32_t lastStatsTime;     // 上次打印统计信息的时间
    bool enableRuntimeStats;    // 是否启用运行时统计
    
    // 运行时统计相关
    uint32_t totalRunTime;      // 总运行时间
    uint32_t lastTotalRunTime;  // 上次记录的总运行时间
    
    /**
     * @brief 获取任务状态字符串
     */
    const char* getTaskStateString(eTaskState state);
    
    /**
     * @brief 打印任务列表
     */
    void printTaskList();
    
    /**
     * @brief 打印运行时统计信息
     */
    void printRuntimeStats();
    
    /**
     * @brief 获取系统总运行时间
     */
    uint32_t getTotalRunTime();
    
    /**
     * @brief 格式化百分比输出
     */
    void formatPercentage(uint32_t value, uint32_t total, char* buffer, size_t bufferSize);

public:
    /**
     * @brief 构造函数
     * @param statsIntervalMs 统计信息打印间隔(毫秒)，0表示禁用
     */
    TaskManager(uint32_t statsIntervalMs = 10000);
    
    /**
     * @brief 析构函数
     */
    virtual ~TaskManager();
    
    /**
     * @brief 获取单例实例
     */
    static TaskManager* getInstance();
    
    /**
     * @brief 设置统计信息打印间隔
     * @param intervalMs 间隔时间(毫秒)，0表示禁用
     */
    void setStatsInterval(uint32_t intervalMs);
    
    /**
     * @brief 启用/禁用运行时统计
     * @param enable true启用，false禁用
     */
    void enableRuntimeStatistics(bool enable);
    
    /**
     * @brief 手动触发打印任务列表
     */
    void triggerTaskList();
    
    /**
     * @brief 手动触发打印运行时统计
     */
    void triggerRuntimeStats();
    
    /**
     * @brief 任务主循环
     */
    virtual void task() override;
    
    /**
     * @brief 静态方法：打印任务列表
     */
    static void printTaskListStatic();
    
    /**
     * @brief 静态方法：打印运行时统计
     */
    static void printRuntimeStatsStatic();
};

#endif // _TASK_MANAGER_H_ 