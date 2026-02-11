// Distributed under the MIT license.
// See LICENSE.txt for details.

#include "arch/Info.h"

#include <stddef.h>

size_t current_exception_level(void) {
#if defined(__aarch64__) && !__STDC_HOSTED__
  size_t exception_level;
  __asm__ volatile("mrs %0, CurrentEL" : "=r"(exception_level));
  // CurrentEL holds exception level in bits 2 and 3, so shift right 2.
  return exception_level >> 2;
#else
  return 0;
#endif
}

size_t mpidr_el1(void) {
#if defined(__aarch64__) && !__STDC_HOSTED__
  size_t result;
  __asm__ volatile("mrs %0, MPIDR_EL1" : "=r"(result));
  return result;
#else
  return 0;
#endif
}

size_t sctlr_el1(void) {
#if defined(__aarch64__) && !__STDC_HOSTED__
  size_t result;
  __asm__ volatile("mrs %0, SCTLR_EL1" : "=r"(result));
  return result;
#else
  return 0;
#endif
}

const char* arch_name(void) { return "arm64"; }

uintptr_t stack_pointer_address(void) {
#if defined(__aarch64__) && !__STDC_HOSTED__
  uintptr_t result;
  __asm__ volatile("mov %0, sp" : "=r"(result));
  return result;
#else
  return 0;
#endif
}
