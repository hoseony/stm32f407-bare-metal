#include "../include/gpio.h"
#include "../include/utils.h"

// ---------- Function Prototype ----------
void GPIO_setMode(uint16_t pin, uint8_t mode);
void GPIO_BSRR_writeBit(uint16_t pin, bool val);
void GPIO_setAF(uint16_t pin, uint8_t AFnum);
void GPIO_enable(uint16_t pin);
// ----------------------------------------

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

void GPIO_setAF(uint16_t pin, uint8_t AFnum) {
    // When using AF, you need to set the AF number 
    // with either AFLR(for pin 0..7) or AFHR(for pin 8..15)

    GPIO_t *gpio = GPIO(PINBANK(pin));
    uint8_t num = PINNO(pin);

    if (num > 7) { // use AFRH
        gpio->AFRH &= ~(0b1111U << ((num - 8) * 4));
        gpio->AFRH |= ((uint32_t)AFnum << ((num - 8) * 4));
    } else { // use AFRL
        gpio->AFRL &= ~(0b1111U << (num * 4));
        gpio->AFRL |= ((uint32_t)AFnum << (num * 4));
    }
}

void GPIO_enable(uint16_t pin) {
    RCC->AHB1ENR |= BIT(PINBANK(pin));
}
