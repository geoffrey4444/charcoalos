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
struct ShellCommand {
  const char *name;
  const char *help_text;
  int (*handler)(size_t argc, const char *const *argv);
};

/*!
 * \brief Returns the number of available shell commands
 * \returns The number of available shell commands
 */
size_t shell_number_of_commands(void);

/*!
 * \brief Returns the shell command at index `index`, panics if index
 * is out of bounds.
 * \param index The index of the command to return
 * \returns The shell command at index `index`
 */
struct ShellCommand shell_command_at(size_t index);

/*!
 * \brief Function that handles the add shell command
 * \param argc Number of arguments (including the command name)
 * \param argv The command arguments (including the command name)
 * \returns 0 on success, nonzero on failure
 */
int add_handler(size_t argc, const char *const *argv);

/*!
 * \brief Function that handles the help shell command
 * \param argc Number of arguments (including the command name)
 * \param argv The command arguments (including the command name)
 * \returns 0 on success, nonzero on failure
 */
int help_handler(size_t argc, const char *const *argv);

/*!
 * \brief Function that handles the info shell command
 * \param argc Number of arguments (including the command name)
 * \param argv The command arguments (including the command name)
 * \returns 0 on success, nonzero on failure
 */
int info_handler(size_t argc, const char *const *argv);

/*!
 * \brief Function that handles the memread shell command
 * \param argc Number of arguments (including the command name)
 * \param argv The command arguments (including the command name)
 * \returns 0 on success, nonzero on failure
 */
int memread_handler(size_t argc, const char *const *argv);

/*!
 * \brief Function that handles the panic shell command
 * \param argc Number of arguments (including the command name)
 * \param argv The command arguments (including the command name)
 * \returns 0 on success, nonzero on failure
 */
int panic_handler(size_t argc, const char *const *argv);

/*!
 * \brief Function that handles the reboot shell command
 * \param argc Number of arguments (including the command name)
 * \param argv The command arguments (including the command name)
 * \returns 0 on success, nonzero on failure
 */
int reboot_handler(size_t argc, const char *const *argv);

/*!
 * \brief Function that handles the trapsvc shell command
 * \param argc Number of arguments (including the command name)
 * \param argv The command arguments (including the command name)
 * \returns 0 on success, nonzero on failure
 */
int trapsvc_handler(size_t argc, const char *const *argv);

/*!
 * \brief Function that handles the uptime shell command
 * \param argc Number of arguments (including the command name)
 * \param argv The command arguments (including the command name)
 * \returns 0 on success, nonzero on failure
 */
int uptime_handler(size_t argc, const char *const *argv);

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

/*!
 * \brief Runs the shell in a loop, reading and dispatching commands until
 * the kernel halts.
 */
void run_shell_loop(void);
