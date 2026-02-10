// Distributed under the MIT license.
// See LICENSE.txt for details.

#pragma once

#include <stddef.h>

// Code to parse commands (received as a string) and call the appropriate
// responders.

#define MAX_SHELL_ARGS 10

/*!
 * \brief A command for the kernel shell
 * \param name The name of the command
 * \param help_text Brief description of the command
 * \param handler The function that handles the command
 */
struct shell_command {
  const char *name;
  const char *help_text;
  int (*handler)(size_t argc, const char *const *argv);
};

/*!
 * \brief Function that handles the help shell command
 * \param argc Number of arguments (including the command name)
 * \param argv The command arguments (including the command name)
 */
int help_handler(size_t argc, const char *const *argv);
int info_handler(size_t argc, const char *const *argv);
int memread_handler(size_t argc, const char *const *argv);
int panic_handler(size_t argc, const char *const *argv);
int reboot_handler(size_t argc, const char *const *argv);
int registers_handler(size_t argc, const char *const *argv);

/*!
 * \brief The table of built-in shell commands
 */
static const struct shell_command commands[] = {
    {"help", "Display this help message", help_handler},
    {"info", "Display information for debugging", info_handler},
    {"memread", "Read memory at address", memread_handler},
    {"panic", "Panic the kernel", panic_handler},
    {"reboot", "Reboot the system", reboot_handler},
    {"regs", "Show system registers", registers_handler},
};

/*!
 * \brief Function to tokenize a command entered on the command line
 * \details Replaces whitespace (`\n`, `\r`, `\t`, space) with terminator
 * `\0` in `command`. Tokens is a list of the address at the start of
 * each argument, including the command name as the zeroth argument.
 * \param command The command to execute
 * \param tokens The tokens recovered from the commmand
 * \param max_tokens The maximum number of tokens (args) allowed for commands
 */
size_t tokenize_command(char *command, char *tokens[], size_t max_tokens);

/*!
 * \brief Tokenize and handle a command.
 * \details Calls `tokenize_command` to turn the command string into
 * tokens, and then calls the appropriate handler.
 * \param command The command to tokenize and handle.
 */
void dispatch_command(char *command);

/*!
 * \brief Print the shell prompt.
 */
void print_prompt(void);
