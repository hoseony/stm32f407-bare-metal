#include "../include/uart.h"
#include "../include/utils.h"

int UART_readReady(USART_t *uart) {
    return uart->SR & BIT(5);
}

uint8_t UART_readByte(USART_t *uart) {
    while (!UART_readReady(uart)) { // wait till ready
        ;
    }

    uint8_t data = (uint8_t)(uart->DR & 0xFF);
    return data;
}
/*
void UART_readBytes(USART_t *uart, uint8_t *buffer, uint32_t bufferSize) {
    for (uint32_t i = 0; i < bufferSize; i++) {
        while ( !(uart->SR & BIT(5)) ) {
            ;
        }
        buffer[i] = (uint8_t)(uart->DR & 0xFF);
    }
}
*/


void UART_transmitByte(USART_t *uart, uint8_t data) {
    uart->DR = data;

    while ( (uart->SR & BIT(7)) == 0 ) { // wait till transfer done
        ;
    }
}


void UART_writeBuf(USART_t *uart, char *buf, size_t len) {
  while (len-- > 0) UART_transmitByte(uart, *(uint8_t *) buf++);
}


//
void UART_transmitBytes(USART_t *uart, uint8_t *data, uint32_t len) {
    for (uint32_t i = 0; i < len; i++) {
        uart->DR = data[i];
        while ( (uart->SR & BIT(7)) == 0 ) {
            ;
        }
    }
} 
