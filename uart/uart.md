# 02 UART 


## Single Byte

## Testing

## Multiple Bytes
Let's make interrupt based uart communication. As mentioned on the comment of the commit, the hardware can only hold 1 byte. Thus, I can not send multiple bytes and hope it is stored inside some FIFO register.

To fix this, let's head to Reference Manual 30.4 USART interrupts. There is RXNE (received Data Ready to be Read) once this event happens, we immidetly need to read the data. Since this is an interrupt requests, things will work nicely.

One way of storing data is thorugh "ring buffer". I believe other people explained it [better](https://ntietz.com/blog/whats-in-a-ring-buffer/).

