#include <stdint.h>
#include <inttypes.h>
#include <stdbool.h>

// ------------ Definitions -------------
// * todo: Move this to a header file 

// Reference Manual 8.4 
// GPIO registers
typedef struct {
    volatile uint32_t MODER;
    volatile uint32_t OTYPER;
    volatile uint32_t OSPEEDR;
    volatile uint32_t PURDR;
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
#define GPIOA ((gpio *) 0x40020000)
#define GPIOB ((gpio *) 0x40020400)
#define GPIOC ((gpio *) 0x40020800)
#define GPIOD ((gpio *) 0x40020C00)
#define GPIOE ((gpio *) 0x40021000)
#define GPIOF ((gpio *) 0x40021400)
#define GPIOG ((gpio *) 0x40021800)
#define GPIOH ((gpio *) 0x40021C00)
#define GPIOI ((gpio *) 0x40022000)
#define GPIOJ ((gpio *) 0x40022400)
#define GPIOK ((gpio *) 0x40022800)

// ------------ function prototypes -------------


// ------------ Main Instructions -------------

int main(void) {
    uint32_t cnt = 0;
    uint32_t half;

    while(1) {
        cnt += 2;
        half = cnt / 2;
        ++half;
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
