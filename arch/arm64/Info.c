// Distributed under the MIT license.
// See LICENSE.txt for details.

#include "arch/Info.h"

#include <stddef.h>

size_t current_exception_level(void) {
#if defined(__aarch64__) && !__STDC_HOSTED__
  size_t exception_level;
  __asm__ volatile ("mrs %0, CurrentEL" : "=r"(exception_level));
  return exception_level;
#else
  return 0;
#endif
}

size_t mpidr_el1(void) {
#if defined(__aarch64__) && !__STDC_HOSTED__
  size_t result;
  __asm__ volatile ("mrs %0, MPIDR_EL1" : "=r"(result));
  return result;
#else
  return 0;
#endif
}

size_t sctlr_el1(void) {
#if defined(__aarch64__) && !__STDC_HOSTED__
  size_t result;
  __asm__ volatile ("mrs %0, SCTLR_EL1" : "=r"(result));
  return result;
#else
  return 0;
#endif
}

const char* arch_name(void) {
  return "arm64";
}
