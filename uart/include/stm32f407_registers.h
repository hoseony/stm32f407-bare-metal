#ifndef STM32F407_REGISTERS_H
#define STM32F407_REGISTERS_H

#include <stdint.h>

// ------------ GPIO -------------
// Reference Manual 8.4 
// GPIO registers
typedef struct {
    volatile uint32_t MODER;
    volatile uint32_t OTYPER;
    volatile uint32_t OSPEEDR;
    volatile uint32_t PUPDR;
    volatile uint32_t IDR;
    volatile uint32_t ODR;
    volatile uint32_t BSRR;
    volatile uint32_t LCKR;
    volatile uint32_t AFRL;
    volatile uint32_t AFRH;
} GPIO_t;

// Reference Manual 8.4.1
// MODER register uses the following enum to control GPIO mode
// makes my life so much easier!
enum {
    GPIO_MODE_INPUT,    // 00: Digital input
    GPIO_MODE_OUTPUT,   // 01: Digital Output
    GPIO_MODE_AF,       // 10: Alternate Function (pin is hadned over to a peripheral)
                        //     e.g. uart, psi, i2c, ...
    GPIO_MODE_ANALOG    // 11: Analog input
};

// Reference Manual 2.2
#define GPIO(bank) ((GPIO_t *)(0x40020000 + 0x400 * (bank))) // calculating GPIO bank

// ------------ RCC -------------

// Reference Manual 7.3.24
// RCC registers
typedef struct {
    volatile uint32_t CR;
    volatile uint32_t PLLCFGR;
    volatile uint32_t CFGR;
    volatile uint32_t CIR;
    volatile uint32_t AHB1RSTR;
    volatile uint32_t AHB2RSTR;
    volatile uint32_t AHB3RSTR;
    volatile uint32_t _RESERVED0;    // reserved

    volatile uint32_t APB1RSTR;
    volatile uint32_t APB2RSTR;
    volatile uint32_t _RESERVED1[2]; // reserved

    volatile uint32_t AHB1ENR;
    volatile uint32_t AHB2ENR;
    volatile uint32_t AHB3ENR;
    volatile uint32_t _RESERVED2;    // reserved

    volatile uint32_t APB1ENR;
    volatile uint32_t APB2ENR;
    volatile uint32_t _RESERVED3[2]; // reserved

    volatile uint32_t AHB1LPENR;
    volatile uint32_t AHB2LPENR;
    volatile uint32_t AHB3LPENR;
    volatile uint32_t _RESERVED4;    // reserved

    volatile uint32_t APB1LPENR;
    volatile uint32_t APB2LPENR;
    volatile uint32_t _RESERVED5[2]; // reserved

    volatile uint32_t BDCR;
    volatile uint32_t CSR;
    volatile uint32_t _RESERVED6[2]; // reserved

    volatile uint32_t SSCGR;
    volatile uint32_t PLLI2SCFGR;
} RCC_t;

// Reference Manual 2.2
#define RCC ((RCC_t *)(0x40023800))

// ------------ SysTick -------------
// https://developer.arm.com/documentation/ddi0413/d/system-control/about-system-control
typedef struct {
    volatile uint32_t CTRL;
    volatile uint32_t LOAD;
    volatile uint32_t VAL;
    volatile uint32_t CALI;
} SYSTICK_t;

// ------------ USART / UART -------------
// Reference Manual 30.6.8
typedef struct {
    volatile uint32_t SR;
    volatile uint32_t DR;
    volatile uint32_t BRR;
    volatile uint32_t CR1;
    volatile uint32_t CR2;
    volatile uint32_t CR3;
    volatile uint32_t GTPR;
} USART_t;

// Reference Manual 2.3           // To Enable the peripheral (RCC Register, Bit)
#define UART8  ((USART_t *)(0x40007c00)) // APB1ENR, 31
#define UART7  ((USART_t *)(0x40007800)) // APB1ENR, 30
#define USART6 ((USART_t *)(0x40011400)) // APB2ENR, 5
#define UART5  ((USART_t *)(0x40005000)) // APB1ENR, 20
#define UART4  ((USART_t *)(0x40004c00)) // APB1ENR, 19
#define USART3 ((USART_t *)(0x40004800)) // APB1ENR, 18
#define USART2 ((USART_t *)(0x40004400)) // APB1ENR, 17
#define USART1 ((USART_t *)(0x40011000)) // APB2ENR, 4

#endif
