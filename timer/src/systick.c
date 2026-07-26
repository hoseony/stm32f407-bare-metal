#include "../include/systick.h"
#include "../include/utils.h"

volatile uint32_t s_ticks;

void SysTick_init(uint32_t ticks) {
    if ((ticks - 1) > 0xffffff) {
        return;
    }

    SYSTICK->LOAD = ticks - 1;
    SYSTICK->VAL = 0; // Writing anything to this register resets it
    SYSTICK->CTRL = BIT(0) | BIT(1) | BIT(2);
    // enable | counting down to 0  send interrupt| processor clock
    
    //RCC->APB2ENR |= BIT(14);
}

// Note that for stm32f407, the default clock is 16MHz
// Here s_ticks is now a clock that is driven by interrupt called SysTick

void SysTick_Handler(void) {
    s_ticks++;
}

void delay(uint32_t ms) {
    uint32_t until = s_ticks + ms;
    while (s_ticks < until) {
        ;
    }
}

bool timer_expired(uint32_t *t, uint32_t period, uint32_t now) {
    if (now + period < *t) { // checking wrap around
        *t = 0;
    }

    if (*t == 0) { // Initizlization
        *t = now + period; 
    }

    if (*t > now) { // timer not yet reached
        return false; 
    }

    if ( (now - *t) > period) { // If it has passed the more than one period
        *t = now + period;      // re-initialize
    } else {
        *t = *t + period; // Otherwise, this becomes the next time it checks
    }

    return true;
}
