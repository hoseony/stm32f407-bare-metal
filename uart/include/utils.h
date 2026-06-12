#ifndef UTILS_H
#define UTILS_H

#define BIT(x) (1UL << (x))

#define PIN(bank, num) (((bank - 'A') << 8) | (num))         // packing things in pin
#define PINNO(pin) (uint8_t)(pin & 255)                      // lower bits
#define PINBANK(pin) (pin >> 8)                              // upper bits
/* This way, you can do something like    *
 *   uint16_t pin = PIN('A', 3);          *
 *   GPIO_setMode(pin, GPIO_MODE_OUTPUT); */

#endif
