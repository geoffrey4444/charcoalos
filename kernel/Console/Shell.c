// Distributed under the MIT license.
// See LICENSE.txt for details.

#include "arch/Info.h"
#include "arch/Interrupt.h"
#include "kernel/Console/IO.h"
#include "kernel/Console/Shell.h"
#include "kernel/Panic/Panic.h"
#include "kernel/Panic/Restart.h"
#include "kernel/String/String.h"
#include "kernel/Time/Uptime.h"
#include "platform/IO.h"
#include "platform/Info.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

static const struct ShellCommand commands[] = {
    {"add", "Add two unsigned hex numbers", add_handler},
    {"help", "Display this help message", help_handler},
    {"info", "Display information for debugging", info_handler},
    {"memread", "Read memory at address", memread_handler},
    {"panic", "Panic the kernel", panic_handler},
    {"reboot", "Reboot the system", reboot_handler},
    {"trapsvc", "Trigger a svc exception", trapsvc_handler},
    {"uptime", "Display milliseconds since last restart", uptime_handler}};

size_t shell_number_of_commands(void) {
  return sizeof(commands) / sizeof(commands[0]);
}

struct ShellCommand shell_command_at(size_t index) {
  if (index >= shell_number_of_commands()) {
    kernel_panic("Requested shell command that does not exist");
  }
  return commands[index];
}

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
      if (number_of_tokens >= max_tokens) {
        return number_of_tokens;
      }
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

  const size_t number_of_commands = shell_number_of_commands();
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

int add_handler(size_t argc, const char *const *argv) {
  if (argc != 3) {
    console_print("Usage: add 0x1234 0xdbca\n");
    return 1;
  }
  uint64_t a = console_uint64_from_hex(argv[1]);
  uint64_t b = console_uint64_from_hex(argv[2]);
  uint64_t result = a + b;
  console_print_hex_value((const void *)&result, 8);
  console_print("\n");
  return 0;
}

int help_handler(size_t argc, const char *const *argv) {
  // Arguments are unused
  (void)argc;
  (void)argv;
  console_print("CharcoalOS available commands:\n\n");
  const size_t number_of_commands = shell_number_of_commands();
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
  console_print_hex_value(data, 1);
  console_print("\n");

  console_print("Hosted C environment: ");
  if (__STDC_HOSTED__) {
    console_print("yes\n");
  } else {
    console_print("no\n");
  }

  console_print("MMIO base address: 0x");
  uintptr_t mmio_address = mmio_base_address();
  console_print_hex_value((const void *)&mmio_address, pointer_size);
  console_print("\n");
  console_print("UART base address: 0x");
  uintptr_t uart_address = uart_base_address();
  console_print_hex_value((const void *)&uart_address, pointer_size);
  console_print("\n");

  console_print("Stack pointer: 0x");
  uintptr_t stack_pointer = stack_pointer_address();
  console_print_hex_value((const void *)&stack_pointer, pointer_size);
  console_print("\n");

  console_print("Current exception level: 0x");
  data[0] = current_exception_level();
  console_print_hex_value(data, 1);
  console_print("\n");

  console_print("MPIDR_EL1: 0x");
  size_t mpidr = mpidr_el1();
  console_print_hex_value((const void *)&mpidr, pointer_size);
  console_print("\n");

  console_print("SCTLR_EL1: 0x");
  size_t sctlr = sctlr_el1();
  console_print_hex_value((const void *)&sctlr, pointer_size);
  console_print("\n");

  console_print("Architecture: ");
  console_print(arch_name());
  console_print("\n");

  console_print("Platform: ");
  console_print(platform_name());
  console_print("\n");

  console_print("System clock frequency (Hz): 0x");
  const uint64_t system_clock_freq = read_timer_frequency_in_hz();
  console_print_hex_value((void *)&system_clock_freq, 8);
  console_print("\n");

  console_print("Interrupt timer ticks per second: 0x");
  const uint64_t interrupt_freq = interrupt_frequency_in_hz();
  console_print_hex_value((void *)&interrupt_freq, 8);
  console_print("\n");

  console_print("3.125 (hex): 0x");
  const double x = 3.125;
  console_print_hex_value((void *)&x, 8);
  console_print("\n");

  console_print("sqrt(2): 0x");
  double y = 2.0;
  __asm__ volatile("fsqrt %d0, %d1" : "=w"(y) : "w"(y));
  // inline assembly to take hardware sqrt, since no math.h yet
  console_print_hex_value((void *)&y, 8);
  console_print("\n");

  return 0;
}

int memread_handler(size_t argc, const char *const *argv) {
  for (size_t i = 1; i < argc; ++i) {
    uintptr_t address = (uintptr_t)console_uint64_from_hex(argv[i]);
    console_print_hex_value((const void *)&address, 8);
    console_print("    ");
    size_t contents = *((size_t *)address);
    console_print_hex_value((const void *)&contents, 8);
    console_print("\n");
  }
  return 0;
}

int panic_handler(size_t argc, const char *const *argv) {
  console_print("Kernel panic\n\n");
  const char *panic_message = NULL;
  if ((argc > 1) && (argv != NULL)) {
    panic_message = argv[1];
  } else if ((argc == 1) && (argv != NULL)) {
    panic_message = argv[0];
  }
  kernel_panic(panic_message);

  return 0;
}

int reboot_handler(size_t argc, const char *const *argv) {
  (void)argc;
  (void)argv;
  kernel_restart();
  return 0;
}

int trapsvc_handler(size_t argc, const char *const *argv) {
  if (argc < 2) {
    // If no argument specified, pass system call 0x4 by default as a test
    __asm__ volatile("mov x8, #4");
    __asm__ volatile("svc #0");
  } else {
    // Make a system call with the desired call number
    const char *digits = argv[1];
    const uint64_t system_call_requested = console_uint64_from_hex(digits);
    __asm__ volatile("mov x8, %0" : : "r"(system_call_requested) : "x8");
    __asm__ volatile("svc #0");
  }

  return 0;
}

int uptime_handler(size_t argc, const char *const *argv) {
  (void)argc;
  (void)argv;
  const uint64_t uptime_seconds = uptime();
  console_print("0x");
  console_print_hex_value((void *)&uptime_seconds, 8);
  console_print("\n");
  return 0;
}

#define COMMAND_LENGTH 255

void run_shell_loop(void) {
  char command[COMMAND_LENGTH];
  print_prompt();
  while (true) {
    console_read_line(command, COMMAND_LENGTH, true);
    dispatch_command(command);
    print_prompt();
  }
}
