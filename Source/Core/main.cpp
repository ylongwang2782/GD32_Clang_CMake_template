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

void lwip_netif_status_callback(struct netif* netif) {
    Log.d("NET", "netif status changed: %d", netif->flags);
    // logd addr
    Log.d("NET", "netif addr: %d.%d.%d.%d", ip4_addr1_16(&netif->ip_addr),
          ip4_addr2_16(&netif->ip_addr), ip4_addr3_16(&netif->ip_addr),
          ip4_addr4_16(&netif->ip_addr));
    if (((netif->flags & NETIF_FLAG_UP) != 0) && (0 != netif->ip_addr.addr)) {
        /* initilaize the tcp server: telnet 8000 */
        hello_gigadevice_init();
        Log.d("BOOT", "tcp server initialized");
        /* initilaize the tcp client: echo 10260 */
        tcp_client_init();
        Log.d("BOOT", "tcp client initialized");
        /* initilaize the udp: echo 1025 */
        udp_echo_init();
        Log.d("BOOT", "udp echo initialized");
    }
}

static void InitTask(void* pvParameters) {
    uint32_t myUid = UIDReader::get();
    Log.d("BOOT", "device ID: %02X", myUid);

    LED led0(GPIOA, GPIO_PIN_0);
    LedBlinkTask ledBlinkTask(led0, 500);
    ledBlinkTask.give();

    LogTask logTask(Log);
    logTask.give();

    /* configure ethernet (GPIOs, clocks, MAC, DMA) */
    enet_system_setup();
    Log.d("BOOT", "ethernet initialized");

    /* initilaize the LwIP stack */
    lwip_stack_init();
    Log.d("BOOT", "lwip stack initialized");

    while (1) {
        Log.d("SYS", "heap minimum: %d", xPortGetMinimumEverFreeHeapSize());
        vTaskDelay(pdMS_TO_TICKS(5000));
    }
}

int main(void) {
    nvic_priority_group_set(NVIC_PRIGROUP_PRE4_SUB0);
    xTaskCreate(InitTask, "InitTask", 4 * 1024, NULL, 4, NULL);
    vTaskStartScheduler();
    for (;;);
}
