# 03 TIMER

Ok, maybe now is the time to finally give some shine to our little blinky (meaning, we will be making interrupt based blinky to start with). That will be good enough starting point in the making of TIMER.

I will be using TIM2 for now (general purpose timer) instead of the advanced timers.

For General Purpose Timers(TIM2 - TIM5) on the board that I am using, we have (Reference Manual 8.2)
    - 16 bit (TIM3 and TIM4) or 32 bit (TIM2 and TIM5) up, down, up/down auto-reload counter
    - 16 bit programmable prescaler to divide the counter clock frequency (1 - 65536)
    - Input Capture, Output compare, PWM gen, One-pulse mode output 
    - Synchronization circuitt
    - Interrupt/DMA (direct memory access) generation
    - Supports Incremental encoder and hall-sensor circuitry
    - Trigger input for external clock or cycle-by-cycle current management

Talking about it in general, timer is a hardware counter that counts clock ticks and we can use them into useful events as listed above. The image follows,
```
clock -> Prescaler -> counter -> update event
```

There are three Time-base units:
- Auto-Reload Register: contains value that counter will count up to.
- Prescaler Register: divides counter clock frequency (by factor between 1 and 65536)
- Update Interrupt: interrupt signal when the counter reaches Auto-Reload Register's value. 


Prescaler updates when next event hits. This makes sense as if it updates immediately, that will cause some problem (different clock rate in the middle, not fun, not good clock). This is done through 'Prescaler buffer,' (Reference Manual Figure.135 and 136).
