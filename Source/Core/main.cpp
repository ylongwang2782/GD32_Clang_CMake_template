#include "main.h"

#include <array>
#include <cstdint>
#include <cstdio>

#include "FreeRTOS.h"
#include "TaskCPP.h"
#include "bsp_led.hpp"
#include "bsp_log.hpp"
#include "bsp_uid.hpp"
#include "cx_uwb_test.hpp"
#include "dw1000.hpp"
#include "netconf.h"
#include "task.h"
#include "udp_echo.h"
#include "uwb.hpp"

UartConfig uart3Conf(uart3_info);
Uart uartLog(uart3Conf);
Logger Log(uartLog);

extern Logger Log;

static void InitTask(void* pvParameters) {
    // UWB<UwbSpiInterface> uwb;
    // uint32_t myUid = UIDReader::get();
    // Log.d("BOOT", "device ID: %02X", myUid);
    // printf("dwt_initialise");

    LED led0(GPIOC, GPIO_PIN_13);
    LedBlinkTask ledBlinkTask(led0, 500);
    ledBlinkTask.give();
    // Log.v("BOOT", "ledBlinkTask initialized");

    LogTask logTask(Log);
    Log.setLogLevel(Logger::Level::VERBOSE);
    logTask.give();
    // Log.v("BOOT", "logTask initialized");

    // EthTask ethTask;
    // ethTask.give();
    // Log.v("BOOT", "ethTask initialized");

    DW1000 dw1000;
    // taskENTER_CRITICAL();

    if (dw1000.init()) {
        Log.d("BOOT", "dw1000 initialized successfully");
    } else {
        // Log.e("BOOT", "dw1000 initialization failed");
    }
    // taskEXIT_CRITICAL();

    // Log.v("BOOT", "dw1000 initflag: %d", dw1000.init_succseed);
    std::vector<uint8_t> usr_data = {0x01, 0x02, 0x03, 0x04, 0x05, 0x00, 0x00};
    while (1) {
        Log.v("SYS", "heap minimum: %d", xPortGetMinimumEverFreeHeapSize());

        // taskENTER_CRITICAL();
        // if (dw1000.data_transmit(usr_data)) {
        //     Log.d("SYS", "Transmit Data");
        // } else {
        //     Log.w("SYS", "Transmit failed");
        // }

        dw1000.set_recv_mode();    // 设置接收模式
        dw1000.update();

        if (dw1000.get_recv_data(usr_data)) {
            Log.r(usr_data.data(), usr_data.size());
        }
        // taskEXIT_CRITICAL();
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}

int main(void) {
    nvic_priority_group_set(NVIC_PRIGROUP_PRE4_SUB0);
    xTaskCreate(InitTask, "InitTask", 4 * 1024, NULL, 4, NULL);
    vTaskStartScheduler();
    for (;;);
}
