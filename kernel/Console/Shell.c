// Distributed under the MIT license.
// See LICENSE.txt for details.

#include "arch/Info.h"
#include "arch/Halt.h"
#include "kernel/Console/IO.h"
#include "kernel/Console/Shell.h"
#include "kernel/String/String.h"
#include "platform/IO.h"
#include "platform/Info.h"
#include "platform/Reboot.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

size_t tokenize_command(char *command, char *tokens[], size_t max_tokens) {
  if (max_tokens == 0) {
    tokens = NULL;
    return 0;
  }

  size_t command_length = string_length(command);
  if (command_length == 0) {
    tokens = NULL;
    return 0;
  }

  bool between_tokens = false;
  size_t i = 0;
  size_t number_of_tokens = 1;
  tokens[0] = command;

  while (i < command_length) {
    if (command[i] == '\0') {
      return number_of_tokens;
    } else if ((command[i] == '\t') || (command[i] == ' ') ||
               (command[i] == '\r') || (command[i] == '\n')) {
      between_tokens = true;
      command[i] = '\0';
    } else if (between_tokens) {
      tokens[number_of_tokens] = command + i;
      ++number_of_tokens;
      between_tokens = false;
    }
    ++i;
  }
  return number_of_tokens;
}

void dispatch_command(char *command) {
  const size_t max_tokens = MAX_SHELL_ARGS;  // max number of arguments
  char *tokens[MAX_SHELL_ARGS];

  const size_t argc = tokenize_command(command, tokens, max_tokens);
  if (argc == 0) {
    // empty command; just return
    return;
  }

  const size_t number_of_commands = sizeof(commands) / sizeof(commands[0]);
  int result = 0;

  // Loop over available commands, and dispatch if command found
  const char *const *cargv = (const char *const *)tokens;

  for (size_t i = 0; i < number_of_commands; ++i) {
    if (string_compare(cargv[0], commands[i].name)) {
      result = commands[i].handler(argc, cargv);
      if (result) {
        console_print("Error: an error occurred in the command ");
        console_print(cargv[0]);
        console_print("\n");
      }
      return;
    }
  }

  // No command found
  console_print("Error: the command ");
  console_print(cargv[0]);
  console_print(
      " could not be recognized. Type help for a list of commands.\n");
}

void print_prompt(void) { console_print("> "); }

// Handlers for built-in commands

int help_handler(size_t argc, const char *const *argv) {
  // Arguments are unused
  (void)argc;
  (void)argv;
  console_print("CharcoalOS available commands:\n\n");
  const size_t number_of_commands = sizeof(commands) / sizeof(commands[0]);
  for (size_t i = 0; i < number_of_commands; ++i) {
    console_print(commands[i].name);
    console_print("\t\t");
    console_print(commands[i].help_text);
    console_print("\n");
  }
  console_print("\n");
  return 0;
}

int info_handler(size_t argc, const char *const *argv) {
  (void)argc;
  (void)argv;

  char data[16];

  console_print("CharcoalOS built and linked on ");
  console_print(__DATE__);
  console_print(" ");
  console_print(__TIME__);
  console_print("\n");

  console_print("Compiler: ");
#if defined(__clang__)
  console_print("clang\n");
#elif defined(__GNUC__)
  console_print("gcc\n");
#else
  console_print("Unknown");
#endif

  console_print("Pointer size (bytes): 0x");
  const size_t pointer_size = sizeof(void *);
  data[0] = pointer_size;
  console_print_hex(data, 1);
  console_print("\n");

  console_print("Hosted C environment: ");
  if (__STDC_HOSTED__) {
    console_print("yes\n");
  } else {
    console_print("no\n");
  }

  console_print("MMIO base address: 0x");
  uintptr_t mmio_address = mmio_base_address();
  console_print_hex((const void *)&mmio_address, pointer_size);
  console_print("\n");
  console_print("UART base address: 0x");
  uintptr_t uart_address = uart_base_address();
  console_print_hex((const void *)&uart_address, pointer_size);
  console_print("\n");

  console_print("Current exception level: 0x");
  data[0] = current_exception_level();
  console_print_hex(data, 1);
  console_print("\n");

  console_print("MPIDR_EL1: 0x");
  size_t mpidr = mpidr_el1();
  console_print_hex((const void *)&mpidr, pointer_size);
  console_print("\n");

  console_print("SCTLR_EL1: 0x");
  size_t sctlr = sctlr_el1();
  console_print_hex((const void *)&sctlr, pointer_size);
  console_print("\n");

  console_print("Architecture: ");
  console_print(arch_name());
  console_print("\n");

  console_print("Platform: ");
  console_print(platform_name());
  console_print("\n");

  return 0;
}

int memread_handler(size_t argc, const char *const *argv) {
  (void)argc;
  (void)argv;
  // for (size_t i = 1; i < argc; ++i) {
  //   uintptr_t address = (uintptr_t)argv[i];
  //   console_print_hex((const void*)&address, 8);
  //   console_print("    ");
  //   size_t contents = (size_t)*(argv[i]);
  //   console_print_hex((const void*)&contents, 8);
  // }
  console_print("Command not yet implemented\n");
  return 1;
}

int panic_handler(size_t argc, const char *const *argv) {
  console_print("Sorry, a system error has occurred\n\n");
  const char *panic_message = NULL;
  if ((argc > 1) && (argv != NULL)) {
    panic_message = argv[1];
  } else if ((argc == 1) && (argv != NULL)) {
    panic_message = argv[0];
  }

  if (panic_message != NULL) {
    console_print(panic_message);
    console_print("\n\n");
  }
  halt();
  return 0;
}

int reboot_handler(size_t argc, const char *const *argv) {
  (void)argc;
  (void)argv;
  platform_reboot();
  return 0;
}

int registers_handler(size_t argc, const char *const *argv) {
  (void)argc;
  (void)argv;
  console_print("Command not yet implemented\n");
  return 1;
}
