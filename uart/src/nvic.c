#include "../include/nvic.h"
#include "../include/utils.h"
#include "stddef.h"

void NVIC_enableIRQ(uint32_t irqn);
void NVIC_setPriority(uint32_t irqn, uint8_t priority);

void NVIC_enableIRQ(uint32_t irqn) {
    NVIC_ISER[irqn / 32] = BIT(irqn % 32);
    // you use = becuase it is a write-one-to-set register
}

void NVIC_setPriority(uint32_t irqn, uint8_t priority) {
    NVIC_IPR[irqn] = priority << 4; 
    // 4 because it doesn't use the entier 8 bit (only uses the 4)
}
