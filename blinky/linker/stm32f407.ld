/* Entry Point */
ENTRY(RESET_HANDLER)

MEMORY {
    FLASH (rx) : ORIGIN = 0x08000000, LENGTH = 1024K /* flash memory region */
    SRAM (rxw) : ORIGIN = 0x20000000, LENGTH = 128K  /* This is sram1 + sram2 region, ends at 0x2001_FFFF */
}

_estack = ORIGIN(SRAM) + LENGTH(SRAM); /* initial stack pointer: end of RAM (initialize descending call stack)*/

SECTIONS {
  /* | vectors : This is an array of addresses at the start of flash
   * | It tells cpu what to do when smt happens 
   * | e.g. where to go when certain interrupt */
    .vectors : { KEEP(*(.vectors)) } > FLASH
  /* | The code itself is doing the following
   * | Keep - telling compiler not to delete it 
   * | * - get all the .o files
   * | (.vectors) grab the section named .vectors */

  /* | .text: Where the instructions go */
    .text    : { *(.text*) }         > FLASH
  /* | after .text is just wildcard */

  /* | .rodata: Where constants are being stored */
    .rodata  : { *(.rodata*)}        > FLASH
}
