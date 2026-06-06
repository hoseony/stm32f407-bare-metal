/* Entry Point */
ENTRY(_reset) /* This is a first `vector` */

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

    .data : {
        _sdata = .; /* These just marks the start */
        *(.first_data)
        *(.data SORT(.data.*))
        _edata = .;
    } > SRAM AT > FLASH

    _sidata = LOADADDR(.data);

    .bss : {
        _sbss = .;
        *(.bss SORT(.bss.*) COMMON)
        _ebss = .;
    } > SRAM

    . = ALIGN(8); /* This rounds up to the next 8 byte boundary
                     (allocation expects to start on an alligned address) */
    _end = .;     /* good to go */

    /* 
     * ------------------------------
     * The picture goes like This
     * ------------------------------
     * _estack  | (stack grows down)
     *          | stack
     *          | heap
     *          | (malloc goes up)
     * _end     | ALGIN(8) padding
     * _ebss    |
     *          | .bss
     * _sbss    |
     * _edata   |
     *          | .data
     * _sdata   |
     * ------------------------------
     * Start of RAM
     * /
}
