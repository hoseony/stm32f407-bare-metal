#include <stdint.h>
#include <inttypes.h>
#include <stdbool.h>

// ------------ Definitions -------------
// * todo: Move this to a header file 


// ------------ GPIO -------------
// Reference Manual 8.4 
// GPIO registers
typedef struct {
    volatile uint32_t MODER;
    volatile uint32_t OTYPER;
    volatile uint32_t OSPEEDR;
    volatile uint32_t PUPDR;
    volatile uint32_t IDR;
    volatile uint32_t ODR;
    volatile uint32_t BSRR;
    volatile uint32_t LCKR;
    volatile uint32_t AFRL;
    volatile uint32_t AFRH;
} GPIO_t;

// Reference Manual 8.4.1
// MODER register uses the following enum to control GPIO mode
// makes my life so much easier!
enum {
    GPIO_MODE_INPUT,    // 00: Digital input
    GPIO_MODE_OUTPUT,   // 01: Digital Output
    GPIO_MODE_AF,       // 10: Alternate Function (pin is hadned over to a peripheral)
                        //     e.g. uart, psi, i2c, ...
    GPIO_MODE_ANALOG    // 11: Analog input
};

// Reference Manual 2.2
#define GPIO(bank) ((GPIO_t *)(0x40020000 + 0x400 * (bank))) // calculating GPIO bank

#define PIN(bank, num) (((bank - 'A') << 8) | (num))         // packing things in pin
#define PINNO(pin) (uint8_t)(pin & 255)                      // lower bits
#define PINBANK(pin) (pin >> 8)                              // upper bits
/* This way, you can do something like    *
 *   uint16_t pin = PIN('A', 3);          *
 *   GPIO_setMode(pin, GPIO_MODE_OUTPUT); */


// ------------ RCC -------------

// Reference Manual 7.3.24
// RCC registers
typedef struct {
    volatile uint32_t CR;
    volatile uint32_t PLLCFGR;
    volatile uint32_t CFGR;
    volatile uint32_t CIR;
    volatile uint32_t AHB1RSTR;
    volatile uint32_t AHB2RSTR;
    volatile uint32_t AHB3RSTR;
    volatile uint32_t _RESERVED0;    // reserved
    volatile uint32_t APB1RSTR;
    volatile uint32_t APB2RSTR;
    volatile uint32_t _RESERVED1[2]; // reserved
    volatile uint32_t AHB1ENR;
    volatile uint32_t AHB2ENR;
    volatile uint32_t AHB3ENR;
    volatile uint32_t _RESERVED2;    // reserved
    volatile uint32_t APB1ENR;
    volatile uint32_t APB2ENR;
    volatile uint32_t _RESERVED3[2]; // reserved
    volatile uint32_t AHB1LPENR;
    volatile uint32_t AHB2LPENR;
    volatile uint32_t AHB3LPENR;
    volatile uint32_t _RESERVED4;    // reserved
    volatile uint32_t APB1LPENR;
    volatile uint32_t APB2LPENR;
    volatile uint32_t _RESERVED5[2]; // reserved
    volatile uint32_t BDCR;
    volatile uint32_t CSR;
    volatile uint32_t _RESERVED6[2]; // reserved
    volatile uint32_t SSCGR;
    volatile uint32_t PLLI2SCFGR;
} RCC_t;

// Reference Manual 2.2
#define RCC ((RCC_t *)(0x40023800))

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

// ------------ Main Instructions -------------

int main(void) {
    // PD13 - orange LED
    // PD12 - green LED
    // PD14 - RED LED
    // PD15 - blue LED


    while(1) {

    }
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
