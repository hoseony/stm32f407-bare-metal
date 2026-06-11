#include <inttypes.h>
#include <stdbool.h>
#include "../include/stm32f407_registers.h"

// ------------ Helper Functions -------------

#define BIT(x) (1UL << (x)) // just for the convinience

// input: packed "pin", enum mode
static inline void GPIO_setMode(uint16_t pin, uint8_t mode) {
    GPIO_t *gpio = GPIO(PINBANK(pin)); // calculate GPIO bank
    uint8_t num = PINNO(pin);          // unpacking

    // reference manual 8.4.1
    gpio->MODER &= ~(0b11U << (num * 2));                   // clear it first
    gpio->MODER |= ((uint32_t)(mode & 0b11U)) << (num * 2); // Then set the mode
}

static inline void GPIO_BSRR_writeBit(uint16_t pin, bool val) {
    GPIO_t *gpio = GPIO(PINBANK(pin));
    uint8_t num = PINNO(pin);

    if (val) {
        gpio->BSRR = BIT(num);
    } else {
        gpio->BSRR = BIT(num + 16);
    }
}

static inline void SysTick_init(uint32_t ticks) {
    if ((ticks - 1) > 0xffffff) { // 
        return;
    }

    SYSTICK->LOAD = ticks - 1;
    SYSTICK->VAL = 0; // Writing anything to this register resets it
    SYSTICK->CTRL = BIT(0) | BIT(1) | BIT(2);
    // enable | counting down to 0  send interrupt| processor clock
    
    //RCC->APB2ENR |= BIT(14);
}

// ------------ Main Instructions -------------

void counter(uint32_t count) {
    for (uint32_t i = 0; i <= count; i++) {
        ;
    }
}

// we can do better than just for loop (we want it to be more accurate)
// Let's use the system clock 
// Note that for stm32f407, the default clock is 16MHz

// Here s_ticks is now a clock that is driven by interrupt called SysTick
static volatile uint32_t s_ticks;

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

int main(void) {
    // Let's try blinking the built-in LEDs 

    // We can see from User Manual 4.4 that 
    //  - PD13: orange LED
    //  - PD12: green LED
    //  - PD14: RED LED
    //  - PD15: blue LED

    // To use this, we need to enable GPIO D...

    // RCC_AHB1ENR has IO port A-I clock enable bits
    // (bit 0: A, bit 1: B, ..., bit 8: I)

    uint16_t LED_blue = PIN('D', 15);
    RCC->AHB1ENR |= BIT(3);
    GPIO_setMode(LED_blue, GPIO_MODE_OUTPUT);

    uint16_t LED_green = PIN('D', 12);
    GPIO_setMode(LED_green, GPIO_MODE_OUTPUT);

    
    // Reference Manual 8.4.6 8.4.7
    // to turn LED on and off, you need to modify ODR register 
    // you can do this by modifying BSRR register

    /*
    while (1) {
        GPIO_BSRR_writeBit(LED_blue, 1);
        counter(1000000);
        GPIO_BSRR_writeBit(LED_blue, 0);
        counter(1000000);
    } 
    */
    
    // That's all good, but polling is not so cool.
    // Let's do something better

    SysTick_init(16000000 / 1000);

    /*
    while (1) {
        GPIO_BSRR_writeBit(LED_blue, 1);
        delay(500);
        GPIO_BSRR_writeBit(LED_blue, 0);
        delay(500);
    } 
    */

    // A little bit better. However, what we eventually want 
    // is non-blocking functions where cpu is not stuck at delay
    // Let's do that

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

// ------------ Startup Code -------------

extern void _estack(void); // stm32f407.ld

// _reset handler
__attribute__((naked, noreturn)) void _reset(void) {
    // we have defined these variables in stm32f407.ld
    extern long _sbss, _ebss, _sdata, _edata, _sidata;

    // setting bss to 0
    for (long *dst = &_sbss; dst < &_ebss; dst++) *dst = 0;
    // copying .data initialized in flash to .data in memory
    for (long *dst = &_sdata, *src = &_sidata; dst < &_edata;) *dst++ = *src++;

    main();             // Call main()
    for (;;) (void) 0;  // Infinite loop in the case if main() returns
}

// 16 standard and 91 STM32-specific handlers
// The 16 standard Interrupts are from ARM Cortex-M4 Generic User Guide 
// 0s are placeholder (for now)

__attribute__((section(".vectors"))) void (*const tab[16 + 91])(void) = {
    _estack, _reset, 0, 0, 0, 
    0, 0, 0, 0, 0, 
    0, 0, 0, 0, 0, 
    SysTick_Handler};
