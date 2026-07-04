#ifndef NVIC_H
#define NVIC_H

#include "stm32f407_registers.h"
#include "stdint.h"

void NVIC_enableIRQ(uint32_t irqn);
void NVIC_setPriority(uint32_t irqn, uint8_t priority);

#endif
