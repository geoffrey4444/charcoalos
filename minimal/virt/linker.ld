ENTRY(_start) /* set entry point to global symbol _start */
SECTIONS {
  . = 0x40100000;
  .startup . : { boot.o(.text) } /* . : means put code (.text section */
                                 /* of assembled boot.s before anything else */
				 /* in RAM. */
  .text : { *(.text .text.*) }   /* all code */
  .data : { *(.data) }           /* data that isn't code...might be strings? */
  .rodata : { *(.rodata .rodata.*) } /* string literal ("Hello...") in here */
  .bss : { *(.bss COMMON) }      /* common symbols at link time, e.g. */
                                 /* global scope symbols, in COMMON */
				 /* i.e. COMMON is tentative definitions */
				 /* bss = zero-initialized globals/statics */
				 /* not zeroed unless my startup code does */
				 /* round locations to multiple of 8 bytes */
  . = ALIGN(16);                 /* stack pointer should be 16-byte aligned */
                                 /* at call boundaries */
  . += 0x1000; /* Set up 4kB stack memory: move counter up 4kB */
  stack_top = .; /* define stack_top for boot code as 0x40101000*/
}
