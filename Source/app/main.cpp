#include "Logger.h"
#include "mode_entry.h"

#ifdef __cplusplus
extern "C" {
#endif
#ifdef __cplusplus
}
#endif

int main(void) {
    nvic_priority_group_set(NVIC_PRIGROUP_PRE4_SUB0);

#ifdef MASTER
    // 主节点模式入口
    MasterInit();
#elif defined(SLAVE)
    // 从节点模式入口
    Slave_Init();
#elif defined(MASTER_BOARDTEST)
    // 从节点模式入口
    master_boardtest_init();
#elif defined(SLAVE_BOARDTEST)
    // 从节点模式入口
    slave_boardtest_init();
#else
#error "Please define either MASTER or SLAVE mode"
#endif

    vTaskStartScheduler();
    for (;;);
    return 0;
}
