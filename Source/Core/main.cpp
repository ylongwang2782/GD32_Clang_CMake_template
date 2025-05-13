#include <array>
#include <cstdint>
#include <cstdio>

#include "TaskCPP.h"
#include "TaskCpp.h"
#include "bsp_adc.hpp"
#include "bsp_led.hpp"
#include "bsp_log.hpp"
#include "bsp_spi.hpp"
#include "bsp_uid.hpp"

#ifdef __cplusplus
extern "C" {
#endif
#include "FreeRTOS.h"
#include "task.h"

#ifdef __cplusplus
}
#endif

UartConfig uart7Conf(uart7_info);
Uart uartLog(uart7Conf);
Logger Log(uartLog);

class UsartDMATask : public TaskClassS<1024> {
   public:
    UsartDMATask() : TaskClassS<1024>("UsartDMATask", TaskPrio_Mid) {}

    void task() override {
        std::vector<uint8_t> rx_data;
        for (;;) {
            // 等待 DMA 完成信号
            if (xSemaphoreTake(usart0_info.dmaRxDoneSema, portMAX_DELAY) ==
                pdPASS) {
                // rx_data = usart0.getReceivedData();
                // if (rx_data.size() > 0) {
                //     usart0.send(rx_data.data(), rx_data.size());
                // }
            }
        }
    }
};

// UsartDMATask usartDMATask;

static void InitTask(void* pvParameters) {
    uint32_t myUid = UIDReader::get();
    Log.d("BOOT", "device ID: %02X", myUid);

    LED led0(GPIOA, GPIO_PIN_0);
    LedBlinkTask ledBlinkTask(led0, 500);
    ledBlinkTask.give();

    LogTask logTask(Log);
    logTask.give();

    while (1) {
        Log.d("SYS", "heap minimum: %d", xPortGetMinimumEverFreeHeapSize());
        vTaskDelay(pdMS_TO_TICKS(5000));
    }
}

int main(void) {
    nvic_priority_group_set(NVIC_PRIGROUP_PRE4_SUB0);
    xTaskCreate(InitTask, "InitTask", 4 * 1024, NULL, 2, NULL);
    vTaskStartScheduler();
    for (;;);
}
