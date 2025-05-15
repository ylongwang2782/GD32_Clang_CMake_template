#include "main.h"

#include <array>
#include <cstdint>
#include <cstdio>

#include "FreeRTOS.h"
#include "TaskCPP.h"
#include "bsp_led.hpp"
#include "bsp_log.hpp"
#include "bsp_uid.hpp"
#include "hello_gigadevice.h"
#include "netconf.h"
#include "task.h"
#include "tcp_client.h"
#include "udp_echo.h"

UartConfig uart7Conf(uart7_info);
Uart uartLog(uart7Conf);
Logger Log(uartLog);

extern Logger Log;

static void InitTask(void* pvParameters) {
    uint32_t myUid = UIDReader::get();
    Log.d("BOOT", "device ID: %02X", myUid);

    LED led0(GPIOA, GPIO_PIN_0);
    LedBlinkTask ledBlinkTask(led0, 500);
    ledBlinkTask.give();
    Log.v("BOOT", "ledBlinkTask initialized");

    LogTask logTask(Log);
    Log.setLogLevel(Logger::Level::VERBOSE);
    logTask.give();
    Log.v("BOOT", "logTask initialized");

    EthTask ethTask;
    ethTask.give();
    Log.v("BOOT", "ethTask initialized");

    while (1) {
        Log.v("SYS", "heap minimum: %d", xPortGetMinimumEverFreeHeapSize());
        vTaskDelay(pdMS_TO_TICKS(5000));
    }
}

int main(void) {
    nvic_priority_group_set(NVIC_PRIGROUP_PRE4_SUB0);
    xTaskCreate(InitTask, "InitTask", 4 * 1024, NULL, 4, NULL);
    vTaskStartScheduler();
    for (;;);
}
