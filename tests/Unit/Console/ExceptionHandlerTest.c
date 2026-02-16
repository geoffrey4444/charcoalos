// Distributed under the MIT license.
// See LICENSE.txt for details.

#include "arch/ExceptionHandler.h"
#include "arch/arm64/ExceptionTypes.h"
#include "unity.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>

static char g_tx_buffer[8192];
static size_t g_tx_index;
static size_t g_halt_calls;
static size_t g_interrupt_handler_calls;
static bool g_interrupt_handler_result;

void platform_console_putc(char c) {
  if (g_tx_index < sizeof(g_tx_buffer)) {
    g_tx_buffer[g_tx_index++] = c;
  }
}

char platform_console_getc(void) { return '\0'; }

void halt(void) { ++g_halt_calls; }

bool handle_interrupt_exception(void) {
  ++g_interrupt_handler_calls;
  return g_interrupt_handler_result;
}

static void assert_tx_contains(const char *needle) {
  TEST_ASSERT_NOT_NULL(strstr(g_tx_buffer, needle));
}

void setUp(void) {
  memset(g_tx_buffer, 0, sizeof(g_tx_buffer));
  g_tx_index = 0;
  g_halt_calls = 0;
  g_interrupt_handler_calls = 0;
  g_interrupt_handler_result = true;
}

void tearDown(void) {}

void test_handle_exception_prints_diagnostics_and_panics(void) {
  uint64_t saved_registers[32] = {0};
  for (uint64_t i = 0; i < 32; ++i) {
    saved_registers[i] = 0x1000ULL + i;
  }

  uint64_t action =
      handle_exception(saved_registers, EXCEPTION_TYPE_SYNC_EL1_SPX);

  TEST_ASSERT_EQUAL_size_t(action, EXCEPTION_ACTION_PANIC);
  assert_tx_contains("Sorry, a system error has occurred.");
  assert_tx_contains("Type =  0x0000000000000004 = SYNC_EL1_SPX");
  assert_tx_contains("Class = 0x0000000000000000 = EXCEPTION_CLASS_UNKNOWN");
  assert_tx_contains("Saved registers (Register Value):");
  assert_tx_contains("0x0000000000000000 0x0000000000001000");
  assert_tx_contains("0x000000000000001F 0x000000000000101F");
  assert_tx_contains("Kernel panic: your computer must be restarted.");
}

void test_handle_exception_irq_calls_interrupt_handler_and_resumes(void) {
  uint64_t saved_registers[32] = {0};
  g_interrupt_handler_result = true;

  uint64_t action = handle_exception(saved_registers, EXCEPTION_TYPE_IRQ_EL1_SPX);

  TEST_ASSERT_EQUAL_size_t(EXCEPTION_ACTION_RESUME, action);
  TEST_ASSERT_EQUAL_size_t(1, g_interrupt_handler_calls);
  TEST_ASSERT_EQUAL_size_t(0, g_halt_calls);
  TEST_ASSERT_EQUAL_size_t(0, g_tx_index);
}

void test_handle_exception_irq_unhandled_source_panics(void) {
  uint64_t saved_registers[32] = {0};
  g_interrupt_handler_result = false;

  uint64_t action = handle_exception(saved_registers, EXCEPTION_TYPE_IRQ_EL1_SPX);

  TEST_ASSERT_EQUAL_size_t(EXCEPTION_ACTION_PANIC, action);
  TEST_ASSERT_EQUAL_size_t(1, g_interrupt_handler_calls);
  assert_tx_contains("Sorry, a system error has occurred.");
  assert_tx_contains("Type =  0x0000000000000005 = IRQ_EL1_SPX");
  assert_tx_contains("Kernel panic: your computer must be restarted.");
}

int main(void) {
  UNITY_BEGIN();
  RUN_TEST(test_handle_exception_prints_diagnostics_and_panics);
  RUN_TEST(test_handle_exception_irq_calls_interrupt_handler_and_resumes);
  RUN_TEST(test_handle_exception_irq_unhandled_source_panics);
  return UNITY_END();
}
