#include "../include/uart.h"
#include "../include/utils.h"
#include "../include/gpio.h"
#include "../include/nvic.h"

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
// initialization of ring buffer
RINGBUFFER_t USART1_ringBuffer = {0};
RINGBUFFER_t USART2_ringBuffer = {0};
RINGBUFFER_t USART3_ringBuffer = {0};
RINGBUFFER_t UART4_ringBuffer = {0};
RINGBUFFER_t UART5_ringBuffer = {0};
RINGBUFFER_t USART6_ringBuffer = {0};
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
    while ( (uart->SR & BIT(7)) == 0 ) { // wait till transfer done
        ;
    }

    uart->DR = data;
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
    return true; // if it did pop, return true
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
void USART1_IRQHandler(void) {
     if (USART1->SR & BIT(5)) { // RXNE
        uint8_t byte = (uint8_t)(USART1->DR);
        RING_push(&USART1_ringBuffer, byte);
    }
}

void USART2_IRQHandler(void) {
     if (USART2->SR & BIT(5)) { // RXNE
        uint8_t byte = (uint8_t)(USART2->DR);
        RING_push(&USART2_ringBuffer, byte);
    }
}

void USART3_IRQHandler(void) {
     if (USART3->SR & BIT(5)) { // RXNE
        uint8_t byte = (uint8_t)(USART3->DR);
        RING_push(&USART3_ringBuffer, byte);
    }
}

void UART4_IRQHandler(void) {
     if (UART4->SR & BIT(5)) { // RXNE
        uint8_t byte = (uint8_t)(UART4->DR);
        RING_push(&UART4_ringBuffer, byte);
    }
}

void UART5_IRQHandler(void) {
     if (UART5->SR & BIT(5)) { // RXNE
        uint8_t byte = (uint8_t)(UART5->DR);
        RING_push(&UART5_ringBuffer, byte);
    }
}

void USART6_IRQHandler(void) {
     if (USART6->SR & BIT(5)) { // RXNE
        uint8_t byte = (uint8_t)(USART6->DR);
        RING_push(&USART6_ringBuffer, byte);
    }
}

// ---------- Generalized Driver ----------
// To make using things easier, I made initialization functions
// e.g. 
/*
    UART_gpioInit(UART4, uart_tx, uart_rx);
    UART_init(UART4, 115200, true, 5);
*/
void UART_init(USART_t *uart, uint32_t baudRate, bool rxInterruptEnable, uint8_t priority) {
// Initialize uart peripheral register 
// priority is unused if rxInterruptEnable is false

    // initialize default values
    uint32_t pclk = 16000000;
    uint32_t irqn = 0;
    
    // reference ../include/stm32f407_registers.h for more
    // uart 8 and uart 7 are not real thing on this mcu!
    /*if (uart == UART8) {
        RCC->APB1ENR |= BIT(31);
        irqn = IRQn_UART8;

    } else if (uart == UART7) {
        RCC->APB1ENR |= BIT(30);
        irqn = IRQn_UART7;

    } else */ 
    if (uart == USART6) {
        RCC->APB2ENR |= BIT(5);
        irqn = IRQn_USART6;

    } else if (uart == UART5) {
        RCC->APB1ENR |= BIT(20);
        irqn = IRQn_UART5;

    } else if (uart == UART4) {
        RCC->APB1ENR |= BIT(19);
        irqn = IRQn_UART4;

    } else if (uart == USART3) {
        RCC->APB1ENR |= BIT(18);
        irqn = IRQn_USART3;

    } else if (uart == USART2) {
        RCC->APB1ENR |= BIT(17);
        irqn = IRQn_USART2;

    } else if (uart == USART1) {
        RCC->APB2ENR |= BIT(4);
        irqn = IRQn_USART1;
    }

    uart->CR1 = 0;
    uart->CR2 = 0;
    uart->CR3 = 0;

    // actual ok way of doing the baudRate
    uart->BRR = (pclk + (baudRate / 2U))/ baudRate;
    uart->CR1 = BIT(3) | BIT(2) | BIT(13);

    if (rxInterruptEnable) {
        uart->CR1 |= BIT(5);
        NVIC_setPriority(irqn, priority);
        NVIC_enableIRQ(irqn);
    }
}

bool UART_gpioInit(USART_t *uart, uint16_t tx, uint16_t rx) {
// receive tx & rx -> gpio enable, gpio set to AF, set AF number (maybe this is too much???)
// to do this eaiser, let's make function to get AF...

    uint8_t txAF, rxAF;

    // Check if txAF and rxAF is available (and save it if is)
    if (!UART_getPinAF(uart, tx, false, &txAF)) {
        return false;
    } 

    if (!UART_getPinAF(uart, rx, true, &rxAF)) {
        return false;
    }

    GPIO_enable(tx);
    GPIO_enable(rx);

    GPIO_setMode(tx, GPIO_MODE_AF);
    GPIO_setMode(rx, GPIO_MODE_AF);

    GPIO_setAF(tx, txAF);
    GPIO_setAF(rx, rxAF);

    return true;
}

bool UART_getPinAF(USART_t *uart, uint16_t pin, bool isRX, uint8_t *af) {
// function that returns af number for gpioInit

        if (af == NULL) {
            return false;
        }

        if (uart == USART1) {
            // from Datasheet - Table 9
            // tx - PA9, PB6
            // rx - PA10 PB7

            if(!isRX && (pin == PIN('A', 9) || pin == PIN('B', 6))) { // for tx
                *af = 7;
                return true;
            }

            if(isRX && (pin == PIN('A', 10) || pin == PIN('B', 7))) { // for rx
                *af = 7;
                return true;
            }
        }

        else if (uart == USART2) {
            if(!isRX && (pin == PIN('A', 2) || pin == PIN('D', 5))) {
                *af = 7;
                return true;
            }

            if(isRX && (pin == PIN('A', 3) || pin == PIN('D', 6))) {
                *af = 7;
                return true;
            }
        }

        else if (uart == USART3) {
            if(!isRX && (pin == PIN('B', 10) || pin == PIN('C', 10) || pin == PIN('D', 8))) {
                *af = 7;
                return true;
            }

            if(isRX && (pin == PIN('B', 11) || pin == PIN('C', 11) || pin == PIN('D', 9))) {
                *af = 7;
                return true;
            }
        }

        else if (uart == UART4) {
            if(!isRX && (pin == PIN('A', 0) || pin == PIN('C', 10))) {
                *af = 8;
                return true;
            }

            if(isRX && (pin == PIN('A', 1) || pin == PIN('C', 11))) {
                *af = 8;
                return true;
            }
        }

        else if (uart == UART5) {
            if(!isRX && (pin == PIN('C', 12))) {
                *af = 8;
                return true;
            }

            if(isRX && (pin == PIN('D', 2))) {
                *af = 8;
                return true;
            }
        }

        else if (uart == USART6) {
            if(!isRX && (pin == PIN('C', 6) || pin == PIN('G', 14))) {
                *af = 8;
                return true;
            }

            if(isRX && (pin == PIN('C', 7) || pin == PIN('G', 9))) {
                *af = 8;
                return true;
            }
        }

        return false;
}

