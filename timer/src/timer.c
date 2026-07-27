#include "../include/utils.h"
#include "../include/stm32f407_registers.h"
#include "../include/gpio.h"
#include "stdint.h"
#include "stdbool.h"
#include "../include/timer.h"

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
