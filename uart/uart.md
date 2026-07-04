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
I haven't used it yet but maybe having UASRT to serial would be nice. Though, it is still not hard to test with openOCD (Open On-Chip Debugger). I always setup alias on my terminal when I am working with openOCD as the command gets out of my brain capacity.
```
alias openocd-stm32f407d='openocd -f interface/stlink.cfg -f target/stm32f4x.cfg'
```

The default usage of gdb with openOCD requires using two terminal (thank you tmux!). You need to type the above command on the one terminal. Then, open another terminal and type these commands:
```
arm-none-eabi-gdb firmware.elf  | This will start a gdb session
target remote :3333             | when you first connect to the board with openOCD, it will tell you the port. 
                                | For my case it is 3333
monitor reset halt
load
break main
break 324                       | for the current version of main code I am writing, this is equivalent
                                | to the line after "UART_transmitBytes(UART4, bytes, size);"
continue
```

In gdb, you can also define a function. It makes the life easier. Let me show you an example:
```
(gdb) define rb
End with a line saying just "end".
>p UART4_ringBuffer.head
>p UART4_ringBuffer.tail
>x/16cb UART4_ringBuffer.data
>end
(gdb) rb
$11 = 36
$12 = 40
0x20000004 <UART4_ringBuffer>:	111 'o'	104 'h'	101 'e'	108 'l'	108 'l'	111 'o'	104 'h'	101 'e'
0x2000000c <UART4_ringBuffer+8>:	108 'l'	108 'l'	111 'o'	104 'h'	101 'e'	108 'l'	108 'l'	111 'o'
```

## Multiple Bytes
That's good and all... But it's not that useful.

Let's make interrupt based uart communication. As mentioned on the comment of the commit, the hardware can only hold 1 byte. Thus, I can not send multiple bytes and hope it is stored inside some FIFO register.

To fix this, let's head to Reference Manual 30.4 USART interrupts. There is RXNE (received Data Ready to be Read) once this event happens, we immidetly need to read the data. Since this is an interrupt requests, things will work nicely.

One way of storing data is thorugh "ring buffer". I believe other people explained it [better](https://ntietz.com/blog/whats-in-a-ring-buffer/).

## Then?
Then, I started generalizing my implementation for UART4 to other UARTs to make it an actual usable driver. Quite a bit of work, but I wanted to make some working driver that I can use later.
