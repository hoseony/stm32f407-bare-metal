#include "../include/utils.h"
#include "../include/stm32f407_registers.h"
#include "../include/gpio.h"
#include "../include/nvic.h"
#include "../include/timer.h"

#include "stdint.h"
#include "stdbool.h"

// after all, I need some constnats :(
#define HSI_HZ 16000000U // this is internal to the board
#define HSE_HZ 8000000U

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

// I think this is a bit unintuitive, maybe inputting getitng an input 
// as Hz makes more sense. To do that we need to figure out 
// how to calculate timer clock first.

/* uh.... how do I get the system clock speed?
 * ok! I found it RCC->CFGR Bits 3:2 SWS 
 *  - 00: HSI oscillator
 *  - 01: HSE oscillator
 *  - 10: PLL (phase-locked loop)
 *  - 11: not applicable
 * 
 * ok so what is PLL?
 *  
 *
 * Then, you need to get prescaler
 *  - RCC->CFGR -> HPRE (AHB prescaler)
 *  - RCC-CFGR  -> PPREx (PPRE high-speed prescaler)
 *      - PPRE1 for low-speed, PPRE2 for high-speed
 *
 * WHATS THE DIFFERENCE??
 *  on the Clock Tree (figure), SW choose the mux of HSI, HSE, PLLCLK
 *  and that goes into "AHB PRESC" and then "APB PRESC."
 *
 * Timer clocks depend on SYSCLK source, AHB prescaler, APB prescaler, and x2 depending on APBxpresc (if 1, x1; else x2). 
 *
 *  So the flow goes like 
 *  SYSCLK -> HCLK -> PCLK1 / PCLK2
 *   1. sysclk (either of HSI, HSE, or PLL)
 *   2. HCLK (clock after AHB PRESC) = SYSCLK / AHB_PRESC
 *   3. PCLK (peripheral clock) (there's 1 and 2 for APB1 and APB2)
 *
 * Alr, let's start coding!!
 */

uint32_t RCC_getSYSCLK(void) {
    uint32_t sws = (RCC->CFGR >> 2) & 0b11U;

    if (sws == 0b00U) {
        return HSI_HZ;
    } else if (sws == 0b01U) {
        return HSE_HZ;
    } else {
        return HSI_HZ;
    }
}





/*
bool TIM_GEN_initHz(TIM_GEN_t * TIM, uint32_t hz, bool updateInterrupt) {

    TIM_GEN_init(TIM, );
}
*/
