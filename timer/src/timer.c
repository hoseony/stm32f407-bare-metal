#include "../include/utils.h"
#include "../include/stm32f407_registers.h"
#include "../include/gpio.h"
#include "../include/nvic.h"
#include "../include/timer.h"

#include "stdint.h"
#include "stdbool.h"

extern uint16_t LED_green;
extern volatile bool LED_green_bool;

void TIM2_IRQHandler(void) {
    if (TIM2->SR & BIT(0)) { 
        // if update interrupt flag bit is 1 (pending)
        // This must be cleared by software (Reference Manual 18.4.5)
        TIM2->SR &= ~BIT(0); // like this

        GPIO_BSRR_writeBit(LED_green, LED_green_bool);
        LED_green_bool = !LED_green_bool;
    }
}

// I think this is a bit unintuitive, maybe inputting getitng an input 
// as Hz makes more sense.
bool TIM_GEN_init(TIM_GEN_t* TIM, uint32_t PSC, uint32_t ARR, bool updateInterrupt) {
    unsigned int irq;

    if (TIM == TIM2) {
        RCC->APB1ENR |= BIT(0);
        irq = 28;
    } else if (TIM == TIM3) {
        RCC->APB1ENR |= BIT(1);
        irq = 29;
    } else if (TIM == TIM4) {
        RCC->APB1ENR |= BIT(2);
        irq = 30;
    } else {
        return false;
    }

    // set PSC and ARR
    TIM->PSC = PSC;
    TIM->ARR = ARR;

    if (updateInterrupt) {
        TIM->DIER |= BIT(0);
        NVIC_enableIRQ(irq);
    }

    // enable timer
    TIM->CR1 |= BIT(0);

    return true;
}
