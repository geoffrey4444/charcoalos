// Distributed under the MIT license.
// See LICENSE.txt for details.

	.global _start       // Sets entry point of the code
_start:
	ldr x5, =stack_top   // load register x5; =stack_top supplied somehow?
	mov sp, x5           // set stack pointer to address in x5

	// Enable floating point
	// Why enable early? I know numerical algorithms, and I might want to
	// try out some floating point in the kernel
	mrs x5, cpacr_el1      // cpacr_el1[21:20] = FPEN = 0b11 to enable FP/SIMD
	orr x5, x5, #(3 << 20) // set bits 20 and 21 to 1 (3 = 0b11)
	msr cpacr_el1, x5      // write back to cpacr_el1
	isb                    // instruction synchronization barrier:
	                       // make sure future instructions see the effect of
												 // enabling floating point

	bl kmain             // Branch to kmain, put return address in x30
	                     //   kmain is a c function that will compile to
	                     //   the symbol kmain that assembly calls
	// b .                  // Branch to self, no return address (inf loop)
	// Nope! Politely park the core to minimize power draw
halt:
	wfe
	b halt
