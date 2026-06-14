#include <inttypes.h>

#include "../include/systick.h"
#include "../include/gpio.h"
#include "../include/utils.h"
#include "../include/uart.h"

// ------------ Main Instructions -------------
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

void UART_readBytes(USART_t *uart, uint8_t *buffer, uint32_t bufferSize) {
    for (uint32_t i = 0; i < bufferSize; i++) {
        while ( !(uart->SR & BIT(5)) ) {
            ;
        }
        buffer[i] = (uint8_t)(uart->DR & 0xFF);
    }
}

void UART_transmitByte(USART_t *uart, uint8_t data) {
    uart->DR = data;

    while ( (uart->SR & BIT(7)) == 0 ) { // wait till transfer done
        ;
    }
}

void UART_transmitBytes(USART_t *uart, uint8_t *data, uint32_t len) {
    for (int i = 0; i < len; i++) {
        uart->DR = data[i];
        while ( (uart->SR & BIT(7)) == 0 ) {
            ;
        }
    }
} 

int main(void) {
    
    // blinking setup
    uint16_t LED_green = PIN('D', 12);
    RCC->AHB1ENR |= BIT(3);

    GPIO_setMode(LED_green, GPIO_MODE_OUTPUT);
    SysTick_init(16000000 / 1000);

    uint32_t timer_green= 0;
    uint32_t period = 500;
     
    // USART
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
    UART4->BRR = 16000000 / 115200; // This is actually not exact because we are leaving the fraction part for now
    UART4->CR1 = BIT(3) | BIT(2) | BIT(13);

    // Main Loop
    while (1) {
        // Now it is "non blocking"
        // Though still this is a polling
        if (timer_expired(&timer_green, period, s_ticks)) {
            static bool LED_green_bool = true;
            GPIO_BSRR_writeBit(LED_green, LED_green_bool);
            LED_green_bool = !LED_green_bool;
        }
    }
    
   
    return 0;
}
