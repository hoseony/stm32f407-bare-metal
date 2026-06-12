#include "../include/gpio.h"
#include "../include/utils.h"

// input: packed "pin", enum mode
void GPIO_setMode(uint16_t pin, uint8_t mode) {
    GPIO_t *gpio = GPIO(PINBANK(pin)); // calculate GPIO bank
    uint8_t num = PINNO(pin);          // unpacking

    // reference manual 8.4.1
    gpio->MODER &= ~(0b11U << (num * 2));                   // clear it first
    gpio->MODER |= ((uint32_t)(mode & 0b11U)) << (num * 2); // Then set the mode
}

void GPIO_BSRR_writeBit(uint16_t pin, bool val) {
    GPIO_t *gpio = GPIO(PINBANK(pin));
    uint8_t num = PINNO(pin);

    if (val) {
        gpio->BSRR = BIT(num);
    } else {
        gpio->BSRR = BIT(num + 16);
    }
}

