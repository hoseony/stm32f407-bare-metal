#include <stdint.h>

// ------------ function prototypes -------------
// ----------------------------------------------

/* SOME NOTES...
 * 1. __attribute__()
 *
 * __attribute__((__aligned__(x)))
 * it aligns memory address to multiple of x 
 *
 * __attribute__((weak))
 * It allows you to override the symbol
 *
 * __attribute__((section("section_name"))): 
 * It forces stuff into a specific section in the object file
 * 
 * There are more stuff you can do, but I only put here things that might useful to me
 *  - naked: omit standard function prologue and epilogue sequence (when generating asesembly)
 *  - noreturn: this function never returns
 */

int main(void) {
    uint32_t cnt = 0;
    uint32_t half;

    while(1) {
        cnt += 2;
        half = cnt / 2;
        ++half;
    }
}

extern void _estack(void); // stm32f407.ld

// Startup code

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
