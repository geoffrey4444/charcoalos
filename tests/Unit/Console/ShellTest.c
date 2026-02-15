// Distributed under the MIT license.
// See LICENSE.txt for details.

#include "kernel/Console/Shell.h"
#include "kernel/Console/IO.h"
#include "unity.h"

#include <setjmp.h>
#include <stdbool.h>
#include <stdio.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>

static char g_tx_buffer[512];
static size_t g_tx_index;
static size_t g_halt_calls;
static size_t g_reboot_calls;
static char g_rx_buffer[512];
static size_t g_rx_size;
static size_t g_rx_index;
static bool g_break_shell_loop_on_rx_exhausted;
static bool g_shell_loop_escape_enabled;
// Stores the saved execution point for setjmp/longjmp test escape.
static jmp_buf g_shell_loop_escape_jmp;

void platform_console_putc(char c) {
  if (g_tx_index < sizeof(g_tx_buffer)) {
    g_tx_buffer[g_tx_index++] = c;
  }
}

char platform_console_getc(void) {
  if (g_rx_index < g_rx_size) {
    return g_rx_buffer[g_rx_index++];
  }
  if (g_break_shell_loop_on_rx_exhausted && g_shell_loop_escape_enabled) {
    // Jump back to the matching setjmp() in the test, which exits the
    // infinite run_shell_loop() once scripted input is consumed.
    longjmp(g_shell_loop_escape_jmp, 1);
  }
  return '\0';
}

uintptr_t mmio_base_address(void) { return (uintptr_t)0x11223344u; }

uintptr_t uart_base_address(void) { return (uintptr_t)0x55667788u; }

size_t current_exception_level(void) { return 0x4; }

size_t mpidr_el1(void) { return (size_t)0xA1B2C3D4u; }

size_t sctlr_el1(void) { return (size_t)0x0F0E0D0Cu; }

uintptr_t stack_pointer_address(void) { return (uintptr_t)0xCAFEBABEu; }

const char *arch_name(void) { return "arm64-test"; }

const char *platform_name(void) { return "virt-test"; }

void halt(void) { ++g_halt_calls; }

void platform_reboot(void) { ++g_reboot_calls; }

void kernel_panic(const char *panic_message) {
  if (panic_message != NULL) {
    console_print(panic_message);
    console_print("\n\n");
  }
  halt();
}

void kernel_restart(void) { platform_reboot(); }

static void assert_tx_equals(const char *expected) {
  const size_t expected_len = strlen(expected);
  TEST_ASSERT_EQUAL_size_t(expected_len, g_tx_index);
  TEST_ASSERT_EQUAL_MEMORY(expected, g_tx_buffer, expected_len);
}

static void assert_tx_contains(const char *needle) {
  TEST_ASSERT_NOT_NULL(strstr(g_tx_buffer, needle));
}

static void load_rx(const char *data, size_t size) {
  TEST_ASSERT_TRUE(size <= sizeof(g_rx_buffer));
  memcpy(g_rx_buffer, data, size);
  g_rx_size = size;
  g_rx_index = 0;
}

void setUp(void) {
  memset(g_tx_buffer, 0, sizeof(g_tx_buffer));
  memset(g_rx_buffer, 0, sizeof(g_rx_buffer));
  g_tx_index = 0;
  g_halt_calls = 0;
  g_reboot_calls = 0;
  g_rx_size = 0;
  g_rx_index = 0;
  g_break_shell_loop_on_rx_exhausted = false;
  g_shell_loop_escape_enabled = false;
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

void test_tokenize_command_respects_max_tokens() {
  char command[] = "help arg1 arg2 arg3 arg4 arg5 arg6 arg7";
  char *tokens[4] = {0};

  const size_t argc = tokenize_command(command, tokens, 4);
  TEST_ASSERT_EQUAL_size_t(4, argc);
  TEST_ASSERT_EQUAL_STRING("help", tokens[0]);
  TEST_ASSERT_EQUAL_STRING("arg1", tokens[1]);
  TEST_ASSERT_EQUAL_STRING("arg2", tokens[2]);
  TEST_ASSERT_EQUAL_STRING("arg3", tokens[3]);
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

void test_help_handler_prints_all_command_names(void) {
  help_handler(0, NULL);

  assert_tx_contains("CharcoalOS available commands:");
  const size_t number_of_commands = shell_number_of_commands();
  for (size_t i = 0; i < number_of_commands; ++i) {
    assert_tx_contains(shell_command_at(i).name);
  }
}

void test_shell_command_at_exposes_expected_commands_without_order_assumptions(
    void) {
  bool found_add = false;
  bool found_help = false;
  bool found_info = false;
  bool found_memread = false;
  bool found_panic = false;
  bool found_reboot = false;
  bool found_trapsvc = false;

  const size_t number_of_commands = shell_number_of_commands();
  TEST_ASSERT_TRUE(number_of_commands > 0);

  for (size_t i = 0; i < number_of_commands; ++i) {
    const struct shell_command command = shell_command_at(i);
    TEST_ASSERT_NOT_NULL(command.name);
    TEST_ASSERT_NOT_NULL(command.help_text);
    TEST_ASSERT_NOT_NULL(command.handler);

    if (strcmp(command.name, "add") == 0) {
      found_add = true;
    } else if (strcmp(command.name, "help") == 0) {
      found_help = true;
    } else if (strcmp(command.name, "info") == 0) {
      found_info = true;
    } else if (strcmp(command.name, "memread") == 0) {
      found_memread = true;
    } else if (strcmp(command.name, "panic") == 0) {
      found_panic = true;
    } else if (strcmp(command.name, "reboot") == 0) {
      found_reboot = true;
    } else if (strcmp(command.name, "trapsvc") == 0) {
      found_trapsvc = true;
    }
  }

  TEST_ASSERT_TRUE(found_add);
  TEST_ASSERT_TRUE(found_help);
  TEST_ASSERT_TRUE(found_info);
  TEST_ASSERT_TRUE(found_memread);
  TEST_ASSERT_TRUE(found_panic);
  TEST_ASSERT_TRUE(found_reboot);
  TEST_ASSERT_TRUE(found_trapsvc);
}

void test_dispatch_command_help_prints_expected_command_list(void) {
  char command[] = "help";

  dispatch_command(command);

  assert_tx_contains("CharcoalOS available commands:");
  assert_tx_contains("memread");
  assert_tx_contains("reboot");
  assert_tx_contains("add");
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

void test_panic_handler_prints_message_and_calls_halt(void) {
  const int result = panic_handler(0, NULL);

  TEST_ASSERT_EQUAL_INT(0, result);
  assert_tx_equals("Kernel panic\r\n\r\n");
  TEST_ASSERT_EQUAL_size_t(1, g_halt_calls);
}

void test_panic_handler_with_argument_prints_message_argument_and_calls_halt(void) {
  const char *args[] = {"Error code"};
  const int result = panic_handler(1, args);

  TEST_ASSERT_EQUAL_INT(0, result);
  assert_tx_equals("Kernel panic\r\n\r\nError code\r\n\r\n");
  TEST_ASSERT_EQUAL_size_t(1, g_halt_calls);
}

void test_panic_handler_with_two_arguments_prints_second_argument_and_calls_halt(void) {
  const char *args[] = {"panic", "Error code"};
  const int result = panic_handler(2, args);

  TEST_ASSERT_EQUAL_INT(0, result);
  assert_tx_equals("Kernel panic\r\n\r\nError code\r\n\r\n");
  TEST_ASSERT_EQUAL_size_t(1, g_halt_calls);
}

void test_dispatch_command_panic_prints_message_and_calls_halt(void) {
  char command[] = "panic";

  dispatch_command(command);

  assert_tx_equals("Kernel panic\r\n\r\npanic\r\n\r\n");
  TEST_ASSERT_EQUAL_size_t(1, g_halt_calls);
}

void test_reboot_handler_calls_platform_reboot(void) {
  const int result = reboot_handler(0, NULL);

  TEST_ASSERT_EQUAL_INT(0, result);
  TEST_ASSERT_EQUAL_size_t(1, g_reboot_calls);
  TEST_ASSERT_EQUAL_size_t(0, g_tx_index);
}

void test_dispatch_command_reboot_calls_platform_reboot(void) {
  char command[] = "reboot";

  dispatch_command(command);

  TEST_ASSERT_EQUAL_size_t(1, g_reboot_calls);
  TEST_ASSERT_EQUAL_size_t(0, g_tx_index);
}

void test_dispatch_command_info_prints_expected_info_fields(void) {
  char command[] = "info";

  dispatch_command(command);

  assert_tx_contains("Architecture: arm64-test");
  assert_tx_contains("Platform: virt-test");
}

void test_memread_handler_prints_address_and_contents(void) {
  const size_t value = 0x0123456789ABCDEFu;
  char address_text[32] = {0};
  char expected[64] = {0};
  const int address_len =
      snprintf(address_text, sizeof(address_text), "0x%llX",
               (unsigned long long)(uintptr_t)&value);
  TEST_ASSERT_TRUE(address_len > 0);
  const char *args[] = {"memread", address_text};

  const int result = memread_handler(2, args);

  TEST_ASSERT_EQUAL_INT(0, result);
  const int expected_len =
      snprintf(expected, sizeof(expected), "%016llX    %016llX\r\n",
               (unsigned long long)(uintptr_t)&value, (unsigned long long)value);
  TEST_ASSERT_TRUE(expected_len > 0);
  assert_tx_equals(expected);
}

void test_add_handler_prints_hex_sum_for_two_arguments(void) {
  const char *args[] = {"add", "0x1A", "0x05"};

  const int result = add_handler(3, args);

  TEST_ASSERT_EQUAL_INT(0, result);
  assert_tx_equals("000000000000001F\r\n");
}

void test_add_handler_with_wrong_argument_count_prints_usage_and_fails(void) {
  const char *args[] = {"add", "0x1A"};

  const int result = add_handler(2, args);

  TEST_ASSERT_EQUAL_INT(1, result);
  assert_tx_equals("Usage: add 0x1234 0xdbca\r\n");
}

void test_dispatch_command_add_prints_sum(void) {
  char command[] = "add 0x10 0x20";

  dispatch_command(command);

  assert_tx_equals("0000000000000030\r\n");
}

void test_print_prompt_outputs_expected_prompt(void) {
  print_prompt();

  assert_tx_equals("> ");
}

void test_run_shell_loop_prints_prompt_dispatches_input_and_prompts_again(void) {
  const char input[] = "help\n";
  load_rx(input, sizeof(input) - 1);
  g_break_shell_loop_on_rx_exhausted = true;

  // setjmp() saves a return point in g_shell_loop_escape_jmp. It returns 0
  // the first time, then returns the value passed to longjmp() (1 here) when
  // platform_console_getc() longjmps after input is exhausted.
  if (setjmp(g_shell_loop_escape_jmp) == 0) {
    g_shell_loop_escape_enabled = true;
    run_shell_loop();
  }
  g_shell_loop_escape_enabled = false;

  assert_tx_contains("> ");
  assert_tx_contains("CharcoalOS available commands:");
  TEST_ASSERT_TRUE(g_tx_index >= 4);
  TEST_ASSERT_EQUAL_CHAR('>', g_tx_buffer[g_tx_index - 2]);
  TEST_ASSERT_EQUAL_CHAR(' ', g_tx_buffer[g_tx_index - 1]);
}

int main(void) {
  UNITY_BEGIN();
  RUN_TEST(test_tokenize_command_returns_zero_for_empty_command);
  RUN_TEST(test_tokenize_command_splits_whitespace_and_rewrites_separators);
  RUN_TEST(test_dispatch_command_ignores_empty_input);
  RUN_TEST(test_dispatch_command_prints_error_for_unknown_command);
  RUN_TEST(test_help_handler_prints_all_command_names);
  RUN_TEST(
      test_shell_command_at_exposes_expected_commands_without_order_assumptions);
  RUN_TEST(test_dispatch_command_help_prints_expected_command_list);
  RUN_TEST(test_info_handler_prints_expected_info_fields);
  RUN_TEST(test_dispatch_command_info_prints_expected_info_fields);
  RUN_TEST(test_panic_handler_prints_message_and_calls_halt);
  RUN_TEST(test_panic_handler_with_argument_prints_message_argument_and_calls_halt);
  RUN_TEST(test_panic_handler_with_two_arguments_prints_second_argument_and_calls_halt);
  RUN_TEST(test_dispatch_command_panic_prints_message_and_calls_halt);
  RUN_TEST(test_reboot_handler_calls_platform_reboot);
  RUN_TEST(test_dispatch_command_reboot_calls_platform_reboot);
  RUN_TEST(test_memread_handler_prints_address_and_contents);
  RUN_TEST(test_add_handler_prints_hex_sum_for_two_arguments);
  RUN_TEST(test_add_handler_with_wrong_argument_count_prints_usage_and_fails);
  RUN_TEST(test_dispatch_command_add_prints_sum);
  RUN_TEST(test_print_prompt_outputs_expected_prompt);
  RUN_TEST(test_run_shell_loop_prints_prompt_dispatches_input_and_prompts_again);
  RUN_TEST(test_tokenize_command_respects_max_tokens);
  return UNITY_END();
}
