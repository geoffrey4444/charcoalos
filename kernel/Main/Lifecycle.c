// Distributed under the MIT license.
// See LICENSE.txt for details.

#include "arch/Halt.h"
#include "kernel/Console/IO.h"
#include "kernel/Console/Shell.h"
#include "kernel/Main/Lifecycle.h"

void kernel_init(void) {
  console_print("Welcome to CharcoalOS.\n");
}

void kernel_run(void) {
  run_shell_loop();
}

void kernel_halt() {
  halt();
}
