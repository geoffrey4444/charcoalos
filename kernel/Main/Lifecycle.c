// Distributed under the MIT license.
// See LICENSE.txt for details.

#include "arch/Halt.h"
#include "arch/Interrupt.h"
#include "kernel/Console/IO.h"
#include "kernel/Console/Shell.h"
#include "kernel/Main/Lifecycle.h"
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
  console_print("done.\n\n");

  // Read the dtb
  // It's a pointer, but MMU is not yet on, so it's a physical address
  // First, make sure it's not NULL. Then, dereference it to check for the
  // expected magic.
  if (dtb) {
    // Check for magic
    uint32_t magic = *(uint32_t*)(dtb);
    if (magic != 0xedfe0dd0) {
      kernel_panic(
          "The hardware could not be recognized because the device table blob "
          "is not in the expected format.");
    } else {
      // Device table is valid
      console_print("Device table blob recognized with magic ");
      console_print_hex((void *)&magic, 4);
      console_print("\n");
    }
  } else {
    // DTB pointer is NULL... for now, panic
    kernel_panic(
        "The hardware could not be recognized because the device table blob "
        "is null");
  }

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
