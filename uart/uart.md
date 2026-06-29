# 02 UART 


## Single Byte
The general structure would be as such:
```
/* e.g. testing transmission/reception of a byte */

if (timer_expired(&timer_green, period, s_ticks)) {
    static bool LED_green_bool = false;
    static bool LED_red_bool = false;

    GPIO_BSRR_writeBit(LED_green, LED_green_bool);
    GPIO_BSRR_writeBit(LED_red, LED_red_bool);

    UART_transmitByte(UART4, 'A');
    uint8_t received = UART_readByte(UART4);

    if (received == 'A') {
        LED_green_bool = !LED_green_bool;
    } else {
        LED_red_bool = !LED_red_bool;
    }
}
```
I will transmit a byte and receive a byte (loop-back) to confirm if the data transmitted/received are the same.

## Testing
I haven't used it yet but maybe having UASRT to serial would be nice...

## Multiple Bytes
That's good and all... But it's not that useful.

Let's make interrupt based uart communication. As mentioned on the comment of the commit, the hardware can only hold 1 byte. Thus, I can not send multiple bytes and hope it is stored inside some FIFO register.

To fix this, let's head to Reference Manual 30.4 USART interrupts. There is RXNE (received Data Ready to be Read) once this event happens, we immidetly need to read the data. Since this is an interrupt requests, things will work nicely.

One way of storing data is thorugh "ring buffer". I believe other people explained it [better](https://ntietz.com/blog/whats-in-a-ring-buffer/).

## Then?
Then, I started generalizing my implementation for UART4 to other UARTs to make it an actual usable driver. Quite a bit of work, but I wanted to make some working driver that I can use later.
