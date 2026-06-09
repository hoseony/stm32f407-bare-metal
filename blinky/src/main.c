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

// ------------ Main Instructions -------------

void counter(uint32_t count) {
    for (uint32_t i = 0; i <= count; i++) {
        ;
    }
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

    // Reference Manual 8.4.6 8.4.7
    // to turn LED on and off, you need to modify ODR register 
    // you can do this by modifying BSRR register

    while(1) {
        GPIO_BSRR_writeBit(LED_blue, 1);
        counter(1000000);
        GPIO_BSRR_writeBit(LED_blue, 0);
        counter(1000000);
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
__attribute__((section(".vectors"))) void (*const tab[16 + 91])(void) = {_estack, _reset};
