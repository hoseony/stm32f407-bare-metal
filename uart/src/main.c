#include <inttypes.h>

#include "../include/systick.h"
#include "../include/gpio.h"
#include "../include/utils.h"

// ------------ Main Instructions -------------

void counter(uint32_t count) {
    for (uint32_t i = 0; i <= count; i++) {
        ;
    }
}

// we can do better than just for loop (we want it to be more accurate)
// Let's use the system clock 


int main(void) {
    // Let's try blinking the built-in LEDs 

    // We can see from User Manual 4.4 that 
    //  - PD13: orange LED
    //  - PD12: green LED
    //  - PD14: RED LED
    //  - PD15: blue LED

    uint16_t LED_blue = PIN('D', 15);
    uint16_t LED_green = PIN('D', 12);

    RCC->AHB1ENR |= BIT(3);

    GPIO_setMode(LED_blue, GPIO_MODE_OUTPUT);
    GPIO_setMode(LED_green, GPIO_MODE_OUTPUT);

    SysTick_init(16000000 / 1000);

    uint32_t timer_blue = 0;
    uint32_t timer_green= 0;
    uint32_t period = 500;
    
    while (1) {
        // Now it is "non blocking"
        // Though still this is a polling
        if (timer_expired(&timer_blue, period, s_ticks)) {
            static bool LED_blue_bool = true;
            GPIO_BSRR_writeBit(LED_blue, LED_blue_bool);
            LED_blue_bool = !LED_blue_bool;
        }

        if (timer_expired(&timer_green, period, s_ticks)) {
            static bool LED_green_bool = true;
            GPIO_BSRR_writeBit(LED_green, LED_green_bool);
            LED_green_bool = !LED_green_bool;
        }
    }
   
    return 0;
}
