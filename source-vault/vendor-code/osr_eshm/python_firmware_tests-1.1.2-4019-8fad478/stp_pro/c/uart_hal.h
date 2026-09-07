#ifndef UART_HAL_H
#define UART_HAL_H

#include <stdint.h>

uint32_t uart_init(void);
uint32_t uart_send_byte(uint8_t val);
uint32_t uart_peek_byte(uint8_t* val);
void uart_deinit(void);


#endif // UART_HAL_H
