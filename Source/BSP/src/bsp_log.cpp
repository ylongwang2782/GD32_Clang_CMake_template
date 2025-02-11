#include "bsp_log.h"

void vAssertCalled(const char *file, int line) {
    taskDISABLE_INTERRUPTS();
    printf("Assert failed in file %s at line %d\n", file, line);
    for (;;);
}

#ifdef ARM
extern "C" {
int fputc(int ch, FILE *f) {
    usart_data_transmit(USART1, (uint8_t)ch);
    while (RESET == usart_flag_get(USART1, USART_FLAG_TBE)) {
    }
    return ch;
}
}
#endif

#ifdef GCC
extern "C" {
int _write(int fd, char *pBuffer, int size) {
    for (int i = 0; i < size; i++) {
        while (RESET == usart_flag_get(USART1, USART_FLAG_TBE));
        usart_data_transmit(USART1, (uint8_t)pBuffer[i]);
    }
    return size;
}
}
#endif