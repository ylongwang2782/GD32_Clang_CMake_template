#include "bsp_log.h"

#include "bsp_uart.h"
#include "printf.h"


void _putchar(char character) {
    usart_data_transmit(USART1, (uint8_t)character);
}