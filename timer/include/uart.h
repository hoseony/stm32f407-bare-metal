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

extern RINGBUFFER_t USART1_ringBuffer;
extern RINGBUFFER_t USART2_ringBuffer;
extern RINGBUFFER_t USART3_ringBuffer;
extern RINGBUFFER_t UART4_ringBuffer;
extern RINGBUFFER_t UART5_ringBuffer;
extern RINGBUFFER_t USART6_ringBuffer;

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
void USART1_IRQHandler(void);
void USART2_IRQHandler(void);
void USART3_IRQHandler(void);
void UART4_IRQHandler(void);
void UART5_IRQHandler(void);
void USART6_IRQHandler(void);


// Driver
void UART_init(USART_t *uart, uint32_t baudRate, bool rxInterruptEnable, uint8_t priority);
bool UART_gpioInit(USART_t *uart, uint16_t tx, uint16_t rx);
bool UART_getPinAF(USART_t *uart, uint16_t pin, bool isRX, uint8_t *af);
// ----------------------------------------

#endif
