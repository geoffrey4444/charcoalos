// Distributed under the MIT license.
// See LICENSE.txt for details.

#include "arch/Info.h"

#include <stddef.h>

size_t current_exception_level(void) {
  size_t exception_level;
  asm volatile ("mrs %0, CurrentEL" : "=r"(exception_level));
  return exception_level;
}

size_t mpidr_el1(void) {
  size_t result;
  asm volatile ("mrs %0, MPIDR_EL1" : "=r"(result));
  return result;
}
size_t sctlr_el1(void) {
  size_t result;
  asm volatile ("mrs %0, SCTLR_EL1" : "=r"(result));
  return result;
}

const char* arch_name(void) {
  return "arm64";
}
