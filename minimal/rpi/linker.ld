/* Distributed under the MIT license. */
/* See LICENSE.txt for details. */

ENTRY(_start)                    /* set the entry point of the program */
SECTIONS {
  . = 0x80000;                   /* Raspberry Pi convention: kernel starts at */
                                 /* this address */
  .text.boot : { KEEP(*(.text.boot .text.boot.*)) }  /* KEEP: do not garbage-collect this  */
				 
  .text : { *(.text .text.*) }   /* all code */
  .rodata : { *(.rodata .rodata.*) } /* read-only data, like string literals */
  .data : { *(.data .data.*) }   /* initialized, writable data */

  /* bss = zero initialized data */
  /* NOLOAD: this section exists in memory, must be zeroed at runtime */
  /* align segment to 8 so boot code can use 64-bit (8-byte) stores to clear this section at runtime */
  .bss (NOLOAD) : {
    . = ALIGN(8);
    __bss_start = .;
    *(.bss .bss.*)
    *(COMMON)
    . = ALIGN(8);
    __bss_end = .;
  }
                                 /* common symbols at link time, e.g. */
                                 /* global scope symbols, in COMMON */
				 /* i.e. COMMON is tentative definitions */
				 /* bss = zero-initialized globals/statics */
				 /* not zeroed unless my startup code does */
				 
				 /* round locations to multiple of 8 bytes */
  . = ALIGN(16);                 /* stack pointer should be 16-byte aligned */
                                 /* at call boundaries */
  __stack_bottom = .;  
  . += 0x4000; /* Set up 16kB stack memory */
  __stack_top = .;
}
