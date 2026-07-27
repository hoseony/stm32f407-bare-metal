#include "../include/systick.h"
#include "../include/uart.h"
#include "../include/timer.h"

// ------------ Startup Code -------------

extern void _estack(void); // stm32f407.ld
extern int main(void);

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
    [0] = (void (*)(void))(&_estack),
    [1] = _reset,

    [15] = SysTick_Handler,

    // UART & USART handlers
    [16 + 37] = USART1_IRQHandler,
    [16 + 38] = USART2_IRQHandler,
    [16 + 39] = USART3_IRQHandler,
    [16 + 52] = UART4_IRQHandler,
    [16 + 53] = UART5_IRQHandler,
    [16 + 71] = USART6_IRQHandler,

    [16 + 28] = TIM2_IRQHandler,
};
