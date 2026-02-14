// Distributed under the MIT license.
// See LICENSE.txt for details.

	.global _start       // Sets entry point of the code
_start:
	ldr x5, =__stack_top // load register x5; =stack_top supplied somehow?
	mov sp, x5           // set stack pointer to address in x5

  // core check
	// mpidr_el1 is a system register = multiprocessor affinity register
	// says which cpu am I running on, which cluster, which socket
	// each core has its own affinity ID
	// mpidr_el1 is a 32-bit register. Contents:
	// [7:0] = Aff0 = core number
	// [15:8] = Aff1 = cluster
	// [23:16] = Aff2
	// [31:24] = Aff3
	// For 4-core virt = the core id (0, 1, 2, or 3)
   mrs x7, mpidr_el1   // mrs move from system register to general register
	 and x7, x7, #3      // zero all but the lowest 2 bits (enough to count up to 3)
	 cbz x7, 0f          // compare and branch to label 0 forward if zero
1: wfe                 // wait for event: don't spin and drain power to wait, wait until something happens
	 b  1b               // if event never comes, it sleeps forever, but on event, it just goes back to sleep after wake
	                     // "polite" way to park a core
0:	
	// Zero bss
	ldr x5, =__bss_start
	ldr x6, =__bss_end

	// 1: sets a label... jump to it as
	// 1b (move backwards to next 1: label) or 1f
	// (move forward)
2:	cmp x5, x6
	b.hs 3f              // HS is >= ... i.e. if x5 >= x6, done, jump to 3f

	str xzr, [x5], #8    // store 8 bytes of zero (64 bits), then x5 += 8
	b 2b                 // jump backwards to 2:
3:
  // Set up exception handlers, defined in arch/arm64/ExceptionVectors.s.
	// This is done by setting the address of the exception vector table to
	// the system register VBAR_EL1.
	ldr x5, =_exception_vectors_el1
	msr vbar_el1, x5     // msr = write to system register

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
