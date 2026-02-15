// Distributed under the MIT license.
// See LICENSE.txt for details.

#include "kernel/Console/IO.h"
#include "kernel/Main/Lifecycle.h"

void kernel_panic(const char *panic_message) {
  if (panic_message != NULL) {
    console_print(panic_message);
    console_print("\n\n");
  }
  kernel_halt();
}
