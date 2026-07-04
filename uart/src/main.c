#include "../include/systick.h"
#include "../include/gpio.h"
#include "../include/utils.h"
#include "../include/uart.h"

// ---------- Function Prototype ----------
// Check if contents of *buf and *bytes matches
bool checkMatch(uint8_t *buf, uint8_t *bytes, uint32_t size);

// ----------------------------------------

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
    // ========== UART ==========
    // Let's use these for now
    //  - PA0 AF8, UART4_TX
    //  - PA1 AF8, UART4_RX
    // You can find this from Datasheet Table. 9

    uint16_t uart_tx = PIN('A', 0);
    uint16_t uart_rx = PIN('A', 1);

    UART_gpioInit(UART4, uart_tx, uart_rx);
    UART_init(UART4, 115200, true, 5);

    // ========= Main Loop ==========
    while (1) {
        if (timer_expired(&timer_green, period, s_ticks)) {
            // setting up LED
            static bool LED_green_bool = false;
            static bool LED_red_bool = false;

            GPIO_BSRR_writeBit(LED_green, LED_green_bool);
            GPIO_BSRR_writeBit(LED_red, LED_red_bool);

            // UART 
            uint8_t bytes[] = "hello";
            uint32_t size = sizeof(bytes) - 1;
            uint8_t buf[sizeof(bytes) - 1];
            
            uint8_t temp;
            while (RING_pop(&UART4_ringBuffer, &temp)) {
                ;
            }

            // This should fire interrupt
            // (the interrupt handler pushes it to the ringBuffer)
            UART_transmitBytes(UART4, bytes, size);

            // pop the ringBuffer and put it to a buffer to check 
            // if it is transmitted correctly
            for (uint32_t i = 0; i < size;) {
                if (RING_pop(&UART4_ringBuffer, &buf[i])) {
                    i++;
                }
            }

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
