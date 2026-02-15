// Distributed under the MIT license.
// See LICENSE.txt for details.

#include "arch/Halt.h"
#include "kernel/Console/IO.h"
#include "kernel/Console/Shell.h"
#include "kernel/Main/Lifecycle.h"

#include <stddef.h>

static void run_default_foreground_client(void) {
  run_shell_loop();
}

static kernel_foreground_client_t g_foreground_client = NULL;

void kernel_set_foreground_client(kernel_foreground_client_t client) {
  g_foreground_client = client;
}

void kernel_init(void) {
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

void kernel_halt() {
  halt();
}
