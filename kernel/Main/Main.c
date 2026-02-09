// Distributed under the MIT license.
// See LICENSE.txt for details.

#include "arch/Halt.h"
#include "kernel/Console/IO.h"
#include "kernel/Console/Shell.h"

#include <stdbool.h>

#define COMMAND_LENGTH 255

void kmain(void) {
  char command[COMMAND_LENGTH];
  console_print("Welcome to CharcoalOS.\n");
  print_prompt();
  while (true) {
    console_read_line(command, COMMAND_LENGTH, true);
    dispatch_command(command);
    print_prompt();
  }
  halt();
}
