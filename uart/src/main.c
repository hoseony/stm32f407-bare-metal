#include "../include/systick.h"
#include "../include/gpio.h"
#include "../include/utils.h"
#include "../include/uart.h"

// ---------- Function Prototype ----------
void GPIO_setAF(uint16_t pin, uint8_t AFnum);
bool checkMatch(uint8_t *buf, uint8_t *bytes, uint32_t size);

void NVIC_enableIRQ(uint32_t irqn);
void NVIC_setPriority(uint32_t irqn, uint8_t priority);
void UART_init(USART_t *uart, uint32_t baudRate, bool rxInterruptEnable, uint8_t priority);
// ----------------------------------------

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

/* For now, I might not need to do this? I'll see how it goes 
 * (I am thinking perhasp default is fine)

void GPIO_OTYPER_writeBit(uint16_t pin, bool val) {
    // OTYPER 
    // 0: Output Push-Pull 
    // 1: Output Open-drain

    GPIO_t *gpio = GPIO(PINBANK(pin));
    uint8_t num = PINNO(pin);
}

void GPIO_PUPDR_writeBit(uint16_t pin, uint8_t val) {
    // PUPDR 
    // 00: No pull up, No pull down
    // 01: Pull-up
    // 10: Pull-down
    // 11: Reserved

    GPIO_t *gpio = GPIO(PINBANK(pin));
    uint8_t num = PINNO(pin);
}

void GPIO_OSPEEDR_writeBit(uint16_t pin, uint8_t val) {
    // OSPEEDR
    // 00: Low Speed
    // 01: Medium Speed
    // 10: High Speed
    // 11: Very High Speed

    GPIO_t *gpio = GPIO(PINBANK(pin));
    uint8_t num = PINNO(pin);
}
*/

// ================= NVIC =====================
void NVIC_enableIRQ(uint32_t irqn) {
    NVIC_ISER[irqn / 32] = BIT(irqn % 32);
    // you use = becuase it is a write-one-to-set register
}

void NVIC_setPriority(uint32_t irqn, uint8_t priority) {
    NVIC_IPR[irqn] = priority << 4; // 4 because it doesn't use the entier 8 bit (only uses the 4)
}

// === uart send help ==

void UART_init(USART_t *uart, uint32_t baudRate, bool rxInterruptEnable, uint8_t priority) {
// Initialize uart peripheral register 
// priority is unused if rxInterruptEnable is false

    // initialize default values
    uint32_t pclk = 16000000;
    uint32_t irqn = 0;
    
    // reference ../include/stm32f407_registers.h for more
    // uart 8 and uart 7 are not real thing on this mcu!
    /*if (uart == UART8) {
        RCC->APB1ENR |= BIT(31);
        irqn = IRQn_UART8;

    } else if (uart == UART7) {
        RCC->APB1ENR |= BIT(30);
        irqn = IRQn_UART7;

    } else */ 
    if (uart == USART6) {
        RCC->APB2ENR |= BIT(5);
        irqn = IRQn_USART6;

    } else if (uart == UART5) {
        RCC->APB1ENR |= BIT(20);
        irqn = IRQn_UART5;

    } else if (uart == UART4) {
        RCC->APB1ENR |= BIT(19);
        irqn = IRQn_UART4;

    } else if (uart == USART3) {
        RCC->APB1ENR |= BIT(18);
        irqn = IRQn_USART3;

    } else if (uart == USART2) {
        RCC->APB1ENR |= BIT(17);
        irqn = IRQn_USART2;

    } else if (uart == USART1) {
        RCC->APB2ENR |= BIT(4);
        irqn = IRQn_USART1;
    }

    uart->CR1 = 0;
    uart->CR2 = 0;
    uart->CR3 = 0;

    // actual ok way of doing the baudRate
    uart->BRR = (pclk + (baudRate / 2U))/ baudRate;
    uart->CR1 = BIT(3) | BIT(2) | BIT(13);

    if (rxInterruptEnable) {
        uart->CR1 |= BIT(5);
        NVIC_setPriority(irqn, priority);
        NVIC_enableIRQ(irqn);
    }
}


bool UART_getPinAF(USART_t *uart, uint16_t pin, bool isRX, uint8_t *af);

bool UART_gpioInit(USART_t *uart, uint16_t tx, uint16_t rx) {
// receive tx & rx -> gpio enable, gpio set to AF, set AF number (maybe this is too much???)
// to do this eaiser, let's make function to get AF...

    uint8_t txAF, rxAF;

    // Check if txAF and rxAF is available (and save it if is)
    if (!UART_getPinAF(uart, tx, false, &txAF)) {
        return false;
    } 

    if (!UART_getPinAF(uart, rx, true, &rxAF)) {
        return false;
    }

    GPIO_enable(tx);
    GPIO_enable(rx);

    GPIO_setMode(tx, GPIO_MODE_AF);
    GPIO_setMode(rx, GPIO_MODE_AF);

    GPIO_setAF(tx, txAF);
    GPIO_setAF(rx, rxAF);

    return true;
}

bool UART_getPinAF(USART_t *uart, uint16_t pin, bool isRX, uint8_t *af) {
// function that returns af number for gpioInit

        if (af == NULL) {
            return false;
        }

        if (uart == USART1) {
            // from Datasheet - Table 9
            // tx - PA9, PB6
            // rx - PA10 PB7

            if(!isRX && (pin == PIN('A', 9) || pin == PIN('B', 6))) { // for tx
                *af = 7;
                return true;
            }

            if(isRX && (pin == PIN('A', 10) || pin == PIN('B', 7))) { // for rx
                *af = 7;
                return true;
            }
        }

        else if (uart == USART2) {
            if(!isRX && (pin == PIN('A', 2) || pin == PIN('D', 5))) {
                *af = 7;
                return true;
            }

            if(isRX && (pin == PIN('A', 3) || pin == PIN('D', 6))) {
                *af = 7;
                return true;
            }
        }

        else if (uart == USART3) {
            if(!isRX && (pin == PIN('B', 10) || pin == PIN('D', 8))) {
                *af = 7;
                return true;
            }

            if(isRX && (pin == PIN('B', 11) || pin == PIN('D', 9))) {
                *af = 7;
                return true;
            }
        }

        else if (uart == UART4) {
            if(!isRX && (pin == PIN('A', 0) || pin == PIN('C', 10))) {
                *af = 8;
                return true;
            }

            if(isRX && (pin == PIN('A', 1) || pin == PIN('C', 11))) {
                *af = 8;
                return true;
            }
        }

        else if (uart == UART5) {
            if(!isRX && (pin == PIN('C', 12))) {
                *af = 8;
                return true;
            }

            if(isRX && (pin == PIN('D', 2))) {
                *af = 8;
                return true;
            }
        }

        else if (uart == USART6) {
            if(!isRX && (pin == PIN('C', 6) || pin == PIN('G', 14))) {
                *af = 8;
                return true;
            }

            if(isRX && (pin == PIN('C', 7) || pin == PIN('G', 9))) {
                *af = 8;
                return true;
            }
        }

        return false;
}

// =============================================

int main(void) {
    
    // ========== blinky ==========
    uint16_t LED_green = PIN('D', 12);
    uint16_t LED_red = PIN('D', 14);

    RCC->AHB1ENR |= BIT(3);

    GPIO_setMode(LED_green, GPIO_MODE_OUTPUT);
    GPIO_setMode(LED_red , GPIO_MODE_OUTPUT);

    SysTick_init(16000000 / 1000);

    uint32_t timer_green= 0;
    uint32_t period = 500;
    // ========== USART ==========
    // Let's use these for now
    //  - PA0 AF8, UART4_TX
    //  - PA1 AF8, UART4_RX
    // You can find this from Datasheet Table. 9

    uint16_t uart_tx = PIN('A', 0);
    uint16_t uart_rx = PIN('A', 1);

    UART_gpioInit(UART4, uart_tx, uart_rx);
    UART_init(UART4, 115200, true, 5);

    // ========= Main Loop ==========
    while (1) {
        if (timer_expired(&timer_green, period, s_ticks)) {
            static bool LED_green_bool = false;
            static bool LED_red_bool = false;

            GPIO_BSRR_writeBit(LED_green, LED_green_bool);
            GPIO_BSRR_writeBit(LED_red, LED_red_bool);

            uint8_t bytes[] = "hello";
            uint32_t size = sizeof(bytes) - 1;
            uint8_t buf[sizeof(bytes) - 1];

            // Let's make interrupt based uart communication
            // As mentioned on the comment of the commit, 
            // The hardware can only hold 1 byte, so I can not send multiple bytes 
            // and hope it is stored inside some FIFO register 
            
            // For that, let's head to Reference Manual 30.4 USART interrupts
            // There is RXNE (received Data Ready to be Read)
            // Once this event happens, we immidetly need to read the data

            // One way of achieving this is by using "ring buffer" 
            // to store incoming data.

            // Then, you need to enable NVIC (nestied vector interrupt controller)
            // And make IRQ stuff

            // First, let's clean up the Ring buffer

            uint8_t temp;
            while (RING_pop(&UART4_ringBuffer, &temp)) {
                ;
            }

            // This should fire interrupt
            // (the interrupt handler pushes it to the ringBuffer)
            UART_transmitBytes(UART4, bytes, size);

            for (uint32_t i = 0; i < size;) {
                if (RING_pop(&UART4_ringBuffer, &buf[i])) {
                    i++;
                }
            }

            if (checkMatch(bytes, buf, size)) {
                LED_green_bool = !LED_green_bool;
            } else {
                LED_red_bool = !LED_red_bool;
            }
        }
    }
    
    return 0;
}

bool checkMatch(uint8_t *buf, uint8_t *bytes, uint32_t size) {
    bool match = true;
    for (uint32_t i = 0; i < size; i++) {
        if (buf[i] != bytes[i]) {
            return false;
        }
    }
    
    return match;
}
