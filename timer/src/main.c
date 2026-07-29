#include "../include/systick.h"
#include "../include/gpio.h"
#include "../include/utils.h"
//#include "../include/uart.h"
#include "../include/stm32f407_registers.h"
#include "../include/nvic.h"

// ---------- Function Prototype ----------

// ----------------------------------------

// I had to expose this pin so that
// interrupt handler can access these
uint16_t LED_green = PIN('D', 12);
volatile bool LED_green_bool = false;

int main(void) {
    // ========== blinky ==========
    RCC->AHB1ENR |= BIT(3);
    GPIO_setMode(LED_green, GPIO_MODE_OUTPUT);
    SysTick_init(16000000 / 1000);

    // uint32_t timer_green= 0;
    // uint32_t period = 500;

    // ========== TIMER ==========
   
    // INITIALIZATION:
    
    // enable clock for TIM2
    RCC->APB1ENR |= BIT(0);
    
    // set PSC (idk some random number)
    // system clock: 16000000
    TIM2->PSC = 1600; 
    TIM2->ARR = 5000; // This will be 0.5 sec (I think????)
   
    // enable update interrupt 
    TIM2->DIER |= BIT(0);

    // setup NVIC
    NVIC_enableIRQ(28);

    // enable timer 
    TIM2->CR1 |= BIT(0);

    // setup LED
    GPIO_BSRR_writeBit(LED_green, LED_green_bool);


    // ========= Main Loop ==========
    while (1) {
/*
        if (timer_expired(&timer_green, period, s_ticks)) {
            // setting up LED
            static bool LED_green_bool = false;

            GPIO_BSRR_writeBit(LED_green, LED_green_bool);
        }
*/
    }
    
    return 0;
}
