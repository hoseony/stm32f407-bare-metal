#ifndef UART_H
#define UART_H

#include "stm32f407_registers.h"
#include <stddef.h>
#include <stdbool.h>

#define RINGBUFFER_SIZE 64

typedef struct {
    uint8_t data[RINGBUFFER_SIZE];
    uint32_t head;
    uint32_t tail;
} RINGBUFFER_t; 

extern RINGBUFFER_t UART4_ringBuffer;

// ---------- function prototype ----------
int UART_readReady(USART_t *uart);
uint8_t UART_readByte(USART_t *uart);
void UART_readBytes(USART_t *uart, uint8_t *buf, uint32_t size);
void UART_transmitByte(USART_t *uart, uint8_t data);

// note: these two are functionally the same. I left transmitBytes for readability
void UART_transmitBuf(USART_t *uart, char *buf, size_t len);
void UART_transmitBytes(USART_t *uart, uint8_t *data, uint32_t size);

// Ring Buffer
bool RING_pop(RINGBUFFER_t *rb, uint8_t *byte);
bool RING_push(RINGBUFFER_t *rb, uint8_t byte);

// Interrupt
void UART4_IRQHandler(void);
// ----------------------------------------

#endif
