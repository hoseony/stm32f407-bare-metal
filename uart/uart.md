# 02 UART

After getting blinky working, I wanted to make the board talk to the outside world. UART felt like the natural next step: it is simple enough to understand at the register level, but still useful enough that I can keep reusing it in future projects.

For this part, I used UART4 on the STM32F407 Discovery board:

- TX: PA0, alternate function AF8
- RX: PA1, alternate function AF8
- Baud rate: 115200

For testing, I connected TX and RX together as a loopback. If the byte I send comes back correctly, the green LED toggles. If something does not match, the red LED toggles.

## Useful References

- STM32F407 Reference Manual: Section 7 RCC, Section 8 GPIO, Section 10 Interrupts and events, Section 30 USART
- STM32F407 Datasheet: Table 9 Alternate function mapping

## Starting With One Byte

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
3. Select AF8 for PA0/PA1.
4. Enable the UART4 peripheral clock from RCC.
5. Set the baud rate register.
6. Enable transmitter, receiver, and UART itself.

For UART4, the important registers are:

- `RCC->APB1ENR`: enables the UART4 peripheral clock.
- `USARTx->BRR`: sets the baud rate.
- `USARTx->CR1`: enables TX, RX, UART, and optionally RX interrupt.
- `USARTx->SR`: contains status flags like `RXNE` and `TXE`.
- `USARTx->DR`: the data register used for both transmit and receive.

My first blocking functions looked like this conceptually:

```c
void UART_transmitByte(USART_t *uart, uint8_t data) {
    uart->DR = data;

    while ((uart->SR & BIT(7)) == 0) {
        ;
    }
}

uint8_t UART_readByte(USART_t *uart) {
    while ((uart->SR & BIT(5)) == 0) {
        ;
    }

    return (uint8_t)(uart->DR & 0xFF);
}
```

`BIT(7)` is `TXE`, which tells me the transmit data register is empty. `BIT(5)` is `RXNE`, which tells me received data is ready to read.

This worked for sending one byte, and it was a good first sanity check that the GPIO alternate function setup, UART clock, baud rate, and loopback wiring were all correct.

## The Multiple Byte Problem

One byte worked, so naturally I tried to send multiple bytes.

This cause some problems as UART data register is not a string buffer. I cannot write several bytes and expect the hardware to remember all of them for me.

In fact, on the commit `b28db24`, I left myself the note:
> transmit/reading bytes does not work because of the fifo register size. will need to implement interrupt based

That was the point where the project moved from "write a byte, read a byte" to "I need a small software buffer with interrupt."

The blocking transmit side is still simple because I can wait for `TXE` before writing each byte:

```c
void UART_transmitBytes(USART_t *uart, uint8_t *data, uint32_t size) {
    for (uint32_t i = 0; i < size; i++) {
        uart->DR = data[i];

        while ((uart->SR & BIT(7)) == 0) {
            ;
        }
    }
}
```

Receiving is different. If bytes arrive and I do not read them quickly enough, I can lose data. This is where interrupts make more sense.

## RXNE Interrupt

The useful receive interrupt is `RXNE`: receive data register not empty.

The idea:

1. A byte arrives.
2. The UART sets `RXNE`.
3. The interrupt handler runs.
4. The handler reads `DR`.
5. The byte gets stored into a software buffer.
6. The main loop can read from that buffer later.

For UART4, the handler looks like this:

```c
void UART4_IRQHandler(void) {
    if (UART4->SR & BIT(5)) {
        uint8_t byte = (uint8_t)(UART4->DR);
        RING_push(&UART4_ringBuffer, byte);
    }
}
```

One detail that helped me understand this better: reading `DR` is not just "getting the byte." It is also part of clearing the receive condition, because the hardware now knows I consumed the received data.

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

This made the receive path much cleaner. The interrupt handler only does the small urgent thing: read the byte and push it into the buffer. The main program can decide what to do with the bytes later.

## NVIC Setup

Adding the UART interrupt also meant I needed to deal with NVIC.

For UART4, the IRQ number is 52:

```c
#define IRQn_UART4 52
```

The driver enables the interrupt through helper functions:

```c
NVIC_setPriority(irqn, priority);
NVIC_enableIRQ(irqn);
```

The vector table also needs to point to the correct handler. That is why later commits added IRQ handlers such as:

- `USART1_IRQHandler`
- `USART2_IRQHandler`
- `USART3_IRQHandler`
- `UART4_IRQHandler`
- `UART5_IRQHandler`
- `USART6_IRQHandler`

At first I only needed `UART4_IRQHandler`, but once the driver became more general, the rest of the handlers followed the same pattern.

## Testing With OpenOCD And GDB

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

## Generalizing The Driver

After UART4 worked, I started making the driver usable for other UART/USART peripherals too.

The current initialization flow is:

```c
UART_gpioInit(UART4, PIN('A', 0), PIN('A', 1));
UART_init(UART4, 115200, true, 5);
```

`UART_gpioInit` does:

1. Check if the requested TX/RX pins are valid for the selected UART.
2. Enable the GPIO ports.
3. Set the pins to alternate function mode.
4. Set the correct AF number.

`UART_init` does:

1. Enable the UART/USART peripheral clock.
2. Clear `CR1`, `CR2`, and `CR3`.
3. Set `BRR` from the baud rate.
4. Enable TX, RX, and UART.
5. If requested, enable RXNE interrupt and NVIC.

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

One small note: the register header currently has `UART7` and `UART8` addresses commented around in the driver history, but this board/MCU setup is focused on USART1, USART2, USART3, UART4, UART5, and USART6.

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

That is enough for now: UART4 loopback works, interrupts are being used for RX, and the driver is in a better shape for reuse later.

## Things To Improve Later

- Use the real APB clock instead of assuming 16 MHz.
- Add timeout versions of blocking reads.
- Decide how to handle ring buffer overflow.
- Maybe add interrupt-driven TX too.
- Test the generalized driver on USART1, USART2, USART3, UART5, and USART6.
- Clean up the difference between `UART_transmitBuf` and `UART_transmitBytes`.
