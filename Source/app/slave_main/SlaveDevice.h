#pragma once

#include <memory>

#include "TaskCPP.h"
#include "LedFactory.h"


using namespace Adapter;
using namespace Interface;

namespace SlaveApp {

/**
 * SlaveDevice 类实现了从机设备的功能
 */
class SlaveDevice {
   public:
    static constexpr const char TAG[] = "SlaveDevice";

    SlaveDevice();  // 修改构造函数，自动读取UID
    ~SlaveDevice() = default;

    /**
     * 初始化设备
     * @return 是否成功初始化
     */
    bool initialize();

    /**
     * 运行主循环
     */
    void run();

   private:

    /**
     * 系统LED任务类
     */
    class SystemLedTask : public TaskClassS<512> {
       public:
        SystemLedTask(SlaveDevice& parent);

       private:
        SlaveDevice& parent;
        std::unique_ptr<Interface::ILed> systemLed;
        void task() override;
        static constexpr const char TAG[] = "SystemLedTask";
    };


    std::unique_ptr<SystemLedTask> systemLedTask;
};

}    // namespace SlaveApp