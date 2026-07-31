# 01 Blinky
To start off the project, I decided to make bare-metal blinky using built-in LED.
I will be putting some notes as I go through my journey during this summer (and hopefully I can add it to my website).

Update: 
- The board that I am using is STM32F407 Discovery board. 
- Since I wrote next few part of this series, I thought this is a good time to add some guide on reading/following this if anyone is going to. First, follow "Bare-Metal Programming Guide" for Blinky (sorry for not writing anything substantial here) and start working with uart following the guide (It lead some confusion to me since the board the guide is using was different). From uart part, I did took time writing it nicely, so perhaps, it will be good to have along side you. Do not hasitate contacting me. As long as it is something that I can help, I will.

### General Resources
- [STM32World bare-metal development](https://stm32world.com/wiki/STM32_Bare_Metal_Development)
- [Bare-Metal Programming Guide](https://github.com/cpq/bare-metal-programming-guide)

### src/stm32f407.ld
- [sourceware.org: Linker Script](https://sourceware.org/binutils/docs/ld/Scripts.html)
- [osdev.org: Linker Script](https://wiki.osdev.org/Linker_Scripts)
- [STM32CUBEF4 (github)](https://github.com/PaxInstruments/STM32CubeF4/blob/master/Projects/STM32F4-Discovery/Applications/FatFs/FatFs_USBDisk/SW4STM32/STM32F4-DISCO/STM32F407VGTx_FLASH.ld)
### Bootstrapping
When you start a computer, the process requires some sequences of stages to happen to run a program. You can think bootstrapping as something that happens between starting up a computer and running `main`.

#### \_\_attribute\_\_
`__attribute__((__aligned__(x)))`: Aligns memory address to multiple of x 

`__attribute__((weak))`: Allows you to override the symbol

`__attribute__((section("section_name"))):`: Forces stuff into a specific section in the object file

There are more stuff you can do, but I only put here things that might useful to me
 - naked: omit standard function prologue and epilogue sequence (when generating asesembly)
 - noreturn: this function never returns

### RCC (Reset and Clock Control)
The programmer is required to specify which peripherals one wants to be using. This is done through RCC. It also saves some power too.
