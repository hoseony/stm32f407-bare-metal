/* 
Links that I found useful: 
https://sourceware.org/binutils/docs/ld/Scripts.html
https://github.com/PaxInstruments/STM32CubeF4/blob/master/Projects/STM32F4-Discovery/Applications/FatFs/FatFs_USBDisk/SW4STM32/STM32F4-DISCO/STM32F407VGTx_FLASH.ld 
*/

ENTRY(RESET_HANDLER)

MEMORY {
    flash (rx) : ORIGIN = 0x08000000, LENGTH = 1024K /* flash memory region */
    sram (rxw) : ORIGIN = 0x20000000, LENGTH = 128K  /* This is sram1 + sram2 region, ends at 0x2001_FFFF */
}

SECTIONS {

}
