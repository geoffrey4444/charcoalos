// Distributed under the MIT license.
// See LICENSE.txt for details.

#include "arch/Halt.h"
#include "arch/Interrupt.h"
#include "kernel/Console/IO.h"
#include "kernel/Console/Shell.h"
#include "kernel/Main/Lifecycle.h"
#include "kernel/Hardware/DeviceTree.h"
#include "kernel/Panic/Panic.h"

#include <stddef.h>

static void run_default_foreground_client(void) { run_shell_loop(); }

static kernel_foreground_client_t g_foreground_client = NULL;

void kernel_set_foreground_client(kernel_foreground_client_t client) {
  g_foreground_client = client;
}

void kernel_init(uintptr_t dtb) {
  // Initialize the timer
  print_timer_diagnostics();
  console_print("Initializing timer... ");
  initialize_timer();
  console_print("done.\n");

  console_print("Parsing device tree blob for hardware information...\n");
  static struct hardware_info hw_info = {0};
  parse_device_tree_blob(&hw_info, dtb);

  console_print("Welcome to CharcoalOS.\n");

  if (g_foreground_client == NULL) {
    kernel_set_foreground_client(run_default_foreground_client);
  }
}

void kernel_run(void) {
  if (g_foreground_client == NULL) {
    halt();
    return;
  }
  g_foreground_client();
}

void kernel_halt() { halt(); }
