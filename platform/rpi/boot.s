// Distributed under the MIT license.
// See LICENSE.txt for details.

.section .text.boot
	.global _start       // Sets entry point of the code
_start:
	// core check
	// mpidr_el1 is a system register = multiprocessor affinity register
	// says which cpu am I running on, which cluster, which socket
	// each core has its own affinity ID
	// mpidr_el1 is a 32-bit register. Contents:
	// [7:0] = Aff0 = core number
	// [15:8] = Aff1 = cluster
	// [23:16] = Aff2
	// [31:24] = Aff3
	// For pi, Aff0 = the core id (0, 1, 2, or 3)
   mrs x7, mpidr_el1   // mrs move from system register to general register
	 and x7, x7, #3      // zero all but the lowest 2 bits (enough to count up to 3)
	 cbz x7, 0f          // compare and branch to label 0 forward if zero
1: wfe                 // wait for event: don't spin and drain power to wait, wait until something happens
	 b  1b               // if event never comes, it sleeps forever, but on event, it just goes back to sleep after wake
	                     // "polite" way to park a core
0:	
	// Zero bss safely even when __bss_start is not 8-byte aligned.
	ldr x5, =__bss_start  // x5 = current write pointer (start of .bss)
	ldr x6, =__bss_end    // x6 = end pointer (one past end of .bss)
2:	cmp x5, x6           // if current >= end, zeroing is done
	b.hs 6f               // branch to label 6 when no bytes remain
	tst x5, #7            // test low 3 address bits: aligned iff (x5 % 8) == 0
	b.eq 3f               // if already 8-byte aligned, switch to bulk loop
	strb wzr, [x5], #1    // store one zero byte, then advance pointer by 1
	b 2b                  // keep clearing head bytes until aligned or done
3:	sub x7, x6, x5       // x7 = remaining byte count
	cmp x7, #8            // check whether at least one 8-byte chunk remains
	b.lo 5f               // if fewer than 8 bytes remain, skip to tail-byte loop
4:	str xzr, [x5], #8    // store 8 zero bytes, then advance pointer by 8
	sub x7, x7, #8        // decrement remaining-byte counter
	cmp x7, #8            // check if another full 8-byte chunk remains
	b.hs 4b               // continue bulk 8-byte clearing while possible
5:	cmp x5, x6           // check whether any tail bytes still remain
	b.hs 6f               // if none remain, finish
	strb wzr, [x5], #1    // clear one remaining tail byte
	b 5b                  // continue tail-byte clearing to the end
6:
  // In EL2:
  // CPTR_EL2.TFP (bit 10) = 0  => don’t trap FP/SIMD to EL2
  // CPTR_EL2.TCPAC (bit 31) = 0 => don’t trap CPACR_EL1 accesses
  mrs x5, cptr_el2
	bic x5, x5, #(1 << 10)   // bic = bitwise bit clear, set bit 10 to 0 (don't trap)
	bic x5, x5, #(1 << 31)   // set bit 31 to 0 (don't trap CPACR_EL1 accesses)
	msr cptr_el2, x5     // write back to cptr_el2

	// Drop to EL1 (EL1h : use SP_EL1 as stack pointer)
	// Need to try this out at home, when I can test it
	// First, give el1 a stack
	ldr x5, =__stack_top
	msr sp_el1, x5

	// Make sure EL1 runs in aarch64 state, not aarch32 state
	// set system register HCR_EL2.RW (bit 31) = 1
	mrs x5, hcr_el2
	orr x5, x5, #(1 << 31)  // set bit 31 via bitwise or: preserve other flags
	msr hcr_el2, x5         // write back to hcr_el2 the updated result

	// Let EL1 access the hardware timer
	// CNTHCTL_EL2.EL1PCTEN (bit 0) = 1, EL1PCEN (bit 1) = 1 so cnthctl_el2
	// does not cause trap the timers to EL2, preventing EL1 from accessing them
	mrs x5, cnthctl_el2
	orr x5, x5, #3          // set bits 0 and 1 to 1 (3 = 0b11)
	msr cnthctl_el2, x5     // write back to cnthctl_el2

	// Set the offset of the timer to 0
	msr cntvoff_el2, xzr

	// Set EL1 entrypoint for ERET to branch to
	// Note: CurrentEL, elr_el2 are special purpose, not system, registers
	adr x5, start_el1
	msr elr_el2, x5 // elr_el2 = exception link register: where to return to
	                // after dropping to EL1 with eret

	// Set up saved PSTATE for EL1, which eret will load into PSTATE
  // M[3:0] = 0b0101 = #5 => EL1h = EL1, use SP_EL1 as stack pointer
	// I set SP_EL1 above, so that's what we want to use

  // Mask DAIF (disable Debug, SError, IRQ, FIQ) initially: bits [9:6] = 1111
  // SPSR = saved program status register: what PSTATE to load on eret call
	// SPSR_EL2 = SPSR for EL2
	mov x5, #0x3C5        // 0b1111000101 = 0x3C5	
	msr spsr_el2, x5      // write back to spsr_el2

	isb 									// make sure all the register writes above are visible before we drop to EL1
	eret                  // exception return: drop to EL1

start_el1:
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
	// b .               // Branch to self, no return address (inf loop)
	// Nope! Politely park the core to minimize power draw
7:
	bl halt
