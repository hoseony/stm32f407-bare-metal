#include "../include/systick.h"
#include "../include/gpio.h"
#include "../include/utils.h"
#include "../include/uart.h"

// ============ Function Prototype =============

bool checkMatch(uint8_t *buf, uint8_t *bytes, uint32_t size);

// ========== Main Instructions ============
/* note
 *  maybe I should make a function for this:
 *      GPIO_t *gpio = GPIO(PINBANK(pin));
 *      uint8_t num = PINNO(pin);
 */

void GPIO_setAF(uint16_t pin, uint8_t AFnum) {
    // When using AF, you need to set the AF number 
    // with either AFLR(for pin 0..7) or AFHR(for pin 8..15)

    GPIO_t *gpio = GPIO(PINBANK(pin));
    uint8_t num = PINNO(pin);

    if (num > 7) { // use AFRH
        gpio->AFRH &= ~(0b1111U << ((num - 8) * 4));
        gpio->AFRH |= ((uint32_t)AFnum << ((num - 8) * 4));
    } else { // use AFRL
        gpio->AFRL &= ~(0b1111U << (num * 4));
        gpio->AFRL |= ((uint32_t)AFnum << (num * 4));
    }
}

/* For now, I might not need to do this? I'll see how it goes 
 * (I am thinking perhasp default is fine)

void GPIO_OTYPER_writeBit(uint16_t pin, bool val) {
    // OTYPER 
    // 0: Output Push-Pull 
    // 1: Output Open-drain

    GPIO_t *gpio = GPIO(PINBANK(pin));
    uint8_t num = PINNO(pin);


}

void GPIO_PUPDR_writeBit(uint16_t pin, uint8_t val) {
    // PUPDR 
    // 00: No pull up, No pull down
    // 01: Pull-up
    // 10: Pull-down
    // 11: Reserved

    GPIO_t *gpio = GPIO(PINBANK(pin));
    uint8_t num = PINNO(pin);
}

void GPIO_OSPEEDR_writeBit(uint16_t pin, uint8_t val) {
    // OSPEEDR
    // 00: Low Speed
    // 01: Medium Speed
    // 10: High Speed
    // 11: Very High Speed

    GPIO_t *gpio = GPIO(PINBANK(pin));
    uint8_t num = PINNO(pin);
}
*/

// ========== RingBuffer implementation ==========
#define RINGBUFFER_SIZE 64

typedef struct {
    uint8_t data[RINGBUFFER_SIZE];
    uint32_t head;
    uint32_t tail;
} RINGBUFFER_t; 

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

// ===============================================

int main(void) {
    
    // ========== blinky ==========
    uint16_t LED_green = PIN('D', 12);
    uint16_t LED_red = PIN('D', 14);

    RCC->AHB1ENR |= BIT(3);

    GPIO_setMode(LED_green, GPIO_MODE_OUTPUT);
    GPIO_setMode(LED_red , GPIO_MODE_OUTPUT);

    SysTick_init(16000000 / 1000);

    uint32_t timer_green= 0;
    uint32_t period = 500;
    // ========== USART ==========
    // Let's use these for now
    //  - PA0 AF8, UART4_TX
    //  - PA1 AF8, UART4_RX
    // You can find this from Datasheet Table. 9

    uint16_t uart_tx = PIN('A', 0);
    uint16_t uart_rx = PIN('A', 1);

    RCC->AHB1ENR |= BIT(0); // enable GPIOA
    RCC->APB1ENR |= BIT(19); // enable UART4

    GPIO_setMode(uart_tx, GPIO_MODE_AF);
    GPIO_setMode(uart_rx, GPIO_MODE_AF);

    // When using AF, you need to set the AF number with AFLR(for pin 0..7) or AFHR(for pin 8..15)
    GPIO_setAF(uart_tx, 8);
    GPIO_setAF(uart_rx, 8);

    UART4->CR1 = 0;
    UART4->BRR = 16000000 / 115200; // BaudRateRegister
    // This is actually not exact because we are leaving the fraction part for now

    UART4->CR1 = BIT(3) | BIT(2) | BIT(13);
    
    // ========= Main Loop ==========
    while (1) {
        /* e.g. testing transmission/reception of a byte
        
        if (timer_expired(&timer_green, period, s_ticks)) {
            static bool LED_green_bool = false;
            static bool LED_red_bool = false;

            GPIO_BSRR_writeBit(LED_green, LED_green_bool);
            GPIO_BSRR_writeBit(LED_red, LED_red_bool);

            UART_transmitByte(UART4, 'A');
            uint8_t received = UART_readByte(UART4);

            if (received == 'A') {
                LED_green_bool = !LED_green_bool;
            } else {
                LED_red_bool = !LED_red_bool;
            }
        }
        * Yeah, this works, but multiple bytes would be cooler */

        if (timer_expired(&timer_green, period, s_ticks)) {
            static bool LED_green_bool = false;
            static bool LED_red_bool = true;

            GPIO_BSRR_writeBit(LED_green, LED_green_bool);
            GPIO_BSRR_writeBit(LED_red, LED_red_bool);

            uint8_t bytes[] = "hello";
            uint32_t size = sizeof(bytes) - 1;
            uint8_t buf[sizeof(bytes) - 1];

            // Let's make interrupt based uart communication
            // As mentioned on the comment of the commit, 
            // The hardware can only hold 1 byte, so I can not send multiple bytes 
            // and hope it is stored inside some FIFO register 
            
            // For that, let's head to Reference Manual 30.4 USART interrupts
            // There is RXNE (received Data Ready to be Read)
            // Once this event happens, we immidetly need to read the data

            // One way of achieving this is by using "ring buffer" to store incoming data.
            

            if (checkMatch(bytes, buf, size)) {
                LED_green_bool = !LED_green_bool;
            } else {
                LED_red_bool = !LED_red_bool;
            }
        }
    }
    
    return 0;
}

bool checkMatch(uint8_t *buf, uint8_t *bytes, uint32_t size) {
    bool match = true;
    for (uint32_t i = 0; i < size; i++) {
        if (buf[i] != bytes[i]) {
            return false;
        }
    }
    
    return match;
}
