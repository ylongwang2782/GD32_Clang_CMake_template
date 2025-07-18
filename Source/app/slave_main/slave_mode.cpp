#include "../mode_entry.h"
#include "LogManager.h"
#include "Logger.h"
#include "SlaveDevice.h"
#include "hal_hptimer.hpp"

#ifdef __cplusplus
extern "C" {
#endif
#ifdef __cplusplus
}
#endif

#ifdef SLAVE

using namespace SlaveApp;

static void Slave_Task(void *pvParameters) {
    static constexpr const char TAG[] = "BOOT";

    LogManager::quickInit(LogManager::UART_TYPE_UART3, true,
                          Logger::Level::TRACE);
    Log::d(TAG, "LogTask initialized");

    if (!hal_hptimer_init()) {
        Log::e("SlaveDevice", "Failed to initialize high precision timer");
        // return false;
    }

    // // 创建任务管理器，每15秒打印一次运行时统计
    // TaskManager taskManager(15000);
    // Log::d(TAG, "TaskManager initialized");

    SlaveDevice slaveDevice;
    slaveDevice.run();

    while (1) {
        vTaskDelay(pdMS_TO_TICKS(10));
    }
}

int Slave_Init(void) {
    xTaskCreate(Slave_Task, "SlaveTask", 8 * 1024, NULL, 2, NULL);

    return 0;
}

#endif