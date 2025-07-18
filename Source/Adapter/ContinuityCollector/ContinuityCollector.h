#ifndef CONTINUITY_COLLECTOR_H
#define CONTINUITY_COLLECTOR_H

#include <cstdint>
#include <functional>
#include <memory>
#include <string>
#include <vector>

#include "IGpio.hpp"
#include "Logger.h"

using namespace Interface;

namespace Adapter {

// 导通状态枚举
enum class ContinuityState : uint8_t {
    DISCONNECTED = 0,    // 断开
    CONNECTED = 1        // 导通
};

// 硬件定义的引脚映射表 (64个引脚)
static constexpr uint8_t HARDWARE_PIN_MAP[64] = {
    // PA3 - PA12, PA15
    3, 4, 5, 6, 7,           // PA3-PA7
    8, 9, 10, 11, 12, 15,   // PA8-PA12, PA15
    
    // PB0-1, PB10-PB15
    16+0, 16+1,             // PB0-PB1
    16+10, 16+11, 16+12, 16+13, 16+14, 16+15,  // PB10-PB15
    
    // PC4-PC11
    32+4, 32+5, 32+6, 32+7, 32+8, 32+9, 32+10, 32+11,  // PC4-PC11
    
    // PD0-1, PD3-PD4, PD8-PD15
    48+0, 48+1, 48+3, 48+4,           // PD0-1, PD3-PD4
    48+8, 48+9, 48+10, 48+11,         // PD8-PD11
    48+12, 48+13, 48+14, 48+15,       // PD12-PD15
    
    // PE7-PE15
    64+7, 64+8, 64+9, 64+10, 64+11, 64+12, 64+13, 64+14, 64+15,  // PE7-PE15
    
    // PF11-PF15
    80+11, 80+12, 80+13, 80+14, 80+15,  // PF11-PF15
    
    // PG0-PG8
    96+0, 96+1, 96+2, 96+3, 96+4, 96+5, 96+6, 96+7, 96+8   // PG0-PG8
};

// 导通数据采集配置
struct CollectorConfig {
    uint8_t num;                    // 导通检测数量 a (< 64)
    uint8_t startDetectionNum;      // 开始检测数量 b
    uint8_t totalDetectionNum;      // 总检测数量 k
    uint32_t interval;              // 检测间隔 (毫秒)
    bool autoStart;                 // 是否自动开始采集

    CollectorConfig(uint8_t n = 2, uint8_t startDetNum = 0,
                    uint8_t totalDetNum = 4, uint32_t i = 20,
                    bool autoS = false)
        : num(n),
          startDetectionNum(startDetNum),
          totalDetectionNum(totalDetNum),
          interval(i),
          autoStart(autoS) {

        if (num > 64) num = 64;
        if (totalDetectionNum == 0 || totalDetectionNum > 64)
            totalDetectionNum = 64;
        if (startDetectionNum >= totalDetectionNum) startDetectionNum = 0;

        Log::t("CollectorConfig", "Constructor: n=%d, final num=%d", n, num);
    }

    // 获取逻辑引脚对应的物理引脚
    uint8_t getPhysicalPin(uint8_t logicalPin) const {
        Log::t("CollectorConfig",
               "getPhysicalPin called, logicalPin: %d",
               logicalPin);

        if (logicalPin < 64) {
            uint8_t physicalPin = HARDWARE_PIN_MAP[logicalPin];
            Log::t("CollectorConfig", "returning physicalPin: %d", physicalPin);
            return physicalPin;
        }

        Log::t("CollectorConfig", "logicalPin out of range, returning: %d",
               logicalPin);
        return logicalPin;    // 如果超出范围，返回逻辑引脚号
    }
};

// 导通数据矩阵类型
using ContinuityMatrix = std::vector<std::vector<ContinuityState>>;

// 采集状态枚举
enum class CollectionStatus : uint8_t {
    IDLE = 0,         // 空闲状态
    RUNNING = 1,      // 正在采集
    COMPLETED = 2,    // 采集完成
    ERROR = 3         // 错误状态
};

    // 采集进度回调函数类型
using ProgressCallback =
    std::function<void(uint8_t cycle, uint8_t totalCycles)>;

// 时间同步回调函数类型 - 用于获取同步时间
using SyncTimeCallback = std::function<uint64_t()>;

// 导通数据采集器类
class ContinuityCollector {
   private:
    static constexpr uint8_t MAX_GPIO_PINS = 64;

    std::unique_ptr<IGpio> gpio_;    // GPIO接口
    CollectorConfig config_;         // 采集配置
    ContinuityMatrix dataMatrix_;    // 数据矩阵

    CollectionStatus status_;              // 采集状态
    uint8_t currentCycle_;                 // 当前周期
    uint64_t lastProcessTime_;             // 上次处理时间（毫秒）
    ProgressCallback progressCallback_;    // 进度回调
    SyncTimeCallback syncTimeCallback_;    // 同步时间回调
    
    // 引脚状态跟踪
    int8_t lastActivePin_;                 // 上一个激活的引脚（-1表示无）

    // 私有方法
    void initializeGpioPins();      // 初始化GPIO引脚
    void deinitializeGpioPins();    // 反初始化GPIO引脚
    ContinuityState readPinContinuity(
        uint8_t logicalPin);    // 读取单个引脚导通状态
    void configurePinsForCycle(
        uint8_t currentCycle);      // 为当前周期配置引脚模式
    uint32_t getCurrentTimeMs();    // 获取当前时间（毫秒）
    uint64_t getCurrentTimeUs();    // 获取当前时间（微秒）
    uint32_t getSyncTimeMs();       // 获取同步时间（毫秒）
    uint64_t getSyncTimeUs();       // 获取同步时间（微秒）
    void delayMs(uint32_t ms);      // 延迟函数

   public:
    ContinuityCollector(std::unique_ptr<IGpio> gpio);
    ~ContinuityCollector();

    // 删除拷贝构造函数和赋值操作符
    ContinuityCollector(const ContinuityCollector &) = delete;
    ContinuityCollector &operator=(const ContinuityCollector &) = delete;

    // 配置采集参数
    bool configure(const CollectorConfig &config);

    // 开始采集
    bool startCollection();

    // 停止采集
    void stopCollection();

    // 处理采集状态（状态机）
    void processCollection();

    // 获取采集状态
    CollectionStatus getStatus() const;

    // 获取当前周期
    uint8_t getCurrentCycle() const;

    // 获取总周期数
    uint8_t getTotalCycles() const;

    // 获取采集数据
    ContinuityMatrix getDataMatrix() const;

    // 获取指定周期的数据
    std::vector<ContinuityState> getCycleData(uint8_t cycle) const;

    // 检查是否有新数据
    bool hasNewData() const;

    // 检查采集是否完成
    bool isCollectionComplete() const;

    // 设置进度回调
    void setProgressCallback(ProgressCallback callback);

    // 设置同步时间回调
    void setSyncTimeCallback(SyncTimeCallback callback);

    // 获取采集配置
    const CollectorConfig &getConfig() const { return config_; }

    // 获取采集进度百分比
    float getProgress() const;

    // 获取GPIO接口（用于测试）
    IGpio *getGpio() const { return gpio_.get(); }

    // 获取压缩数据向量（按位压缩，小端模式）
    std::vector<uint8_t> getDataVector() const;

    // 获取指定引脚的所有周期数据
    std::vector<ContinuityState> getPinData(uint8_t pin) const;

    // 清空数据矩阵
    void clearData();

    // 统计功能
    struct Statistics {
        uint32_t totalConnections;       // 总导通次数
        uint32_t totalDisconnections;    // 总断开次数
        double connectionRate;           // 导通率
        uint8_t mostActivePins[5];       // 最活跃的5个引脚
    };

    Statistics calculateStatistics() const;
};

// 导通数据采集器工厂类
class ContinuityCollectorFactory {
   public:
    // 创建带有虚拟GPIO的采集器（用于测试）
    static std::unique_ptr<ContinuityCollector> createWithGpio();

    // 创建带有自定义GPIO的采集器
    static std::unique_ptr<ContinuityCollector> createWithCustomGpio(
        std::unique_ptr<IGpio> gpio);
};

}    // namespace Adapter

#endif    // CONTINUITY_COLLECTOR_H