#ifndef UART_H
#define UART_H

#include "stm32f407_registers.h"
#include <stddef.h>

uint8_t UART_readByte(USART_t *uart);
void UART_transmitByte(USART_t *uart, uint8_t data);
void UART_writeBuf(USART_t *uart, char *buf, size_t len);
void UART_transmitBytes(USART_t *uart, uint8_t *data, uint32_t len);

#endif
