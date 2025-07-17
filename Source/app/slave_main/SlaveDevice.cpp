#include "SlaveDevice.h"

#include <cstdio>

#include "Logger.h"


using namespace Interface;

namespace SlaveApp {

SlaveDevice::SlaveDevice() {
    systemLedTask = std::make_unique<SystemLedTask>(*this);
}

void SlaveDevice::run() {
    if (systemLedTask) {
        systemLedTask->give();
        Log::d(TAG, "SystemLedTask initialized and started");
    }

    while (1) {
        TaskBase::delay(10);
    }
}


// SystemLedTask 实现
SlaveDevice::SystemLedTask::SystemLedTask(SlaveDevice& parent)
    : TaskClassS("SystemLedTask", TaskPrio_Low), parent(parent) {
    systemLed = Adapter::LedFactory::createSystemLeds();
}

void SlaveDevice::SystemLedTask::task() {
    if (!systemLed) {
        Log::e(TAG, "Failed to create system LED");
        return;
    }

    // 初始设置：无错误，正在运行
    systemLed->setSystemStatus(false, true);

    for (;;) {

        systemLed->setSystemStatus(false, true);
        systemLed->update();
        TaskBase::delay(50);    // 50ms更新间隔
    }
}

}    // namespace SlaveApp