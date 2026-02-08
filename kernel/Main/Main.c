// Distributed under the MIT license.
// See LICENSE.txt for details.

#include "arch/Halt.h"
#include "kernel/Console/IO.h"

#include <stdbool.h>

void kmain(void) {
  console_print("Welcome to CharcoalOS.\n");
  console_print("\n\n Now echoing input...\n\n");
  while (true) {
    const char c = platform_console_getc();
    platform_console_putc(c);
  }
  halt();
}
