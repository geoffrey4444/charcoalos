// Distributed under the MIT license.
// See LICENSE.txt for details.

#include "arch/Halt.h"
#include "kernel/Console/IO.h"

void kmain(void) {
  console_print("Welcome to CharcoalOS.\n");
  halt();
}
