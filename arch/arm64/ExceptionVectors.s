// Distributed under the MIT license.
// See LICENSE.txt for details.

// This file handles responding to exceptions on ARM64 architectures.
// The exceptions vector table is 2048 bytes long and must be 2048-byte aligned.
// 2048 = 2^11.

.section .text.vectors, "ax" // ax = allocate in memory, executable
.align 11                    // align to 2^11 = 2048 bytes

// Define a global symbol for the exception vector table. This symbol is 
// an address that identifies the beginning of code to handle the 
// exceptions table. Boot code writes this address to system reg VBAR_EL1.
.global _exception_vectors_el1
_exception_vectors_el1:

// The exception vector table has 4 groups of 4 128-byte entries each, for a
// total of 16 entries and 2048 bytes. Each entry is code to handle a particular
// exception type. You must tell the CPU where the table is, by writing the
// address of _exception_vectors_el1 to the system register VBAR_EL1.
// Then, depending on the exception type, the CPU will jump to an instruction
// at address _exception_vectors_el1 + (entry number * 128). Here are the 
// 16 entries, in order:
// 0: Synchronous error at current EL (EL1), using SP_EL0 as stack pointer
// 1: IRQ (Interrupt ReQuest) at EL1, using SP_EL0 as stack pointer
// 2: FIQ (Fast Interrupt reQuest) at EL1, using SP_EL0 as stack pointer
// 3: SError (System Error, can be asynchronous) at EL1, using SP_EL0
// 4: Synchronous error at EL1, using SPx (SP_EL1) as stack pointer
// 5: IQR at EL1, using SP_EL1 as stack pointer
// 6: FIQ at EL1, using SP_EL1 as stack pointer
// 7: SError at EL1, using SP_EL1 as stack pointer
// 8: Synchronous error at lower EL (EL0), aarch64 state
// 9: IRQ at EL0, aarch64 state
// 10: FIQ at EL0, aarch64 state
// 11: SError at EL0, aarch64 state
// 12: Synchronous error at EL0, aarch32 state
// 13: IRQ at EL0, aarch32 state
// 14: FIQ at EL0, aarch32 state
// 15: SError at EL0, aarch32 state
// The idea is each entry in the table can have up to 128 bytes of instructions
// to handle that exception. But our handler will just be 1 branch instruction
// for each type. We'll use .balign to pad out the rest of the 128 bytes, so
// each entry is where it should be.

// Let's build out this table. First, we'll put in 
// Use b (branch) instructions to set up handlers at the appropriate offsets.
// We want e.g.
//   b sync_el1_sp0 // at offset 0, branch to the handler
//   .balign 128 // make sure the instruction after this 1 is at the next
//               // 128-byte boundary
//   b irq_el1_sp0
//   .balign 128
//   ...
// But instead of writing out branch instructions to 16 handlers, we can
// use a macro to generate these 32 instructions for us. The macro takes
// a label, the handler name, and then generates the branch and balign.
.macro EXCEPTION_ENTRY label
  b \label
  .balign 128
.endm

// Now use the macro to generate the 16 entries in the table at the
// appropriate offsets.
EXCEPTION_ENTRY sync_el1_sp0
EXCEPTION_ENTRY irq_el1_sp0
EXCEPTION_ENTRY fiq_el1_sp0
EXCEPTION_ENTRY serr_el1_sp0
EXCEPTION_ENTRY sync_el1_spx
EXCEPTION_ENTRY irq_el1_spx
EXCEPTION_ENTRY fiq_el1_spx
EXCEPTION_ENTRY serr_el1_spx
EXCEPTION_ENTRY sync_el0_a64
EXCEPTION_ENTRY irq_el0_a64
EXCEPTION_ENTRY fiq_el0_a64
EXCEPTION_ENTRY serr_el0_a64
EXCEPTION_ENTRY sync_el0_a32
EXCEPTION_ENTRY irq_el0_a32
EXCEPTION_ENTRY fiq_el0_a32
EXCEPTION_ENTRY serr_el0_a32

// Now the table is complete. Finally, we need to write the code for each
// of those labels, i.e. the handlers. Then, when an exception occurs, the
// CPU will jump to the appropriate exception entry, and will then immediately
// branch to its handler.
// Our handlers will store some register data on the stack and then will
// call a C function to handle the exception (e.g., print error and panic or
// recover if possible).
// Other than tracking the exception type, the code to run will be the same
// at each entry. So use a macro.

.macro EXCEPTION_HANDLER kind_of_exception
  // Save registers and SP on the stack, making them available to C.
  // ARM64 has 31 general purpose registers (x0-x30) and a stack pointer (sp).
  // These are what we'll save for now. Each is 64 bits = 8 bytes, so
  // 32 registers * 8 bytes = 256 bytes. Make room for 256 bytes on the stack,
  // and then store each register at a known offset from the new sp.
  // stp = store pair of registers. We could make more space and store more
  // things later on, if needed (e.g. system / floating point registers).
  sub sp, sp, #256 // move stack pointer down 256 bytes, make room for regs
  stp x0, x1, [sp, #0] // store x0, x1 at sp +0, sp + 8
  stp x2, x3, [sp, #16] // store x2, x3 at sp +16, sp + 24
  stp x4, x5, [sp, #32]
  stp x6, x7, [sp, #48]
  stp x8, x9, [sp, #64]
  stp x10, x11, [sp, #80]
  stp x12, x13, [sp, #96]
  stp x14, x15, [sp, #112]
  stp x16, x17, [sp, #128]
  stp x18, x19, [sp, #144]
  stp x20, x21, [sp, #160]
  stp x22, x23, [sp, #176]
  stp x24, x25, [sp, #192]
  stp x26, x27, [sp, #208]
  stp x28, x29, [sp, #224]
  mov x9, sp // move current sp (after storing registers) to x9 to C and see it
  // note: x30 is the link register, return address for the exception handler
  stp x30, x9, [sp, #240] // store link register and stack pointer

  // Now call a C function to handle the exception, passing the current
  // stack pointer (which points to the saved registers) and the exception type
  // as parameters. Parameters go in x0, x1, ...
  mov x0, sp
  mov x1, #\kind_of_exception
  // bl = branch with link: jump (branch) to the function, with return address
  // in x30
  // In C, define a function
  // void handle_exception(uint64_t* saved_registers, uint64_t kind_of_exception)
  // to handle the exception. The saved registers will be in saved_registers[i].
  bl handle_exception

  // handle_exception returns one of three actions to take next:
  // 0. Resume
  // 1. Reboot
  // 2. Panic (infinite loop)
  // Function return values are in x0. Check and branch accordingly.
  // Is action == 0 (resume)?
  cmp x0, #0
  beq 0f

  // Is action == 1 (reboot)?
  cmp x0, #1
  beq 1f

  // Action must be 2 or something else, so panic
  b 2f

  0:
    // Resume: restore registers and return from exception
    ldp x0, x1, [sp, #0] // load sp + 0, sp + 8 to x0, x1
    ldp x2, x3, [sp, #16] // load sp + 16, sp + 24 to x2, x3
    ldp x4, x5, [sp, #32]
    ldp x6, x7, [sp, #48]
    ldp x8, x9, [sp, #64]
    ldp x10, x11, [sp, #80]
    ldp x12, x13, [sp, #96]
    ldp x14, x15, [sp, #112]
    ldp x16, x17, [sp, #128]
    ldp x18, x19, [sp, #144]
    ldp x20, x21, [sp, #160]
    ldp x22, x23, [sp, #176]
    ldp x24, x25, [sp, #192]
    ldp x26, x27, [sp, #208]
    ldp x28, x29, [sp, #224]
    ldr x30, [sp, #240]
    add sp, sp, #256 // reset stack pointer to value before this handler
    eret // return from exception; x30 is return address, sp is stack pointer
  1:
    // Reboot: trigger a system reset
    // To trigger the reboot, call existing C function platform_reboot()
    bl platform_reboot
    // Should not return from reboot, but if it does, just continue to panic
  2:
    // Panic: infinite loop
    bl halt
.endm

// Now define all the handlers (labels that get jumped to from the
// exception vector table) using the macro. For now, each exception is handled
// the same way, but that could be changed later if needed. Though by passing
// the kind of exception, the C handler can already handle exceptions
// differently as needed.
sync_el1_sp0:
  EXCEPTION_HANDLER 0
irq_el1_sp0:
  EXCEPTION_HANDLER 1
fiq_el1_sp0:
  EXCEPTION_HANDLER 2
serr_el1_sp0:
  EXCEPTION_HANDLER 3
sync_el1_spx:
  EXCEPTION_HANDLER 4
irq_el1_spx:
  EXCEPTION_HANDLER 5
fiq_el1_spx:
  EXCEPTION_HANDLER 6
serr_el1_spx:
  EXCEPTION_HANDLER 7
sync_el0_a64:
  EXCEPTION_HANDLER 8
irq_el0_a64:
  EXCEPTION_HANDLER 9
fiq_el0_a64:
  EXCEPTION_HANDLER 10
serr_el0_a64:
  EXCEPTION_HANDLER 11
sync_el0_a32:
  EXCEPTION_HANDLER 12
irq_el0_a32:
  EXCEPTION_HANDLER 13
fiq_el0_a32:
  EXCEPTION_HANDLER 14
serr_el0_a32:
  EXCEPTION_HANDLER 15
// Should not get here, but if you do, just infinite loop
4:
  bl halt

