# 02 UART

After getting blinky working, I wanted to make the board do some basic communication. The obvious choice was a UART driver.

For this part, I used UART4 on the STM32F407 Discovery board (I do not remember why I chose UART4, but that is what it is):
- TX: PA0, alternate function AF8
- RX: PA1, alternate function AF8
- Baud rate: 115200

For testing, I connected TX and RX together as a loopback. If the byte I send comes back correctly, the green LED toggles. If something does not match, the red LED toggles.

## Starting with One Byte
The first goal was just to send and receive one byte.

```c
UART_transmitByte(UART4, 'A');
uint8_t received = UART_readByte(UART4);

if (received == 'A') {
    LED_green_bool = !LED_green_bool;
} else {
    LED_red_bool = !LED_red_bool;
}
```

At the register level, this mostly means:

1. Enable the GPIO clock.
2. Set PA0 and PA1 to alternate function mode.
3. Select AF8 for PA0 and PA1.
4. Enable the UART4 peripheral clock from RCC.
5. Set the baud rate register.
6. Enable the transmitter, receiver, and UART itself.

For UART4, the important registers are:

- `RCC->APB1ENR`: enables the UART4 peripheral clock.
- `USARTx->BRR`: sets the baud rate.
- `USARTx->CR1`: enables TX, RX, UART, and optionally the RX interrupt.
- `USARTx->SR`: contains status flags like `RXNE` and `TXE`.
- `USARTx->DR`: the data register used for both transmit and receive.

My first blocking functions looked like this:

```c
void UART_transmitByte(USART_t *uart, uint8_t data) {
    while ((uart->SR & BIT(7)) == 0) {
        ;
    }

    uart->DR = data;
}

uint8_t UART_readByte(USART_t *uart) {
    while ((uart->SR & BIT(5)) == 0) {
        ;
    }

    return (uint8_t)(uart->DR & 0xFF);
}
```

`BIT(7)` is `TXE`, which tells me the transmit data register is empty. `BIT(5)` is `RXNE`, which tells me received data is ready to read.

This worked for sending one byte, and it was a good first step to check that the GPIO alternate function setup, UART clock, baud rate, and loopback wiring were all correct.

## Multiple-Byte
One byte worked, so the next step was sending and receiving multiple bytes.

This caused some problems because the UART data register is not a string buffer. I cannot write several bytes and expect the hardware to remember all of them for me.

To solve this, I started using interrupt-based UART communication. The blocking transmit side is still simple because I can wait for `TXE` before writing each byte:

```c
void UART_transmitBytes(USART_t *uart, uint8_t *data, uint32_t size) {
    for (uint32_t i = 0; i < size; i++) {
        while ((uart->SR & BIT(7)) == 0) {
            ;
        }

        uart->DR = data[i];
    }
}
```

Receiving is different. If bytes arrive and I do not read them quickly enough, I can lose data. The microcontroller needs to immediately check when the data comes in. This is where interrupts make more sense.

## RXNE Interrupt

The useful receive interrupt is `RXNE`: receive data register not empty.

The idea:

1. A byte arrives.
2. The UART sets `RXNE`.
3. The interrupt handler runs.
4. The handler reads `DR`.
5. The byte gets stored in a software buffer.
6. The main loop can read from that buffer later.

For UART4, the handler looks like this (for others it basically looks the same):

```c
void UART4_IRQHandler(void) {
    if (UART4->SR & BIT(5)) {
        uint8_t byte = (uint8_t)(UART4->DR);
        RING_push(&UART4_ringBuffer, byte);
    }
}
```

## Ring Buffer

For the software buffer, I used a ring buffer.

A ring buffer has:
- `data`: fixed-size byte array
- `head`: where data is popped from
- `tail`: where new data is pushed

When `head == tail`, the buffer is empty. When moving `tail` forward would make it equal to `head`, the buffer is full.

```c
typedef struct {
    uint8_t data[RINGBUFFER_SIZE];
    uint32_t head;
    uint32_t tail;
} RINGBUFFER_t;
```

This is my implementation:

```c
bool RING_push(RINGBUFFER_t *rb, uint8_t byte) {
    uint32_t nextTail = (rb->tail + 1) % RINGBUFFER_SIZE;

    if (nextTail == rb->head) {
        return false;
    }

    rb->data[rb->tail] = byte;
    rb->tail = nextTail;
    return true;
}
```

Popping does the same thing from the `head` side:

```c
bool RING_pop(RINGBUFFER_t *rb, uint8_t *byte) {
    if (rb->head == rb->tail) {
        return false;
    }

    *byte = rb->data[rb->head];
    rb->head = (rb->head + 1) % RINGBUFFER_SIZE;
    return true;
}
```

## NVIC Setup

Adding the UART interrupt also meant I needed to deal with NVIC (nested vector interrupt controller).

For UART4, the IRQ number is 52:

```c
#define IRQn_UART4 52
```

The driver enables the interrupt through helper functions:

```c
NVIC_setPriority(irqn, priority);
NVIC_enableIRQ(irqn);
```

## Testing with OpenOCD and GDB

I tested this with OpenOCD and GDB. Since the OpenOCD command is easy to forget, I usually make an alias:

```sh
alias openocd-stm32f407d='openocd -f interface/stlink.cfg -f target/stm32f4x.cfg'
```

Then I use another terminal for GDB:

```gdb
arm-none-eabi-gdb firmware.elf
target remote :3333
monitor reset halt
load
break main
continue
```

For checking the ring buffer, GDB commands were really useful:

```gdb
(gdb) define rb
>p UART4_ringBuffer.head
>p UART4_ringBuffer.tail
>x/16cb UART4_ringBuffer.data
>end
```

Example output:

```gdb
$11 = 36
$12 = 40
0x20000004 <UART4_ringBuffer>: 111 'o' 104 'h' 101 'e' 108 'l'
```

This was a nice way to see that bytes were actually being received by the interrupt handler and stored in RAM.

## Generalizing the Driver

After UART4 worked, I started making the driver usable for other UART/USART peripherals too.

The current initialization flow is:

```c
UART_gpioInit(UART4, PIN('A', 0), PIN('A', 1));
UART_init(UART4, 115200, true, 5);
```

`UART_gpioInit` does the following:

1. Checks if the requested TX/RX pins are valid for the selected UART.
2. Enables the GPIO ports.
3. Sets the pins to alternate function mode.
4. Sets the correct AF number.

`UART_init` does the following:

1. Enables the UART/USART peripheral clock.
2. Clears `CR1`, `CR2`, and `CR3`.
3. Sets `BRR` from the baud rate.
4. Enables TX, RX, and UART.
5. If requested, enables the RXNE interrupt and NVIC.

Right now, the baud rate calculation assumes a 16 MHz peripheral clock:

```c
uint32_t pclk = 16000000;
uart->BRR = (pclk + (baudRate / 2U)) / baudRate;
```

This is good enough for the current setup, but if I change the clock tree later, I will need to update this so `pclk` comes from the actual APB bus clock.

## UART Pin Table

I do not want to keep opening the datasheet every time I choose UART pins, so I made this table from the STM32F407 alternate function mapping.

| Peripheral | TX Pins | RX Pins | AF Number | APB Bus | RCC Enable Bit |
|---|---|---|---:|---|---:|
| USART1 | PA9, PB6 | PA10, PB7 | AF7 | APB2 | APB2ENR bit 4 |
| USART2 | PA2, PD5 | PA3, PD6 | AF7 | APB1 | APB1ENR bit 17 |
| USART3 | PB10, PC10, PD8 | PB11, PC11, PD9 | AF7 | APB1 | APB1ENR bit 18 |
| UART4 | PA0, PC10 | PA1, PC11 | AF8 | APB1 | APB1ENR bit 19 |
| UART5 | PC12 | PD2 | AF8 | APB1 | APB1ENR bit 20 |
| USART6 | PC6, PG14 | PC7, PG9 | AF8 | APB2 | APB2ENR bit 5 |

## Current Status

The current loopback test sends `"hello"` through UART4 and waits for the same bytes to appear in the UART4 ring buffer:

```c
uint8_t bytes[] = "hello";
uint32_t size = sizeof(bytes) - 1;
uint8_t buf[sizeof(bytes) - 1];

UART_transmitBytes(UART4, bytes, size);

for (uint32_t i = 0; i < size;) {
    if (RING_pop(&UART4_ringBuffer, &buf[i])) {
        i++;
    }
}
```

If the received buffer matches the transmitted bytes, the green LED toggles. Otherwise, the red LED toggles.

That is enough for now: UART4 loopback works, interrupts are being used for RX, and the driver is in better shape for reuse later.

## Things to Improve Later

- Use the real APB clock instead of assuming 16 MHz.
- Add timeout versions of blocking reads.
- Decide how to handle ring buffer overflow.
- Maybe add interrupt-driven TX too.
- Test the generalized driver on USART1, USART2, USART3, UART5, and USART6.
- Clean up the difference between `UART_transmitBuf` and `UART_transmitBytes`.
