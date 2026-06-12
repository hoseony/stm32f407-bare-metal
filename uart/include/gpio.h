#ifndef GPIO_H
#define GPIO_H
#include <stdint.h>
#include <stdbool.h>
#include "stm32f407_registers.h"

void GPIO_setMode(uint16_t pin, uint8_t mode);
void GPIO_BSRR_writeBit(uint16_t pin, bool val);

#endif
