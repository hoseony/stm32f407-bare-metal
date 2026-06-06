#include <stdint.h>

// ------------ function prototypes -------------
int main(void);
// ----------------------------------------------

extern void _estack(void); // stm32f407.ld

/* __attribute__()
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
 */

// Startup code
__attribute__((naked, noreturn)) void _reset(void) {
  // memset .bss to zero, and copy .data section to RAM region
  extern long _sbss, _ebss, _sdata, _edata, _sidata;
  for (long *dst = &_sbss; dst < &_ebss; dst++) *dst = 0;
  for (long *dst = &_sdata, *src = &_sidata; dst < &_edata;) *dst++ = *src++;

  main();             // Call main()
  for (;;) (void) 0;  // Infinite loop in the case if main() returns
}

// 16 standard and 91 STM32-specific handlers
__attribute__((section(".vectors"))) void (*const tab[16 + 91])(void) = {_estack, _reset};

int main() {
    while(1) {

    }
}


