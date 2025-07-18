#include "FreeRTOS.h"
#include "task.h"
#include <stdio.h>
// #include "bsp_log.h"
// #define IDLE_TASK_STACK_SIZE 32
// #define TIMER_TASK_STACK_SIZE 50

// static void delay()
// {
// 	for (uint32_t i = 0; i < SystemCoreClock / 2; i++) {
// 		;
// 	}
// }

// /**
//  * @brief 提供Idle任务的内存分配。
//  * 本函数用于在FreeRTOS操作系统中为Idle任务（也称为系统空闲任务）提供必要的内存资源，
//  * 包括任务控制块(TCB)和任务堆栈。
//  *
//  * @param ppxIdleTaskTCBBuffer
//  指向StaticTask_t类型的指针的地址，用于存放Idle任务的任务控制块(TCB)的地址。
//  * @param ppxIdleTaskStackBuffer
//  指向StackType_t类型的指针的地址，用于存放Idle任务的堆栈的起始地址。
//  * @param pulIdleTaskStackSize
//  指向uint32_t类型的指针，用于接收Idle任务的堆栈大小。
//  *
//  * @note 本函数不返回任何值。
//  */
// void vApplicationGetIdleTaskMemory(StaticTask_t **ppxIdleTaskTCBBuffer,
// 				   StackType_t **ppxIdleTaskStackBuffer,
// 				   uint32_t *pulIdleTaskStackSize)
// {
// 	static StaticTask_t xIdleTaskTCBBuffer;
// 	static StackType_t xIdleStack[IDLE_TASK_STACK_SIZE];
// 	*ppxIdleTaskTCBBuffer = &xIdleTaskTCBBuffer;
// 	*ppxIdleTaskStackBuffer = &xIdleStack[0];
// 	*pulIdleTaskStackSize = IDLE_TASK_STACK_SIZE;
// }

// /**
//  * @brief 提供计时器任务的内存。
//  * 本函数用于为 FreeRTOS
//  的计时器任务分配静态内存，包括任务控制块（TCB）和任务栈。
//  *
//  * @param ppxTimerTaskTCBBuffer 指向 StaticTask_t
//  类型指针的指针，函数执行后，将指向计时器任务的 TCB。
//  * @param ppxTimerTaskStackBuffer 指向 StackType_t
//  类型指针的指针，函数执行后，将指向计时器任务的栈内存。
//  * @param pulTimerTaskStackSize 指向 uint32_t
//  类型的指针，函数执行后，将存放计时器任务栈的大小。
//  */
// void vApplicationGetTimerTaskMemory(StaticTask_t **ppxTimerTaskTCBBuffer,
// 				    StackType_t **ppxTimerTaskStackBuffer,
// 				    uint32_t *pulTimerTaskStackSize)
// {
// 	/* Timer task control block and stack */
// 	static StaticTask_t Timer_TCB;
// 	static StackType_t Timer_Stack[TIMER_TASK_STACK_SIZE];

// 	*ppxTimerTaskTCBBuffer = &Timer_TCB;
// 	*ppxTimerTaskStackBuffer = &Timer_Stack[0];
// 	*pulTimerTaskStackSize = (uint32_t)TIMER_TASK_STACK_SIZE;
// }

// void configureTimerForRunTimeStats(void)
// {
// 	// 使能 DWT 周期计数器
// 	if (!(CoreDebug->DEMCR & CoreDebug_DEMCR_TRCENA_Msk)) {
// 		CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;
// 	}

// 	// 使能周期计数器
// 	DWT->CTRL |= DWT_CTRL_CYCCNTENA_Msk;
// }

// unsigned long getRunTimeCounterValue(void)
// {
// 	return DWT->CYCCNT;
// }

// Called by the kernel when a stack overflow is detected
void vApplicationStackOverflowHook(TaskHandle_t xTask, char *pcTaskName) {
    // 停止调度器（调试用）
    taskDISABLE_INTERRUPTS();

    // 打印错误信息（使用你系统的输出接口）
    printf("Stack overflow detected in task: %s\n", pcTaskName);

    // 无限循环等待开发者处理（如：进入 debug）
    for (;;);
}

// void vApplicationMallocFailedHook(void)
// {
// 	while (1) {
// 		delay();
// 		ERRF("Malloc failed");
// 	}
// }
