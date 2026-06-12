#ifndef SYSTICK_H
#define SYSTICK_H

#include <stdint.h>
#include <stdbool.h>
#include "../include/stm32f407_registers.h"

#define SYSTICK ((SYSTICK_t *)(0xe000e010))

extern volatile uint32_t s_ticks;

void SysTick_init(uint32_t ticks);
void SysTick_Handler(void);
void delay(uint32_t ma);
bool timer_expired(uint32_t *t, uint32_t period, uint32_t now);

#endif
