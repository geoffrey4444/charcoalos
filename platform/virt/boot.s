// Distributed under the MIT license.
// See LICENSE.txt for details.

	.global _start       // Sets entry point of the code
_start:
	ldr x0, =stack_top   // load register x0; =stack_top supplied somehow?
	mov sp, x0           // set stack pointer to address in x0
	bl kmain             // Branch to kmain, put return address in x30
	                     //   kmain is a c function that will compile to
	                     //   the symbol kmain that assembly calls
	b .                  // Branch to self, no return address (inf loop)
