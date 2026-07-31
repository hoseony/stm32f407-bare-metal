# 03 TIMER

Ok, maybe now is the time to finally give some shine to our little blinky (meaning, we will be making interrupt based blinky to start with). That will be good enough starting point in the making of TIMER.

I will be using TIM2 for now (general purpose timer) instead of the advanced timers.

## Some Infos..
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

the initialization is also quite straight forward.
 - enable clock in RCC 
 - set PSC 
 - set ARR 
 - enable update interrupt
 - enable IRQ
 - enable timer

### About Period..
These are few thngs that I didn't realize when I first doing it. So might as well share it.

timer_clock = 16,000,000 ticks/second
PSC = 15999
Timer divids by PSC + 1 = 16000

Why + 1? I mean well... the Reference Manual says so, and my guess is that to avoid division by 0.

Also, period = (PSC + 1) * (ARR + 1) / timer_clock. That's because period = #counts / counter_clock. Notice how 
1. #counts = ARR + 1, (it counts from 0)
2. counter_clock = timer_clock / (PSC + 1)

Now things makes sense...

## PWM
When you take a look a PWM, it kind of tells you what you need to do to generate PWM signal (Reference Manual 18.3.9).



### More about Timer
There are three different clock sources that can be used on this board: HSI oscillator, HSE oscillator, and Main PLL clock.
 - HSI clock is generated from 16 MHz RC oscillator, and can be used as a system clock


HSE: high speed external


(Reference Manual 7.2)
STM32 have 2 different clock source: HSE (High Speed Clock) and LSE (Low Speed Clock)


Also, I should probably stop (and you should also probably stop) using 16 mHz constant as timer_clock. Unfortunately, that is not always true. There's something called clock tree (for the mcu I'm using it's on 6.2 or Figure 16 of the Reference Manual).

When you take a look at the Figure, we can see it follows few different things:

SYSCLK->





