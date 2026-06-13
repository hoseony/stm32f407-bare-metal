#include <inttypes.h>

#include "../include/systick.h"
#include "../include/gpio.h"
#include "../include/utils.h"
#include "../include/uart.h"

// ------------ Main Instructions -------------

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

    GPIO_setMode(uart_tx, GPIO_MODE_AF);
    GPIO_setMode(uart_rx, GPIO_MODE_AF);
    

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
