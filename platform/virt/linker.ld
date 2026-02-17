/* Distributed under the MIT license. */
/* See LICENSE.txt for details. */

ENTRY(_start) /* set entry point to global symbol _start */
PHDRS {
  text PT_LOAD FLAGS(5); /* PF_R | PF_X */
  data PT_LOAD FLAGS(6); /* PF_R | PF_W */
}

SECTIONS {
  . = 0x40080000;
  .startup . : { boot.o(.text) } /* . : means put code (.text section */
                                 /* of assembled boot.s before anything else */
                                 /* in RAM. */
                                 :text
  .text : { *(.text .text.*) } :text /* all code */
  .rodata : { *(.rodata .rodata.*) } :text /* read-only data */
  .data : { *(.data .data.*) } :data /* initialized writable data */

  __bss_start = .;
  .bss (NOLOAD) : {
    . = ALIGN(8);
    *(.bss .bss.*)
    *(COMMON)
    . = ALIGN(8);
  } :data /* zero-initialized globals/statics */
  __bss_end = .;

  .stack (NOLOAD) : {
    . = ALIGN(16); /* stack pointer should be 16-byte aligned */
    __stack_bottom = .;
    . += 0x4000; /* set up 16 KiB stack memory */
    __stack_top = .;
  } :data
}
