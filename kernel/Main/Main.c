// Distributed under the MIT license.
// See LICENSE.txt for details.

#include "arch/Halt.h"
#include "kernel/Console/IO.h"

#include <stdbool.h>

#define COMMAND_LENGTH 255

void kmain(void) {
  char command[COMMAND_LENGTH];
  console_print("Welcome to CharcoalOS.\n> ");
  while (true) {
    console_read_line(command, COMMAND_LENGTH, true);
    // Later, replace this with code to parse and handle the command.
    console_print("Command received: ");
    console_print(command);
    console_print("\n> ");
  }
  halt();
}
