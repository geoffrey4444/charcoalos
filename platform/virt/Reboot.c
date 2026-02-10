// Distributed under the MIT license.
// See LICENSE.txt for details.

void platform_reboot(void) {
  // NOTE: later, when I have multiple cores and interrupts, first
  // disable interrupts and shut down other cores.
  //
  // QEMU Virt reboots via PCSI_SYSTEM_RESET call
  // set x0 = 0x84000009
  // set x1-x3 = 0x0
  // smc 0
  // block of assembly: the assembly code, then 
  // : outputs
  // : inputs
  // : clobbers
  // Note: movz : zero register, then move in literal
  // movk: set part of a register, lsl = shift left this much
  __asm__ volatile(
    "movz x0, 0x0009\n"
    "movk x0, 0x8400, lsl #16\n"
    "mov x1, xzr\n"
    "mov x2, xzr\n"
    "mov x3, xzr\n"
    "hvc #0\n"
    :
    :
    : "x0", "x1", "x2", "x3", "memory"
  );
  __builtin_unreachable(); // tell compiler this function should not return
}
