// Distributed under the MIT license.
// See LICENSE.txt for details.

#include "kernel/Console/Shell.h"
#include "unity.h"

#include <stddef.h>
#include <stdint.h>
#include <string.h>

static char g_tx_buffer[512];
static size_t g_tx_index;

void platform_console_putc(char c) {
  if (g_tx_index < sizeof(g_tx_buffer)) {
    g_tx_buffer[g_tx_index++] = c;
  }
}

char platform_console_getc(void) { return '\0'; }

uintptr_t mmio_base_address(void) { return (uintptr_t)0x11223344u; }

uintptr_t uart_base_address(void) { return (uintptr_t)0x55667788u; }

size_t current_exception_level(void) { return 0x4; }

size_t mpidr_el1(void) { return (size_t)0xA1B2C3D4u; }

size_t sctlr_el1(void) { return (size_t)0x0F0E0D0Cu; }

const char *arch_name(void) { return "arm64-test"; }

const char *platform_name(void) { return "virt-test"; }

static void assert_tx_equals(const char *expected) {
  const size_t expected_len = strlen(expected);
  TEST_ASSERT_EQUAL_size_t(expected_len, g_tx_index);
  TEST_ASSERT_EQUAL_MEMORY(expected, g_tx_buffer, expected_len);
}

static void assert_tx_contains(const char *needle) {
  TEST_ASSERT_NOT_NULL(strstr(g_tx_buffer, needle));
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

void test_help_handler_prints_expected_command_list(void) {
  help_handler(0, NULL);

  assert_tx_contains("CharcoalOS available commands:");
  assert_tx_contains("help");
  assert_tx_contains("Display this help message");
  assert_tx_contains("info");
  assert_tx_contains("Display information for debugging");
}

void test_dispatch_command_help_prints_expected_command_list(void) {
  char command[] = "help";

  dispatch_command(command);

  assert_tx_contains("CharcoalOS available commands:");
  assert_tx_contains("memread");
  assert_tx_contains("reboot");
  assert_tx_contains("regs");
}

void test_info_handler_prints_expected_info_fields(void) {
  info_handler(0, NULL);

  assert_tx_contains("CharcoalOS built and linked on ");
  assert_tx_contains("Compiler: ");
  assert_tx_contains("Pointer size (bytes): 0x");
  assert_tx_contains("Hosted C environment: ");
  assert_tx_contains("MMIO base address: 0x");
  assert_tx_contains("UART base address: 0x");
  assert_tx_contains("Current exception level: 0x04");
  assert_tx_contains("MPIDR_EL1: 0x");
  assert_tx_contains("SCTLR_EL1: 0x");
  assert_tx_contains("Architecture: arm64-test");
  assert_tx_contains("Platform: virt-test");
}

void test_dispatch_command_info_prints_expected_info_fields(void) {
  char command[] = "info";

  dispatch_command(command);

  assert_tx_contains("Architecture: arm64-test");
  assert_tx_contains("Platform: virt-test");
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
  RUN_TEST(test_help_handler_prints_expected_command_list);
  RUN_TEST(test_dispatch_command_help_prints_expected_command_list);
  RUN_TEST(test_info_handler_prints_expected_info_fields);
  RUN_TEST(test_dispatch_command_info_prints_expected_info_fields);
  RUN_TEST(test_print_prompt_outputs_expected_prompt);
  return UNITY_END();
}
