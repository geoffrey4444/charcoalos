// Distributed under the MIT license.
// See LICENSE.txt for details.

#include "kernel/Console/IO.h"
#include "kernel/Console/Shell.h"
#include "kernel/String/String.h"

#include <stdbool.h>
#include <stddef.h>

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
  console_print("Command not yet implemented\n");
  return 1;
}

int memread_handler(size_t argc, const char *const *argv) {
  (void)argc;
  (void)argv;
  console_print("Command not yet implemented\n");
  return 1;
}

int reboot_handler(size_t argc, const char *const *argv) {
  (void)argc;
  (void)argv;
  console_print("Command not yet implemented\n");
  return 1;
}

int registers_handler(size_t argc, const char *const *argv) {
  (void)argc;
  (void)argv;
  console_print("Command not yet implemented\n");
  return 1;
}
