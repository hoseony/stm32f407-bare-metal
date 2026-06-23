#include "../include/uart.h"
#include "../include/utils.h"

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
RINGBUFFER_t UART4_ringBuffer = {0};

// ----------------------------------------
// Check if the UART is ready to read
int UART_readReady(USART_t *uart) {
    return uart->SR & BIT(5);
}

// Read the byte on DR and returns that byte
uint8_t UART_readByte(USART_t *uart) {
    while (!UART_readReady(uart)) { // wait till ready
        ;
    }

    uint8_t data = (uint8_t)(uart->DR & 0xFF);
    return data;
}

// Read multiple bytes (probably not going to be used as described in main.c)
//  *The buffer is just a byte. I just need to implement interrupt
void UART_readBytes(USART_t *uart, uint8_t *buf, uint32_t size) {
    for (uint32_t i = 0; i < size; i++) {
        buf[i] = UART_readByte(uart);
    }
}

// Write the data on DR (to transmit bytes)
void UART_transmitByte(USART_t *uart, uint8_t data) {
    uart->DR = data;

    while ( (uart->SR & BIT(7)) == 0 ) { // wait till transfer done
        ;
    }
}

// Transmit bytes in the buffer
void UART_transmitBuf(USART_t *uart, char *buf, size_t len) {
  while (len-- > 0) UART_transmitByte(uart, *(uint8_t *) buf++);
}

// UART_transmitBuf = UART_transmitBytes (functionally the same)
void UART_transmitBytes(USART_t *uart, uint8_t *data, uint32_t size) {
    for (uint32_t i = 0; i < size; i++) {
        uart->DR = data[i];

        while ( (uart->SR & BIT(7)) == 0 ) {
            ;
        }
    }
}

// ---------- RingBuffer implementation ----------
// return byte through *byte
bool RING_pop(RINGBUFFER_t *rb, uint8_t *byte) {
    if (rb->head == rb->tail) { // buffer empty, nothing to free
        return false;
    }

    *byte = rb->data[rb->head]; // return the value at the head
    rb->head = (rb->head + 1) % RINGBUFFER_SIZE; // move the head (% for wraparround)
    return true;
}

bool RING_push(RINGBUFFER_t *rb, uint8_t byte) {
    // % RINGBUFFER_SIZE in case wraparround
    uint32_t nextTail = (rb->tail + 1) % RINGBUFFER_SIZE;

    if (nextTail == rb->head) { // buffer is full
        return false;
    }

    rb->data[rb->tail] = byte;
    rb->tail = nextTail;
    return true;
}

// ---------- Interrupt ----------
void UART4_IRQHandler(void) {
    if (UART4->SR & BIT(5)) { // RXNE
        uint8_t byte = UART4->DR;
        RING_push(&UART4_ringBuffer, byte);
    }
}
