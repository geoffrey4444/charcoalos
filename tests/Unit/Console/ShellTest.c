// Distributed under the MIT license.
// See LICENSE.txt for details.

#include "kernel/Console/Shell.h"
#include "unity.h"

#include <stddef.h>
#include <string.h>

static char g_tx_buffer[512];
static size_t g_tx_index;

void platform_console_putc(char c) {
  if (g_tx_index < sizeof(g_tx_buffer)) {
    g_tx_buffer[g_tx_index++] = c;
  }
}

char platform_console_getc(void) { return '\0'; }

static void assert_tx_equals(const char *expected) {
  const size_t expected_len = strlen(expected);
  TEST_ASSERT_EQUAL_size_t(expected_len, g_tx_index);
  TEST_ASSERT_EQUAL_MEMORY(expected, g_tx_buffer, expected_len);
}

void setUp(void) {
  memset(g_tx_buffer, 0, sizeof(g_tx_buffer));
  g_tx_index = 0;
}

void tearDown(void) {}

void test_tokenize_command_returns_zero_for_empty_command(void) {
  char command[] = "";
  char *tokens[MAX_SHELL_ARGS] = {0};

  const size_t argc = tokenize_command(command, tokens, MAX_SHELL_ARGS);

  TEST_ASSERT_EQUAL_size_t(0, argc);
}

void test_tokenize_command_splits_whitespace_and_rewrites_separators(void) {
  char command[] = "memread\t0x1000  \n0x10\r";
  char *tokens[MAX_SHELL_ARGS] = {0};

  const size_t argc = tokenize_command(command, tokens, MAX_SHELL_ARGS);

  TEST_ASSERT_EQUAL_size_t(3, argc);
  TEST_ASSERT_EQUAL_STRING("memread", tokens[0]);
  TEST_ASSERT_EQUAL_STRING("0x1000", tokens[1]);
  TEST_ASSERT_EQUAL_STRING("0x10", tokens[2]);
}

void test_dispatch_command_ignores_empty_input(void) {
  char command[] = "";

  dispatch_command(command);

  TEST_ASSERT_EQUAL_size_t(0, g_tx_index);
}

void test_dispatch_command_prints_error_for_unknown_command(void) {
  char command[] = "doesnotexist";

  dispatch_command(command);

  assert_tx_equals(
      "Error: the command "
      "doesnotexist"
      " could not be recognized. Type help for a list of commands.\r\n");
}

void test_print_prompt_outputs_expected_prompt(void) {
  print_prompt();

  assert_tx_equals("> ");
}

int main(void) {
  UNITY_BEGIN();
  RUN_TEST(test_tokenize_command_returns_zero_for_empty_command);
  RUN_TEST(test_tokenize_command_splits_whitespace_and_rewrites_separators);
  RUN_TEST(test_dispatch_command_ignores_empty_input);
  RUN_TEST(test_dispatch_command_prints_error_for_unknown_command);
  RUN_TEST(test_print_prompt_outputs_expected_prompt);
  return UNITY_END();
}
