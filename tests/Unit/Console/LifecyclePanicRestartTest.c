// Distributed under the MIT license.
// See LICENSE.txt for details.

#include "kernel/Main/Lifecycle.h"
#include "kernel/Panic/Panic.h"
#include "kernel/Panic/Restart.h"
#include "kernel/Hardware/DeviceTree.h"
#include "unity.h"

#include <stddef.h>
#include <string.h>

static char g_tx_buffer[256];
static size_t g_tx_index;
static size_t g_halt_calls;
static size_t g_shell_loop_calls;
static size_t g_platform_reboot_calls;
static size_t g_custom_foreground_calls;
static size_t g_initialize_timer_calls;
static size_t g_print_timer_diagnostics_calls;
static const uint32_t g_valid_dtb_magic = 0xedfe0dd0U;

void console_print(const char *string) {
  if (string == NULL) {
    return;
  }
  const size_t size = strlen(string);
  if (g_tx_index + size > sizeof(g_tx_buffer)) {
    return;
  }
  memcpy(g_tx_buffer + g_tx_index, string, size);
  g_tx_index += size;
}

void console_print_hex(const void *data, size_t size) {
  static const char k_hex_digits[] = "0123456789ABCDEF";
  const uint8_t *bytes = (const uint8_t *)data;
  for (size_t i = size; i > 0; --i) {
    const uint8_t byte = bytes[i - 1];
    if (g_tx_index + 2 > sizeof(g_tx_buffer)) {
      return;
    }
    g_tx_buffer[g_tx_index++] = k_hex_digits[(byte >> 4) & 0x0F];
    g_tx_buffer[g_tx_index++] = k_hex_digits[byte & 0x0F];
  }
}

void halt(void) { ++g_halt_calls; }

void run_shell_loop(void) { ++g_shell_loop_calls; }

void initialize_timer(void) { ++g_initialize_timer_calls; }

void print_timer_diagnostics(void) { ++g_print_timer_diagnostics_calls; }

void platform_reboot(void) { ++g_platform_reboot_calls; }

void parse_device_tree_blob(struct hardware_info *out_hw_info, uintptr_t dtb) {
  (void)dtb;
  if (out_hw_info == NULL) {
    return;
  }
  out_hw_info->header.magic = g_valid_dtb_magic;
  console_print("Device table blob recognized with magic EDFE0DD0\n");
}

static void custom_foreground_client(void) { ++g_custom_foreground_calls; }

void setUp(void) {
  memset(g_tx_buffer, 0, sizeof(g_tx_buffer));
  g_tx_index = 0;
  g_halt_calls = 0;
  g_shell_loop_calls = 0;
  g_platform_reboot_calls = 0;
  g_custom_foreground_calls = 0;
  g_initialize_timer_calls = 0;
  g_print_timer_diagnostics_calls = 0;
  kernel_set_foreground_client(NULL);
}

void tearDown(void) {}

void test_kernel_init_prints_welcome_message(void) {
  kernel_init((uintptr_t)&g_valid_dtb_magic);

  TEST_ASSERT_EQUAL_STRING_LEN(
      "Initializing timer... done.\n"
      "Parsing device tree blob for hardware information...\n"
      "Device table blob recognized with magic EDFE0DD0\n"
      "Welcome to CharcoalOS.\n",
      g_tx_buffer,
      strlen("Initializing timer... done.\n"
             "Parsing device tree blob for hardware information...\n"
             "Device table blob recognized with magic EDFE0DD0\n"
             "Welcome to CharcoalOS.\n"));
  TEST_ASSERT_EQUAL_size_t(1, g_initialize_timer_calls);
  TEST_ASSERT_EQUAL_size_t(1, g_print_timer_diagnostics_calls);
}

void test_kernel_run_calls_default_shell_client_after_init(void) {
  kernel_init((uintptr_t)&g_valid_dtb_magic);
  kernel_run();

  TEST_ASSERT_EQUAL_size_t(1, g_shell_loop_calls);
}

void test_kernel_run_halts_when_foreground_client_is_unset(void) {
  kernel_run();

  TEST_ASSERT_EQUAL_size_t(1, g_halt_calls);
  TEST_ASSERT_EQUAL_size_t(0, g_shell_loop_calls);
  TEST_ASSERT_EQUAL_size_t(0, g_custom_foreground_calls);
}

void test_kernel_run_calls_custom_foreground_client_when_configured(void) {
  kernel_set_foreground_client(custom_foreground_client);
  kernel_run();

  TEST_ASSERT_EQUAL_size_t(1, g_custom_foreground_calls);
  TEST_ASSERT_EQUAL_size_t(0, g_shell_loop_calls);
}

void test_kernel_halt_calls_halt(void) {
  kernel_halt();

  TEST_ASSERT_EQUAL_size_t(1, g_halt_calls);
}

void test_kernel_panic_with_message_prints_message_and_halts(void) {
  kernel_panic("panic detail");

  TEST_ASSERT_EQUAL_STRING_LEN("panic detail\n\n", g_tx_buffer,
                               strlen("panic detail\n\n"));
  TEST_ASSERT_EQUAL_size_t(1, g_halt_calls);
}

void test_kernel_panic_without_message_halts_without_printing(void) {
  kernel_panic(NULL);

  TEST_ASSERT_EQUAL_size_t(0, g_tx_index);
  TEST_ASSERT_EQUAL_size_t(1, g_halt_calls);
}

void test_kernel_restart_calls_platform_reboot(void) {
  kernel_restart();

  TEST_ASSERT_EQUAL_size_t(1, g_platform_reboot_calls);
}

int main(void) {
  UNITY_BEGIN();
  RUN_TEST(test_kernel_init_prints_welcome_message);
  RUN_TEST(test_kernel_run_calls_default_shell_client_after_init);
  RUN_TEST(test_kernel_run_halts_when_foreground_client_is_unset);
  RUN_TEST(test_kernel_run_calls_custom_foreground_client_when_configured);
  RUN_TEST(test_kernel_halt_calls_halt);
  RUN_TEST(test_kernel_panic_with_message_prints_message_and_halts);
  RUN_TEST(test_kernel_panic_without_message_halts_without_printing);
  RUN_TEST(test_kernel_restart_calls_platform_reboot);
  return UNITY_END();
}
